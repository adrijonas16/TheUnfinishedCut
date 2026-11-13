#include "TUCWatcher.h"

#include "TUCAssets.h"
#include "TUCGameMode.h"
#include "TUCPuzzleDirector.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    const FLinearColor WatcherCalm(1.0f, 0.35f, 0.05f);
    const FLinearColor WatcherAlert(1.0f, 0.0f, 0.0f);
}

ATUCWatcher::ATUCWatcher()
{
    PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);
    Root->SetMobility(EComponentMobility::Movable);

    Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
    Body->SetupAttachment(Root);
    Body->SetMobility(EComponentMobility::Movable);
    Body->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    Body->SetRelativeScale3D(FVector(0.35f, 0.35f, 1.2f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    if (CylinderMesh.Succeeded())
    {
        Body->SetStaticMesh(CylinderMesh.Object);
    }

    // Visor frontal: indica hacia dónde mira el centinela.
    Visor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visor"));
    Visor->SetupAttachment(Root);
    Visor->SetMobility(EComponentMobility::Movable);
    Visor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Visor->SetRelativeLocation(FVector(17.0f, 0.0f, 42.0f));
    Visor->SetRelativeScale3D(FVector(0.06f, 0.26f, 0.08f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        Visor->SetStaticMesh(CubeMesh.Object);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (ShapeMaterial.Succeeded())
    {
        BaseShapeMaterial = ShapeMaterial.Object;
    }

    KitVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KitVisual"));
    KitVisual->SetupAttachment(Root);
    KitVisual->SetMobility(EComponentMobility::Movable);
    KitVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    KitVisual->SetVisibility(false);

    DetectionLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("DetectionLight"));
    DetectionLight->SetupAttachment(Root);
    DetectionLight->SetMobility(EComponentMobility::Movable);
    DetectionLight->SetRelativeLocation(FVector(0.0f, 0.0f, 85.0f));
    DetectionLight->SetLightColor(WatcherCalm);
    DetectionLight->Intensity = 2500.0f;
    DetectionLight->AttenuationRadius = 260.0f;
    DetectionLight->SetCastShadows(false);

    // Cono de visión proyectado al suelo: el jugador ve exactamente qué zona vigila.
    VisionCone = CreateDefaultSubobject<USpotLightComponent>(TEXT("VisionCone"));
    VisionCone->SetupAttachment(Root);
    VisionCone->SetMobility(EComponentMobility::Movable);
    VisionCone->SetRelativeLocation(FVector(10.0f, 0.0f, 60.0f));
    VisionCone->SetRelativeRotation(FRotator(-18.0f, 0.0f, 0.0f));
    VisionCone->SetLightColor(WatcherCalm);
    VisionCone->Intensity = 9000.0f;
    VisionCone->AttenuationRadius = 520.0f;
    VisionCone->InnerConeAngle = 40.0f;
    VisionCone->OuterConeAngle = 50.0f;
}

void ATUCWatcher::BeginPlay()
{
    Super::BeginPlay();

    if (!bConfigured)
    {
        PatrolA = GetActorLocation() + FVector(0.0f, -300.0f, 0.0f);
        PatrolB = GetActorLocation() + FVector(0.0f, 300.0f, 0.0f);
        bConfigured = true;
    }

    // Modelo (ojo centinela de Meshy o el del kit): 110 cm, flotando 15 cm sobre el suelo (la raíz está a 60 cm del suelo).
    if (TUCAssets::FitMesh(KitVisual, TUCAssets::LoadMesh(TUCAssets::KitMesh(TEXT("SM_Watcher"))), 110.0f, true, 0.0f, -45.0f))
    {
        Body->SetHiddenInGame(true);
        Visor->SetHiddenInGame(true);
        KitAccent = TUCAssets::TintAccent(KitVisual, WatcherCalm);
        KitBaseZ = KitVisual->GetRelativeLocation().Z;
    }

    if (BaseShapeMaterial)
    {
        if (UMaterialInstanceDynamic* BodyTint = UMaterialInstanceDynamic::Create(BaseShapeMaterial, this))
        {
            BodyTint->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.03f, 0.02f, 0.03f));
            Body->SetMaterial(0, BodyTint);
        }
        VisorTint = UMaterialInstanceDynamic::Create(BaseShapeMaterial, this);
        if (VisorTint)
        {
            VisorTint->SetVectorParameterValue(TEXT("Color"), WatcherCalm);
            Visor->SetMaterial(0, VisorTint);
        }
    }
}

void ATUCWatcher::ConfigurePatrol(const FVector& InPatrolA, const FVector& InPatrolB, int32 InMinStep, int32 InMaxStep)
{
    PatrolA = InPatrolA;
    PatrolB = InPatrolB;
    MinStep = InMinStep;
    MaxStep = InMaxStep;
    bConfigured = true;
    bTowardB = true;
    SetActorLocation(PatrolA);
    SetActorRotation((PatrolB - PatrolA).GetSafeNormal2D().Rotation());
}

void ATUCWatcher::SetWatcherActive(bool bNewActive)
{
    if (bActive == bNewActive)
    {
        return;
    }
    bActive = bNewActive;
    Detection = 0.0f;
    SetActorHiddenInGame(!bActive);
    SetActorEnableCollision(bActive);
    DetectionLight->SetVisibility(bActive);
    VisionCone->SetVisibility(bActive);
}

void ATUCWatcher::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    const ATUCPuzzleDirector* Director = ATUCPuzzleDirector::Get(this);
    const int32 Step = Director ? Director->GetProgressStep() : 0;
    SetWatcherActive(Step >= MinStep && Step <= MaxStep && Step < TUC::FinalStep);
    if (!bActive)
    {
        return;
    }

    UpdatePatrol(DeltaSeconds);
    UpdateDetection(DeltaSeconds);

    if (KitVisual && KitVisual->IsVisible())
    {
        HoverPhase += DeltaSeconds * 2.2f;
        FVector Relative = KitVisual->GetRelativeLocation();
        Relative.Z = KitBaseZ + FMath::Sin(HoverPhase) * 4.0f;
        KitVisual->SetRelativeLocation(Relative);
    }
}

void ATUCWatcher::UpdatePatrol(float DeltaSeconds)
{
    const FVector Target = bTowardB ? PatrolB : PatrolA;
    const FVector NewLocation = FMath::VInterpConstantTo(GetActorLocation(), Target, DeltaSeconds, PatrolSpeed);
    SetActorLocation(NewLocation);

    const FVector Heading = (Target - NewLocation).GetSafeNormal2D();
    if (!Heading.IsNearlyZero())
    {
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), Heading.Rotation(), DeltaSeconds, 5.0f));
    }

    if (FVector::DistSquared(NewLocation, Target) < FMath::Square(15.0f))
    {
        bTowardB = !bTowardB;
    }
}

bool ATUCWatcher::CanSeePlayer() const
{
    const APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    const UWorld* World = GetWorld();
    if (!PlayerPawn || !World)
    {
        return false;
    }

    const FVector Eye = GetActorLocation() + FVector(0.0f, 0.0f, 45.0f);
    const FVector PlayerPoint = PlayerPawn->GetActorLocation();
    const FVector ToPlayer = PlayerPoint - Eye;

    if (ToPlayer.SizeSquared2D() > FMath::Square(SightRange) || FMath::Abs(ToPlayer.Z) > 250.0f)
    {
        return false;
    }

    const float CosHalfAngle = FMath::Cos(FMath::DegreesToRadians(SightHalfAngle));
    if (FVector::DotProduct(GetActorForwardVector().GetSafeNormal2D(), ToPlayer.GetSafeNormal2D()) < CosHalfAngle)
    {
        return false;
    }

    // Paredes, mostradores, torres y piezas interactivas bloquean la visión.
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(TUCWatcherSight), false, this);
    if (World->LineTraceSingleByChannel(Hit, Eye, PlayerPoint, ECC_Visibility, Params))
    {
        return Hit.GetActor() == PlayerPawn;
    }
    return true;
}

void ATUCWatcher::UpdateDetection(float DeltaSeconds)
{
    ATUCGameMode* GameMode = Cast<ATUCGameMode>(UGameplayStatics::GetGameMode(this));
    const bool bGrace = GameMode && GameMode->IsInCheckpointGrace();

    if (!bGrace && CanSeePlayer())
    {
        Detection = FMath::Min(1.0f, Detection + DeltaSeconds / FMath::Max(0.05f, TimeToDetect));
    }
    else
    {
        Detection = FMath::Max(0.0f, Detection - DeltaSeconds * 0.8f);
    }

    const FLinearColor Current = FMath::Lerp(WatcherCalm, WatcherAlert, Detection);
    DetectionLight->SetLightColor(Current);
    VisionCone->SetLightColor(Current);
    DetectionLight->SetIntensity(2500.0f + 9000.0f * Detection);
    if (VisorTint)
    {
        VisorTint->SetVectorParameterValue(TEXT("Color"), Current);
    }
    if (KitAccent)
    {
        KitAccent->SetVectorParameterValue(TEXT("Tint"), Current);
    }

    if (Detection >= 1.0f)
    {
        Detection = 0.0f;
        if (ATUCPuzzleDirector* Director = ATUCPuzzleDirector::Get(this))
        {
            Director->PushMessage(TEXT("DETECTADO — la película rebobina al último punto seguro."), FLinearColor(1.0f, 0.15f, 0.15f), 3.0f);
        }
        if (GameMode)
        {
            GameMode->ResetPlayersToCheckpoint();
        }
    }
}
