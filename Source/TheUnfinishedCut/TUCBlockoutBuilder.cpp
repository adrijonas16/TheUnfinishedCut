#include "TUCBlockoutBuilder.h"

#include "TUCAssets.h"
#include "TUCInteractionActor.h"
#include "TUCLayoutData.h"
#include "TUCPuzzleDirector.h"
#include "TUCTypes.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextRenderActor.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    // Colores de las cajas de respaldo (solo visibles si el kit no está importado).
    const FLinearColor StreetFloor(0.035f, 0.04f, 0.07f);
    const FLinearColor StreetWall(0.06f, 0.07f, 0.11f);
    const FLinearColor CinemaFloor(0.10f, 0.025f, 0.04f);
    const FLinearColor CinemaWall(0.16f, 0.04f, 0.06f);
    const FLinearColor RetroFloor(0.08f, 0.015f, 0.12f);
    const FLinearColor RetroWall(0.20f, 0.03f, 0.22f);
    const FLinearColor FutureFloor(0.02f, 0.06f, 0.09f);
    const FLinearColor FutureWall(0.04f, 0.12f, 0.18f);
    const FLinearColor ArchiveFloor(0.03f, 0.02f, 0.06f);
    const FLinearColor ArchiveWall(0.07f, 0.04f, 0.13f);

    const FLinearColor IvoCyan(0.1f, 0.8f, 1.0f);
    const FLinearColor NiloGreen(0.2f, 1.0f, 0.3f);
    const FLinearColor ProjectorViolet(0.6f, 0.15f, 1.0f);
    const FLinearColor AnchorYellow(1.0f, 0.8f, 0.15f);
    const FLinearColor EndingOrange(1.0f, 0.55f, 0.05f);

    constexpr float WallZ = 245.0f;
    constexpr float FloorZ = -55.0f;
}

ATUCBlockoutBuilder::ATUCBlockoutBuilder()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SceneRoot->SetMobility(EComponentMobility::Movable);
    SetRootComponent(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeAsset.Succeeded())
    {
        CubeMesh = CubeAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (ShapeMaterial.Succeeded())
    {
        BaseShapeMaterial = ShapeMaterial.Object;
    }
}

void ATUCBlockoutBuilder::SpawnRoomColliders(const FString& Prefix, float CenterX, float HalfLengthX, const FLinearColor& FloorColor, const FLinearColor& WallColor)
{
    const float LengthScale = HalfLengthX / 50.0f;
    SpawnCollider(Prefix + TEXT("Floor"), FVector(CenterX, 0.0f, FloorZ), FVector(LengthScale, 10.0f, 1.0f), FloorColor);
    SpawnCollider(Prefix + TEXT("LeftWall"), FVector(CenterX, -500.0f, WallZ), FVector(LengthScale, 0.5f, 6.0f), WallColor);
    SpawnCollider(Prefix + TEXT("RightWall"), FVector(CenterX, 500.0f, WallZ), FVector(LengthScale, 0.5f, 6.0f), WallColor);
    SpawnCollider(Prefix + TEXT("StartWall"), FVector(CenterX - HalfLengthX, 0.0f, WallZ), FVector(0.5f, 10.0f, 6.0f), WallColor);
    SpawnCollider(Prefix + TEXT("EndWall"), FVector(CenterX + HalfLengthX, 0.0f, WallZ), FVector(0.5f, 10.0f, 6.0f), WallColor);
}

void ATUCBlockoutBuilder::BuildWorld(ATUCPuzzleDirector* Director)
{
    if (bBuilt || !GetWorld())
    {
        return;
    }
    bBuilt = true;
    bKitAvailable = TUCAssets::IsKitAvailable();
    if (!bKitAvailable)
    {
        UE_LOG(LogTemp, Warning, TEXT("TUC: kit de assets no importado; se muestra el blockout. Ejecuta Tools/import_external_assets.py."));
    }

    // ── Colisión de los sets (invisible cuando hay kit) ───────────────────────
    SpawnCollider(TEXT("StreetFloor"), FVector(-700.0f, 0.0f, FloorZ), FVector(11.0f, 12.0f, 1.0f), StreetFloor);
    SpawnCollider(TEXT("StreetBackWall"), FVector(-1250.0f, 0.0f, WallZ), FVector(0.5f, 12.0f, 6.0f), StreetWall);
    SpawnCollider(TEXT("StreetLeftWall"), FVector(-625.0f, -600.0f, WallZ), FVector(12.5f, 0.5f, 6.0f), StreetWall);
    SpawnCollider(TEXT("StreetRightWall"), FVector(-625.0f, 600.0f, WallZ), FVector(12.5f, 0.5f, 6.0f), StreetWall);
    SpawnCollider(TEXT("CinemaFloor"), FVector(650.0f, 0.0f, FloorZ), FVector(16.0f, 12.0f, 1.0f), CinemaFloor);
    SpawnCollider(TEXT("CinemaLeftWall"), FVector(650.0f, -500.0f, WallZ), FVector(16.0f, 0.5f, 6.0f), CinemaWall);
    SpawnCollider(TEXT("CinemaRightWall"), FVector(650.0f, 500.0f, WallZ), FVector(16.0f, 0.5f, 6.0f), CinemaWall);
    SpawnCollider(TEXT("CinemaBackWall"), FVector(1450.0f, 0.0f, WallZ), FVector(0.5f, 10.0f, 6.0f), CinemaWall);
    SpawnCollider(TEXT("FacadeLeft"), FVector(0.0f, -400.0f, WallZ), FVector(0.5f, 4.0f, 6.0f), CinemaWall);
    SpawnCollider(TEXT("FacadeRight"), FVector(0.0f, 400.0f, WallZ), FVector(0.5f, 4.0f, 6.0f), CinemaWall);
    SpawnCollider(TEXT("FacadeLintel"), FVector(0.0f, 0.0f, 420.0f), FVector(0.5f, 4.0f, 2.5f), CinemaWall);
    SpawnRoomColliders(TEXT("Retro"), 2300.0f, 650.0f, RetroFloor, RetroWall);
    SpawnRoomColliders(TEXT("Future"), 3700.0f, 700.0f, FutureFloor, FutureWall);
    SpawnRoomColliders(TEXT("Archive"), 5100.0f, 650.0f, ArchiveFloor, ArchiveWall);

    for (const FTUCLayoutCollider& Entry : GTUCLayoutColliders)
    {
        SpawnCollider(Entry.Name, FVector(Entry.X, Entry.Y, Entry.Z), FVector(Entry.HX, Entry.HY, Entry.HZ) / 50.0f,
            FLinearColor(Entry.R, Entry.G, Entry.B));
    }

    // ── Decorado (kit TUC, instanciado) ───────────────────────────────────────
    if (bKitAvailable)
    {
        for (const FTUCLayoutVisual& Entry : GTUCLayoutVisuals)
        {
            if (UInstancedStaticMeshComponent* Instances = GetVisualComponent(Entry.Asset))
            {
                Instances->AddInstance(FTransform(FRotator(0.0f, Entry.Yaw, 0.0f), FVector(Entry.X, Entry.Y, Entry.Z), FVector(Entry.SX, Entry.SY, Entry.SZ)));
            }
        }
    }

    for (const FTUCLayoutLight& Entry : GTUCLayoutLights)
    {
        SpawnLight(FVector(Entry.X, Entry.Y, Entry.Z), FLinearColor(Entry.R, Entry.G, Entry.B), Entry.Intensity, Entry.Radius);
    }
    for (const FTUCLayoutSign& Entry : GTUCLayoutSigns)
    {
        SpawnSign(Entry.Text, FVector(Entry.X, Entry.Y, Entry.Z), Entry.Yaw, FColor(Entry.R, Entry.G, Entry.B), Entry.Size);
    }

    // ── Piezas interactivas ───────────────────────────────────────────────────
    SpawnInteraction(Director, ETUCInteractionKind::PowerConsole, FVector(260.0f, -422.0f, 30.0f), IvoCyan, FVector(0.45f, 0.35f, 0.7f), TEXT("SM_PowerConsole"), 90.0f);
    SpawnInteraction(Director, ETUCInteractionKind::VentLatch, FVector(900.0f, 466.0f, 6.0f), NiloGreen, FVector(0.7f, 0.18f, 0.22f), TEXT("SM_VentGrate"), -90.0f);
    SpawnInteraction(Director, ETUCInteractionKind::OriginProjector, FVector(1080.0f, -120.0f, 20.0f), ProjectorViolet, FVector(0.7f, 0.5f, 0.5f), nullptr, 0.0f);

    SpawnInteraction(Director, ETUCInteractionKind::RetroFilm, FVector(1900.0f, 350.0f, 22.0f), NiloGreen, FVector(0.35f, 0.12f, 0.55f), TEXT("SM_FilmReelStand"), -90.0f);
    SpawnInteraction(Director, ETUCInteractionKind::RetroProjector, FVector(2740.0f, 0.0f, 50.0f), ProjectorViolet, FVector(0.7f, 0.5f, 0.5f), nullptr, 0.0f);
    Fragments.Add(SpawnInteraction(Director, ETUCInteractionKind::MemoryFragment, FVector(2220.0f, -350.0f, 45.0f), AnchorYellow, FVector(0.18f, 0.18f, 0.55f), TEXT("SM_AnchorFragment"), 0.0f, 0));

    SpawnInteraction(Director, ETUCInteractionKind::FutureTerminal, FVector(3260.0f, -447.0f, 35.0f), IvoCyan, FVector(0.4f, 0.3f, 0.8f), TEXT("SM_Terminal"), 90.0f);
    SpawnInteraction(Director, ETUCInteractionKind::FutureDuct, FVector(3900.0f, 438.0f, 6.0f), NiloGreen, FVector(0.65f, 0.18f, 0.22f), TEXT("SM_DuctHatch"), -90.0f);
    SpawnInteraction(Director, ETUCInteractionKind::FutureProjector, FVector(4300.0f, -120.0f, 20.0f), ProjectorViolet, FVector(0.7f, 0.5f, 0.5f), nullptr, 0.0f);
    Fragments.Add(SpawnInteraction(Director, ETUCInteractionKind::MemoryFragment, FVector(3600.0f, 380.0f, 45.0f), AnchorYellow, FVector(0.18f, 0.18f, 0.55f), TEXT("SM_AnchorFragment"), 0.0f, 1));

    Fragments.Add(SpawnInteraction(Director, ETUCInteractionKind::MemoryFragment, FVector(4760.0f, -350.0f, 45.0f), AnchorYellow, FVector(0.18f, 0.18f, 0.55f), TEXT("SM_AnchorFragment"), 0.0f, 2));
    SpawnInteraction(Director, ETUCInteractionKind::ArchiveCore, FVector(5000.0f, 0.0f, 40.0f), FLinearColor::White, FVector(0.65f, 0.65f, 0.9f), TEXT("SM_ArchiveCore"), 0.0f);
    SpawnInteraction(Director, ETUCInteractionKind::EndingIvo, FVector(5400.0f, -260.0f, 30.0f), IvoCyan, FVector(0.55f, 0.55f, 0.7f), TEXT("SM_EndingPedestal"), 180.0f);
    SpawnInteraction(Director, ETUCInteractionKind::EndingTogether, FVector(5400.0f, 0.0f, 30.0f), AnchorYellow, FVector(0.55f, 0.55f, 0.7f), TEXT("SM_EndingPedestal"), 180.0f);
    SpawnInteraction(Director, ETUCInteractionKind::EndingNilo, FVector(5400.0f, 260.0f, 30.0f), EndingOrange, FVector(0.55f, 0.55f, 0.7f), TEXT("SM_EndingPedestal"), 180.0f);

    // ── Fractura del cine (visible tras el primer viaje) ──────────────────────
    if (AActor* PillarA = SpawnKitActor(TEXT("SM_Monolith"), FVector(1300.0f, -420.0f, -5.0f), 90.0f, FVector(0.3f, 0.3f, 2.4f), FLinearColor(0.6f, 0.05f, 0.5f)))
    {
        ParallelActors.Add(PillarA);
    }
    if (AActor* PillarB = SpawnKitActor(TEXT("SM_Monolith"), FVector(860.0f, -430.0f, -5.0f), 90.0f, FVector(0.2f, 0.2f, 2.0f), FLinearColor(0.05f, 0.5f, 0.6f)))
    {
        ParallelActors.Add(PillarB);
    }
    if (APointLight* FractureA = SpawnLight(FVector(1200.0f, 0.0f, 300.0f), FLinearColor(0.85f, 0.02f, 0.55f), 9000.0f, 1100.0f))
    {
        ParallelLights.Add(FractureA);
    }
    if (APointLight* FractureB = SpawnLight(FVector(700.0f, 200.0f, 160.0f), FLinearColor(0.0f, 0.8f, 1.0f), 6000.0f, 800.0f))
    {
        ParallelLights.Add(FractureB);
    }

    SpawnSky();

    // Aplicar estado guardado.
    for (ATUCInteractionActor* Fragment : Fragments)
    {
        if (Fragment && Director)
        {
            Fragment->SetConsumed(Director->IsFragmentCollected(Fragment->GetFragmentId()));
        }
    }
    SetParallelLook(Director && Director->IsParallelWorldActive());
}

void ATUCBlockoutBuilder::SpawnSky()
{
    // Cielo de crepúsculo sin assets externos: SkyAtmosphere del motor iluminado por un sol muy bajo.
    // Las salas no tienen techo, así que el cielo se ve sobre todos los sets.
    if (ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector(0.0f, 0.0f, 1000.0f), FRotator(-6.0f, 200.0f, 0.0f)))
    {
        Sun->GetRootComponent()->SetMobility(EComponentMobility::Movable);
        if (UDirectionalLightComponent* SunLight = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
        {
            SunLight->SetIntensity(2.5f);
            SunLight->SetLightColor(FLinearColor(1.0f, 0.62f, 0.5f));
            SunLight->SetAtmosphereSunLight(true);
        }
    }
    GetWorld()->SpawnActor<ASkyAtmosphere>(FVector::ZeroVector, FRotator::ZeroRotator);

    // Luz ambiental que captura el cielo en tiempo real (sin construir iluminación).
    const FTransform SkyTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 600.0f));
    if (ASkyLight* Sky = GetWorld()->SpawnActorDeferred<ASkyLight>(ASkyLight::StaticClass(), SkyTransform))
    {
        if (USkyLightComponent* SkyComponent = Sky->GetLightComponent())
        {
            SkyComponent->SetMobility(EComponentMobility::Movable);
            SkyComponent->bRealTimeCapture = true;
            SkyComponent->Intensity = 1.0f;
        }
        Sky->FinishSpawning(SkyTransform);
    }

    GetWorld()->SpawnActor<AExponentialHeightFog>(FVector::ZeroVector, FRotator::ZeroRotator);
}

UInstancedStaticMeshComponent* ATUCBlockoutBuilder::GetVisualComponent(const TCHAR* AssetName)
{
    const FName Key(AssetName);
    if (const TObjectPtr<UInstancedStaticMeshComponent>* Found = VisualComponents.Find(Key))
    {
        return Found->Get();
    }

    UInstancedStaticMeshComponent* Instances = nullptr;
    if (UStaticMesh* KitMesh = TUCAssets::LoadMesh(TUCAssets::KitMesh(AssetName)))
    {
        Instances = NewObject<UInstancedStaticMeshComponent>(this, Key);
        Instances->SetMobility(EComponentMobility::Movable);
        Instances->SetStaticMesh(KitMesh);
        Instances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Instances->SetCanEverAffectNavigation(false);
        Instances->SetupAttachment(SceneRoot);
        Instances->RegisterComponent();
        AddInstanceComponent(Instances);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("TUC: falta el asset del kit %s"), AssetName);
    }

    VisualComponents.Add(Key, Instances);
    return Instances;
}

AActor* ATUCBlockoutBuilder::SpawnCollider(const FString& Name, const FVector& Location, const FVector& Scale, const FLinearColor& Color)
{
    if (!CubeMesh)
    {
        return nullptr;
    }

    AStaticMeshActor* Block = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (Block)
    {
#if WITH_EDITOR
        Block->SetActorLabel(FString(TEXT("COL_")) + Name);
#endif
        UStaticMeshComponent* BlockMesh = Block->GetStaticMeshComponent();
        BlockMesh->SetMobility(EComponentMobility::Movable);
        BlockMesh->SetStaticMesh(CubeMesh);
        Block->SetActorScale3D(Scale);

        if (bKitAvailable)
        {
            // Solo colisión: el arte lo ponen las mallas del kit.
            BlockMesh->SetVisibility(false);
            BlockMesh->SetCastShadow(false);
        }
        else if (BaseShapeMaterial)
        {
            if (UMaterialInstanceDynamic* Tint = UMaterialInstanceDynamic::Create(BaseShapeMaterial, Block))
            {
                Tint->SetVectorParameterValue(TEXT("Color"), Color);
                BlockMesh->SetMaterial(0, Tint);
            }
        }
    }
    return Block;
}

AActor* ATUCBlockoutBuilder::SpawnKitActor(const TCHAR* AssetName, const FVector& Location, float Yaw, const FVector& FallbackScale, const FLinearColor& FallbackColor)
{
    UStaticMesh* KitMesh = bKitAvailable ? TUCAssets::LoadMesh(TUCAssets::KitMesh(AssetName)) : nullptr;
    if (!KitMesh)
    {
        return SpawnCollider(AssetName, Location + FVector(0.0f, 0.0f, FallbackScale.Z * 50.0f), FallbackScale, FallbackColor);
    }

    AStaticMeshActor* Prop = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator(0.0f, Yaw, 0.0f));
    if (Prop)
    {
        Prop->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
        Prop->GetStaticMeshComponent()->SetStaticMesh(KitMesh);
    }
    return Prop;
}

ATUCInteractionActor* ATUCBlockoutBuilder::SpawnInteraction(ATUCPuzzleDirector* Director, ETUCInteractionKind Kind, const FVector& Location, const FLinearColor& Color, const FVector& Scale, const TCHAR* KitAsset, float Yaw, int32 FragmentId)
{
    ATUCInteractionActor* Interaction = GetWorld()->SpawnActor<ATUCInteractionActor>(Location, FRotator::ZeroRotator);
    if (Interaction)
    {
        Interaction->Configure(Director, Kind, Color, Scale, FragmentId, bKitAvailable ? KitAsset : nullptr, Yaw);
    }
    return Interaction;
}

APointLight* ATUCBlockoutBuilder::SpawnLight(const FVector& Location, const FLinearColor& Color, float Intensity, float Radius)
{
    APointLight* Light = GetWorld()->SpawnActor<APointLight>(Location, FRotator::ZeroRotator);
    if (Light)
    {
        // Movable: sin lightmaps construidos, una luz Stationary generada en runtime muestra avisos.
        Light->GetRootComponent()->SetMobility(EComponentMobility::Movable);
        Light->PointLightComponent->SetLightColor(Color);
        Light->PointLightComponent->SetIntensity(Intensity);
        Light->PointLightComponent->SetAttenuationRadius(Radius);
    }
    return Light;
}

void ATUCBlockoutBuilder::SpawnSign(const FString& Text, const FVector& Location, float Yaw, const FColor& Color, float WorldSize)
{
    ATextRenderActor* Sign = GetWorld()->SpawnActor<ATextRenderActor>(Location, FRotator(0.0f, Yaw, 0.0f));
    if (Sign && Sign->GetTextRender())
    {
        UTextRenderComponent* TextRender = Sign->GetTextRender();
        TextRender->SetText(FText::FromString(Text));
        TextRender->SetTextRenderColor(Color);
        TextRender->SetWorldSize(WorldSize);
        TextRender->SetHorizontalAlignment(EHTA_Center);
        TextRender->SetVerticalAlignment(EVRTA_TextCenter);
    }
}

void ATUCBlockoutBuilder::SetParallelLook(bool bEnabled)
{
    for (AActor* Actor : ParallelActors)
    {
        if (Actor)
        {
            Actor->SetActorHiddenInGame(!bEnabled);
            Actor->SetActorEnableCollision(bEnabled);
        }
    }
    for (APointLight* Light : ParallelLights)
    {
        if (Light && Light->PointLightComponent)
        {
            Light->PointLightComponent->SetVisibility(bEnabled);
        }
    }
}

void ATUCBlockoutBuilder::ResetWorld()
{
    for (ATUCInteractionActor* Fragment : Fragments)
    {
        if (Fragment)
        {
            Fragment->SetConsumed(false);
        }
    }
    SetParallelLook(false);
}
