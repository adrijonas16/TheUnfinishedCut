#include "TUCInteractionActor.h"

#include "TUCAssets.h"
#include "TUCCharacterBase.h"
#include "TUCPuzzleDirector.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    const TCHAR* ImportedProjectorPackage = TEXT("/Game/TUC/Props/Projector/SM_RetroProjector_Optimized");
    constexpr float ProjectorTargetLength = 110.0f;
}

ATUCInteractionActor::ATUCInteractionActor()
{
    PrimaryActorTick.bCanEverTick = true;

    InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
    SetRootComponent(InteractionBox);
    InteractionBox->SetBoxExtent(FVector(50.0f));
    InteractionBox->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    InteractionBox->SetMobility(EComponentMobility::Movable);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(InteractionBox);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->SetMobility(EComponentMobility::Movable);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        Mesh->SetStaticMesh(CubeMesh.Object);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (ShapeMaterial.Succeeded())
    {
        BaseShapeMaterial = ShapeMaterial.Object;
    }

    AccentLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("AccentLight"));
    AccentLight->SetupAttachment(InteractionBox);
    AccentLight->SetMobility(EComponentMobility::Movable);
    AccentLight->SetRelativeLocation(FVector(0.0f, 0.0f, 90.0f));
    AccentLight->Intensity = BaseLightIntensity;
    AccentLight->AttenuationRadius = 320.0f;
    AccentLight->SetCastShadows(false);
}

void ATUCInteractionActor::Configure(ATUCPuzzleDirector* InDirector, ETUCInteractionKind NewKind, const FLinearColor& NewColor, const FVector& Scale, int32 InFragmentId, const TCHAR* KitAsset, float KitYaw)
{
    Director = InDirector;
    InteractionKind = NewKind;
    FragmentId = InFragmentId;

    const FVector HalfExtent = Scale * 50.0f;
    InteractionBox->SetBoxExtent(HalfExtent);
    Mesh->SetRelativeLocation(FVector::ZeroVector);
    Mesh->SetRelativeScale3D(Scale);

    bool bUsesImportedMesh = false;
    if (InteractionKind == ETUCInteractionKind::OriginProjector
        || InteractionKind == ETUCInteractionKind::RetroProjector
        || InteractionKind == ETUCInteractionKind::FutureProjector)
    {
        bUsesImportedMesh = TryUseImportedProjector(Scale);
    }
    else if (KitAsset)
    {
        bUsesImportedMesh = TryUseKitMesh(KitAsset, KitYaw, Scale, NewColor);
    }

    if (!bUsesImportedMesh && BaseShapeMaterial)
    {
        if (UMaterialInstanceDynamic* Tint = UMaterialInstanceDynamic::Create(BaseShapeMaterial, this))
        {
            Tint->SetVectorParameterValue(TEXT("Color"), NewColor * 0.55f);
            Mesh->SetMaterial(0, Tint);
        }
    }

    AccentLight->SetLightColor(NewColor);
    AccentLight->SetRelativeLocation(FVector(0.0f, 0.0f, InteractionBox->GetUnscaledBoxExtent().Z + 60.0f));
}

bool ATUCInteractionActor::TryUseKitMesh(const TCHAR* KitAsset, float KitYaw, const FVector& RequestedScale, const FLinearColor& AccentColor)
{
    UStaticMesh* KitMesh = TUCAssets::LoadMesh(TUCAssets::KitMesh(KitAsset));
    if (!KitMesh)
    {
        return false;
    }

    // Caja de interacción = bounds de la malla girada (yaw libre), a escala real del kit.
    const FBoxSphereBounds Bounds = KitMesh->GetBounds();
    const FRotator Rotation(0.0f, KitYaw, 0.0f);
    const FBox LocalBox(Bounds.Origin - Bounds.BoxExtent, Bounds.Origin + Bounds.BoxExtent);
    const FBox RotatedBox = LocalBox.TransformBy(FTransform(Rotation));
    const FVector RotatedCenter = RotatedBox.GetCenter();
    const FVector RotatedHalf = RotatedBox.GetExtent();

    // Los fragmentos flotan (se conserva su centro); el resto se apoya en el suelo original.
    FVector NewLocation = GetActorLocation();
    if (InteractionKind != ETUCInteractionKind::MemoryFragment)
    {
        const float GroundZ = GetActorLocation().Z - RequestedScale.Z * 50.0f;
        NewLocation.Z = GroundZ + RotatedHalf.Z;
    }
    SetActorLocation(NewLocation);

    Mesh->SetStaticMesh(KitMesh);
    Mesh->SetRelativeScale3D(FVector::OneVector);
    Mesh->SetRelativeRotation(Rotation);
    Mesh->SetRelativeLocation(-RotatedCenter);
    InteractionBox->SetBoxExtent(RotatedHalf);
    TUCAssets::TintAccent(Mesh, AccentColor);
    return true;
}

bool ATUCInteractionActor::TryUseImportedProjector(const FVector& RequestedScale)
{
    // Carga silenciosa: si el asset todavía no fue importado se mantiene el cubo de blockout.
    UStaticMesh* Projector = TUCAssets::LoadMesh(ImportedProjectorPackage);
    if (!Projector)
    {
        return false;
    }

    const FBoxSphereBounds Bounds = Projector->GetBounds();
    const float LongestHorizontal = 2.0f * FMath::Max(Bounds.BoxExtent.X, Bounds.BoxExtent.Y);
    if (LongestHorizontal <= KINDA_SMALL_NUMBER)
    {
        return false;
    }

    // Ajuste por bounds: funciona tanto si el FBX llega en metros como en centímetros.
    const float Fit = ProjectorTargetLength / LongestHorizontal;
    const FVector FittedHalf = Bounds.BoxExtent * Fit;

    // Mantener la base apoyada en el mismo suelo que el cubo original.
    const float GroundZ = GetActorLocation().Z - RequestedScale.Z * 50.0f;
    FVector NewLocation = GetActorLocation();
    NewLocation.Z = GroundZ + FittedHalf.Z;
    SetActorLocation(NewLocation);

    Mesh->SetStaticMesh(Projector);
    Mesh->SetRelativeScale3D(FVector(Fit));
    Mesh->SetRelativeLocation(-Bounds.Origin * Fit);
    InteractionBox->SetBoxExtent(FittedHalf);
    return true;
}

void ATUCInteractionActor::SetConsumed(bool bNewConsumed)
{
    bConsumed = bNewConsumed;
    SetActorHiddenInGame(bConsumed);
    SetActorEnableCollision(!bConsumed);
    AccentLight->SetVisibility(!bConsumed);
}

ATUCPuzzleDirector* ATUCInteractionActor::ResolveDirector() const
{
    if (Director.IsValid())
    {
        return Director.Get();
    }
    return ATUCPuzzleDirector::Get(this);
}

void ATUCInteractionActor::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    const ATUCPuzzleDirector* CurrentDirector = ResolveDirector();
    if (bConsumed || !CurrentDirector || !GetWorld())
    {
        return;
    }

    // La luz comunica el estado: pulsa la pieza que hace avanzar la historia, se apaga lo resuelto.
    const float Time = static_cast<float>(GetWorld()->GetTimeSeconds());
    float Multiplier = 0.35f;
    if (CurrentDirector->IsCurrentObjective(InteractionKind))
    {
        Multiplier = 1.4f + 0.6f * FMath::Sin(Time * 4.0f);
    }
    else
    {
        const ETUCInteractionState State = CurrentDirector->GetInteractionState(InteractionKind, nullptr, FragmentId);
        if (State == ETUCInteractionState::Done)
        {
            Multiplier = 0.12f;
        }
        else if (State == ETUCInteractionState::Available)
        {
            Multiplier = 0.9f + 0.25f * FMath::Sin(Time * 2.5f);
        }
    }
    AccentLight->SetIntensity(BaseLightIntensity * Multiplier);

    if (Mesh && Mesh->IsVisible())
    {
        if (!bAnimInitialized)
        {
            bAnimInitialized = true;
            MeshBaseLocation = Mesh->GetRelativeLocation();
            MeshBaseScale = Mesh->GetRelativeScale3D();
            MeshBaseRotation = Mesh->GetRelativeRotation();
        }
        if (InteractionKind == ETUCInteractionKind::MemoryFragment)
        {
            SpinYaw = FMath::Fmod(SpinYaw + DeltaSeconds * 70.0f, 360.0f);
            Mesh->SetRelativeRotation(MeshBaseRotation + FRotator(0.0f, SpinYaw, 0.0f));
            Mesh->SetRelativeLocation(MeshBaseLocation + FVector(0.0f, 0.0f, 8.0f + 8.0f * FMath::Sin(Time * 2.2f)));
        }
        else
        {
            const bool bCurrent = CurrentDirector->IsCurrentObjective(InteractionKind);
            const float Breath = bCurrent ? 1.0f + 0.035f * FMath::Sin(Time * 4.0f) : 1.0f;
            Mesh->SetRelativeScale3D(MeshBaseScale * Breath);
        }
    }
}

bool ATUCInteractionActor::CanInteract_Implementation(ATUCCharacterBase* Interactor) const
{
    if (bConsumed)
    {
        return false;
    }
    const ATUCPuzzleDirector* CurrentDirector = ResolveDirector();
    return CurrentDirector && CurrentDirector->CanPerform(InteractionKind, Interactor, FragmentId);
}

void ATUCInteractionActor::Interact_Implementation(ATUCCharacterBase* Interactor)
{
    ATUCPuzzleDirector* CurrentDirector = ResolveDirector();
    if (!CurrentDirector || !CanInteract_Implementation(Interactor))
    {
        return;
    }

    CurrentDirector->Perform(InteractionKind, Interactor, FragmentId);
    if (InteractionKind == ETUCInteractionKind::MemoryFragment)
    {
        SetConsumed(true);
    }
}

FText ATUCInteractionActor::GetBaseLabel() const
{
    switch (InteractionKind)
    {
        case ETUCInteractionKind::PowerConsole:    return FText::FromString(TEXT("Restaurar energía"));
        case ETUCInteractionKind::VentLatch:       return FText::FromString(TEXT("Abrir conducto"));
        case ETUCInteractionKind::OriginProjector: return FText::FromString(TEXT("Encender el proyector"));
        case ETUCInteractionKind::RetroFilm:       return FText::FromString(TEXT("Recuperar el rollo perdido"));
        case ETUCInteractionKind::RetroProjector:  return FText::FromString(TEXT("Proyectar el rollo"));
        case ETUCInteractionKind::FutureTerminal:  return FText::FromString(TEXT("Reactivar terminal"));
        case ETUCInteractionKind::FutureDuct:      return FText::FromString(TEXT("Cruzar el ducto de seguridad"));
        case ETUCInteractionKind::FutureProjector: return FText::FromString(TEXT("Proyectar: Archivo de los Ecos"));
        case ETUCInteractionKind::MemoryFragment:  return FText::FromString(TEXT("Recoger fragmento del Ancla"));
        case ETUCInteractionKind::ArchiveCore:     return FText::FromString(TEXT("Activar el núcleo de los ecos"));
        case ETUCInteractionKind::EndingIvo:       return FText::FromString(TEXT("Final A — sostener la puerta para que Ivo salga"));
        case ETUCInteractionKind::EndingNilo:      return FText::FromString(TEXT("Final B — sostener la puerta para que Nilo salga"));
        case ETUCInteractionKind::EndingTogether:  return FText::FromString(TEXT("Final C — activar el Ancla y salir juntos"));
        default:                                   return FText::FromString(TEXT("Interactuar"));
    }
}

FText ATUCInteractionActor::GetInteractionText_Implementation(ATUCCharacterBase* Interactor) const
{
    const ATUCPuzzleDirector* CurrentDirector = ResolveDirector();
    if (bConsumed || !CurrentDirector)
    {
        return FText::GetEmpty();
    }

    const FText Label = GetBaseLabel();
    switch (CurrentDirector->GetInteractionState(InteractionKind, Interactor, FragmentId))
    {
        case ETUCInteractionState::Available:
            return Label;
        case ETUCInteractionState::WrongCharacter:
        {
            ETUCCharacterType Required = ETUCCharacterType::Ivo;
            ATUCPuzzleDirector::GetRequiredCharacter(InteractionKind, Required);
            return FText::Format(FText::FromString(TEXT("{0} — cambia a {1} (Tab)")), Label,
                FText::FromString(Required == ETUCCharacterType::Ivo ? TEXT("IVO") : TEXT("NILO")));
        }
        case ETUCInteractionState::NotYet:
            return FText::Format(FText::FromString(TEXT("{0} — todavía no")), Label);
        case ETUCInteractionState::MissingAnchor:
            return FText::Format(FText::FromString(TEXT("{0} — Ancla {1}/{2}")), Label,
                FText::AsNumber(CurrentDirector->GetMemoryCount()), FText::AsNumber(TUC::FragmentCount));
        case ETUCInteractionState::Done:
        default:
            return FText::GetEmpty();
    }
}
