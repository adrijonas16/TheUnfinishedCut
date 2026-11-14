#include "TUCIvoCharacter.h"

#include "TUCAssets.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/PackageName.h"

namespace
{
    const TCHAR* IvoMeshPackage = TEXT("/Game/TUC/Characters/Ivo/SK_Ivo");
    const TCHAR* IvoMaterialPackage = TEXT("/Game/TUC/Characters/Ivo/M_Ivo_PBR");
    const TCHAR* IvoWalkPackage = TEXT("/Game/TUC/Characters/Ivo/A_Ivo_Walk");
    const TCHAR* IvoRunPackage = TEXT("/Game/TUC/Characters/Ivo/A_Ivo_Run");
    const TCHAR* CubePath = TEXT("/Engine/BasicShapes/Cube.Cube");
    const TCHAR* CylPath = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
    const TCHAR* SpherePath = TEXT("/Engine/BasicShapes/Sphere.Sphere");
}

ATUCIvoCharacter::ATUCIvoCharacter()
{
    CharacterType = ETUCCharacterType::Ivo;
    GetCapsuleComponent()->InitCapsuleSize(42.0f, 92.0f);
    bUseControllerRotationYaw = true;
    BaseEyeHeight = 70.0f;
    InteractionDistance = 230.0f;

    FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
    FirstPersonCamera->SetRelativeLocation(FVector(-10.0f, 0.0f, 72.0f));
    FirstPersonCamera->bUsePawnControlRotation = true;

    Flashlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flashlight"));
    Flashlight->SetupAttachment(FirstPersonCamera);
    Flashlight->SetRelativeLocation(FVector(42.0f, 10.5f, -9.5f));
    Flashlight->Intensity = 8500.0f;
    Flashlight->AttenuationRadius = 1400.0f;
    Flashlight->InnerConeAngle = 18.0f;
    Flashlight->OuterConeAngle = 32.0f;
    Flashlight->SetLightColor(FLinearColor(1.0f, 0.86f, 0.62f));

    // Mano + linterna low-poly en primera persona (solo las ve el propio jugador cuando controla a Ivo).
    auto CreateFPPart = [this](const TCHAR* Name, const TCHAR* ShapePath, const FVector& Loc, const FRotator& Rot, const FVector& Scale)
    {
        UStaticMeshComponent* Comp = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Comp->SetupAttachment(FirstPersonCamera);
        if (UStaticMesh* Shape = LoadObject<UStaticMesh>(nullptr, ShapePath))
        {
            Comp->SetStaticMesh(Shape);
        }
        Comp->SetRelativeLocation(Loc);
        Comp->SetRelativeRotation(Rot);
        Comp->SetRelativeScale3D(Scale);
        Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Comp->CastShadow = false;
        Comp->SetOnlyOwnerSee(true);
        return Comp;
    };
    FPHandMesh = CreateFPPart(TEXT("FPHandMesh"), CubePath,
        FVector(22.0f, 15.0f, -15.0f), FRotator(10.0f, -12.0f, -8.0f), FVector(0.18f, 0.085f, 0.085f));
    FPFlashlightBody = CreateFPPart(TEXT("FPFlashlightBody"), CylPath,
        FVector(28.0f, 13.5f, -12.5f), FRotator(-80.0f, -12.0f, 0.0f), FVector(0.055f, 0.055f, 0.22f));
    FPFlashlightHead = CreateFPPart(TEXT("FPFlashlightHead"), CylPath,
        FVector(38.5f, 11.2f, -10.6f), FRotator(-80.0f, -12.0f, 0.0f), FVector(0.078f, 0.078f, 0.055f));

    // Placeholder clásico (se oculta porque usamos el rig articulado low-poly de Ivo).
    BodyPlaceholder = CreatePlaceholderPart(TEXT("BodyPlaceholder"), CylPath,
        FVector(0.0f, 0.0f, -18.0f), FRotator::ZeroRotator, FVector(0.62f, 0.62f, 1.48f));
    HeadPlaceholder = CreatePlaceholderPart(TEXT("HeadPlaceholder"), SpherePath,
        FVector(0.0f, 0.0f, 72.0f), FRotator::ZeroRotator, FVector(0.34f));
    FlashlightPlaceholder = CreatePlaceholderPart(TEXT("FlashlightPlaceholder"), CylPath,
        FVector(34.0f, 22.0f, 22.0f), FRotator(-90.0f, 0.0f, 0.0f), FVector(0.07f, 0.07f, 0.3f));

    // ───────── Rig articulado low-poly de Ivo (fiel a SM_Ivo del mockup, con piernas y brazos animados) ─────────
    IvoRigRoot = CreateDefaultSubobject<USceneComponent>(TEXT("IvoRigRoot"));
    IvoRigRoot->SetupAttachment(GetCapsuleComponent());
    IvoRigRoot->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f)); // Base de los pies en -92 cm

    LeftLegPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LeftLegPivot"));
    LeftLegPivot->SetupAttachment(IvoRigRoot);
    LeftLegPivot->SetRelativeLocation(FVector(0.0f, -11.0f, 88.0f));

    RightLegPivot = CreateDefaultSubobject<USceneComponent>(TEXT("RightLegPivot"));
    RightLegPivot->SetupAttachment(IvoRigRoot);
    RightLegPivot->SetRelativeLocation(FVector(0.0f, 11.0f, 88.0f));

    LeftArmPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LeftArmPivot"));
    LeftArmPivot->SetupAttachment(IvoRigRoot);
    LeftArmPivot->SetRelativeLocation(FVector(0.0f, -25.5f, 145.0f));

    RightArmPivot = CreateDefaultSubobject<USceneComponent>(TEXT("RightArmPivot"));
    RightArmPivot->SetupAttachment(IvoRigRoot);
    RightArmPivot->SetRelativeLocation(FVector(0.0f, 25.5f, 145.0f));

    auto AddRigPart = [this](const TCHAR* Name, USceneComponent* Parent, const TCHAR* ShapePath, const FVector& Loc, const FRotator& Rot, const FVector& Scale, const TCHAR* MatTag)
    {
        UStaticMeshComponent* Comp = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Comp->SetupAttachment(Parent);
        if (UStaticMesh* Shape = LoadObject<UStaticMesh>(nullptr, ShapePath))
        {
            Comp->SetStaticMesh(Shape);
        }
        Comp->SetRelativeLocation(Loc);
        Comp->SetRelativeRotation(Rot);
        Comp->SetRelativeScale3D(Scale);
        Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Comp->SetOwnerNoSee(true);
        Comp->ComponentTags.Add(FName(MatTag));
        IvoRigParts.Add(Comp);
        return Comp;
    };

    // Pierna y zapato izquierdos (relativos a LeftLegPivot en Z=88)
    AddRigPart(TEXT("Ivo_LegL"), LeftLegPivot, CylPath, FVector(0.0f, 0.0f, -40.0f), FRotator::ZeroRotator, FVector(0.15f, 0.15f, 0.80f), TEXT("M_Jeans"));
    AddRigPart(TEXT("Ivo_ShoeL"), LeftLegPivot, CubePath, FVector(4.0f, 0.0f, -83.0f), FRotator::ZeroRotator, FVector(0.28f, 0.12f, 0.10f), TEXT("M_Black"));

    // Pierna y zapato derechos (relativos a RightLegPivot en Z=88)
    AddRigPart(TEXT("Ivo_LegR"), RightLegPivot, CylPath, FVector(0.0f, 0.0f, -40.0f), FRotator::ZeroRotator, FVector(0.15f, 0.15f, 0.80f), TEXT("M_Jeans"));
    AddRigPart(TEXT("Ivo_ShoeR"), RightLegPivot, CubePath, FVector(4.0f, 0.0f, -83.0f), FRotator::ZeroRotator, FVector(0.28f, 0.12f, 0.10f), TEXT("M_Black"));

    // Brazo y mano izquierdos (relativos a LeftArmPivot en Z=145)
    AddRigPart(TEXT("Ivo_ArmL"), LeftArmPivot, CylPath, FVector(0.0f, 0.0f, -27.0f), FRotator(0.0f, 0.0f, 4.0f), FVector(0.12f, 0.12f, 0.55f), TEXT("M_Jacket"));
    AddRigPart(TEXT("Ivo_HandL"), LeftArmPivot, SpherePath, FVector(0.0f, -1.5f, -58.0f), FRotator::ZeroRotator, FVector(0.10f, 0.09f, 0.12f), TEXT("M_Skin"));

    // Brazo, mano y linterna derechos (relativos a RightArmPivot en Z=145)
    AddRigPart(TEXT("Ivo_ArmR"), RightArmPivot, CylPath, FVector(0.0f, 0.0f, -27.0f), FRotator(0.0f, 0.0f, -4.0f), FVector(0.12f, 0.12f, 0.55f), TEXT("M_Jacket"));
    AddRigPart(TEXT("Ivo_HandR"), RightArmPivot, SpherePath, FVector(0.0f, 1.5f, -58.0f), FRotator::ZeroRotator, FVector(0.10f, 0.09f, 0.12f), TEXT("M_Skin"));
    AddRigPart(TEXT("Ivo_TorchR"), RightArmPivot, CylPath, FVector(10.0f, 1.5f, -58.0f), FRotator(-90.0f, 0.0f, 0.0f), FVector(0.06f, 0.06f, 0.24f), TEXT("M_Black"));

    // Cadera, Torso (chaqueta marrón), Camisa azul, Hombros, Cuello, Cabeza, Pelo y Ojos (relativos a IvoRigRoot)
    AddRigPart(TEXT("Ivo_Hips"), IvoRigRoot, CubePath, FVector(0.0f, 0.0f, 92.0f), FRotator::ZeroRotator, FVector(0.22f, 0.38f, 0.14f), TEXT("M_Jeans"));
    AddRigPart(TEXT("Ivo_Torso"), IvoRigRoot, CubePath, FVector(0.0f, 0.0f, 120.0f), FRotator::ZeroRotator, FVector(0.26f, 0.42f, 0.48f), TEXT("M_Jacket"));
    AddRigPart(TEXT("Ivo_Shirt"), IvoRigRoot, CubePath, FVector(13.5f, 0.0f, 125.0f), FRotator::ZeroRotator, FVector(0.02f, 0.14f, 0.36f), TEXT("M_Shirt"));
    AddRigPart(TEXT("Ivo_Shoulders"), IvoRigRoot, CubePath, FVector(0.0f, 0.0f, 147.0f), FRotator::ZeroRotator, FVector(0.24f, 0.46f, 0.08f), TEXT("M_Jacket"));
    AddRigPart(TEXT("Ivo_Neck"), IvoRigRoot, CylPath, FVector(0.0f, 0.0f, 155.0f), FRotator::ZeroRotator, FVector(0.11f, 0.11f, 0.10f), TEXT("M_Skin"));
    AddRigPart(TEXT("Ivo_Head"), IvoRigRoot, SpherePath, FVector(1.0f, 0.0f, 169.0f), FRotator::ZeroRotator, FVector(0.22f, 0.20f, 0.26f), TEXT("M_Skin"));
    AddRigPart(TEXT("Ivo_HairBack"), IvoRigRoot, SpherePath, FVector(-1.5f, 0.0f, 175.0f), FRotator::ZeroRotator, FVector(0.23f, 0.216f, 0.20f), TEXT("M_Hair"));
    AddRigPart(TEXT("Ivo_HairBangs"), IvoRigRoot, CubePath, FVector(7.0f, 0.0f, 179.0f), FRotator(20.0f, 0.0f, 0.0f), FVector(0.08f, 0.20f, 0.05f), TEXT("M_Hair"));
    AddRigPart(TEXT("Ivo_EyeL"), IvoRigRoot, SpherePath, FVector(10.0f, -4.0f, 170.0f), FRotator::ZeroRotator, FVector(0.024f, 0.024f, 0.028f), TEXT("M_Black"));
    AddRigPart(TEXT("Ivo_EyeR"), IvoRigRoot, SpherePath, FVector(10.0f, 4.0f, 170.0f), FRotator::ZeroRotator, FVector(0.024f, 0.024f, 0.028f), TEXT("M_Black"));
    AddRigPart(TEXT("Ivo_Nose"), IvoRigRoot, CubePath, FVector(11.5f, 0.0f, 166.0f), FRotator::ZeroRotator, FVector(0.03f, 0.025f, 0.04f), TEXT("M_Skin"));

    AnimatedBody = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AnimatedBody"));
    AnimatedBody->SetupAttachment(GetCapsuleComponent());
    AnimatedBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AnimatedBody->SetOwnerNoSee(true);
    AnimatedBody->SetVisibility(false);
    AnimatedBody->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

    KitBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KitBody"));
    KitBody->SetupAttachment(GetCapsuleComponent());
    KitBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    KitBody->SetOwnerNoSee(true);
    KitBody->SetVisibility(false);

    BodyPlaceholder->SetOwnerNoSee(true);
    HeadPlaceholder->SetOwnerNoSee(true);
    FlashlightPlaceholder->SetOwnerNoSee(true);
}

void ATUCIvoCharacter::BeginPlay()
{
    Super::BeginPlay();
    TintPart(FPHandMesh, FLinearColor(0.76f, 0.54f, 0.42f));
    TintPart(FPFlashlightBody, FLinearColor(0.05f, 0.05f, 0.07f));
    TintPart(FPFlashlightHead, FLinearColor(0.08f, 0.08f, 0.10f));

    BodyPlaceholder->SetVisibility(false);
    HeadPlaceholder->SetVisibility(false);
    FlashlightPlaceholder->SetVisibility(false);
    KitBody->SetVisibility(false);
    AnimatedBody->SetVisibility(false);

    // Aplicar materiales del kit (MI_M_Jacket, MI_M_Shirt, MI_M_Jeans, MI_M_Skin, MI_M_Hair, MI_M_Black) o colores de respaldo:
    for (UStaticMeshComponent* Part : IvoRigParts)
    {
        if (!Part || Part->ComponentTags.Num() == 0)
        {
            continue;
        }
        const FString Tag = Part->ComponentTags[0].ToString();
        const FString MatPath = FString::Printf(TEXT("/Game/TUC/Kit/Materials/MI_%s"), *Tag);
        if (UMaterialInterface* KitMat = TUCAssets::LoadAsset<UMaterialInterface>(*MatPath))
        {
            Part->SetMaterial(0, KitMat);
        }
        else
        {
            FLinearColor Fallback(0.5f, 0.5f, 0.5f);
            if (Tag == TEXT("M_Jacket")) Fallback = FLinearColor(0.30f, 0.17f, 0.08f);
            else if (Tag == TEXT("M_Shirt")) Fallback = FLinearColor(0.14f, 0.24f, 0.42f);
            else if (Tag == TEXT("M_Jeans")) Fallback = FLinearColor(0.08f, 0.11f, 0.20f);
            else if (Tag == TEXT("M_Skin")) Fallback = FLinearColor(0.72f, 0.50f, 0.38f);
            else if (Tag == TEXT("M_Hair")) Fallback = FLinearColor(0.16f, 0.09f, 0.05f);
            else if (Tag == TEXT("M_Black")) Fallback = FLinearColor(0.02f, 0.02f, 0.025f);
            TintPart(Part, Fallback);
        }
    }
}

void ATUCIvoCharacter::UpdateProceduralWalk(float DeltaSeconds)
{
    const bool bControllingIvo = IsPlayerControlled();
    if (FPHandMesh) FPHandMesh->SetVisibility(bControllingIvo);
    if (FPFlashlightBody) FPFlashlightBody->SetVisibility(bControllingIvo);
    if (FPFlashlightHead) FPFlashlightHead->SetVisibility(bControllingIvo);

    for (UStaticMeshComponent* Part : IvoRigParts)
    {
        if (Part)
        {
            Part->SetVisibility(!bControllingIvo);
        }
    }

    const float Speed = GetVelocity().Size2D();
    const float Alpha = FMath::Clamp(Speed / 380.0f, 0.0f, 1.0f);
    if (Alpha > 0.02f)
    {
        WalkCyclePhase += DeltaSeconds * (7.5f + 5.5f * Alpha);
    }
    else
    {
        // Suavizar de vuelta a pose de reposo cuando se detiene
        WalkCyclePhase = FMath::FInterpTo(WalkCyclePhase, 0.0f, DeltaSeconds, 10.0f);
    }

    const float Swing = FMath::Sin(WalkCyclePhase) * 34.0f * Alpha;
    const float ArmSwing = FMath::Sin(WalkCyclePhase) * 26.0f * Alpha;
    const float BounceZ = FMath::Abs(FMath::Sin(WalkCyclePhase)) * 3.5f * Alpha;

    if (LeftLegPivot) LeftLegPivot->SetRelativeRotation(FRotator(Swing, 0.0f, 0.0f));
    if (RightLegPivot) RightLegPivot->SetRelativeRotation(FRotator(-Swing, 0.0f, 0.0f));
    if (LeftArmPivot) LeftArmPivot->SetRelativeRotation(FRotator(-ArmSwing, 0.0f, 0.0f));
    if (RightArmPivot) RightArmPivot->SetRelativeRotation(FRotator(ArmSwing * 0.45f + 12.0f, 0.0f, 0.0f));
    if (IvoRigRoot) IvoRigRoot->SetRelativeLocation(FVector(0.0f, 0.0f, -92.0f + BounceZ));
}

bool ATUCIvoCharacter::TryUseAnimatedBody()
{
    return false;
}

void ATUCIvoCharacter::UpdateBodyAnimation()
{
}

void ATUCIvoCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateProceduralWalk(DeltaSeconds);

    // Balanceo de cámara al caminar (primera persona más viva).
    if (FirstPersonCamera)
    {
        const float Speed = GetVelocity().Size2D();
        const float Alpha = FMath::Clamp(Speed / 450.0f, 0.0f, 1.0f);
        HeadBobPhase += DeltaSeconds * (6.0f + 6.0f * Alpha);
        const float BobZ = FMath::Sin(HeadBobPhase * 2.0f) * 2.4f * Alpha;
        const float BobY = FMath::Sin(HeadBobPhase) * 1.4f * Alpha;
        FirstPersonCamera->SetRelativeLocation(FVector(-10.0f, BobY, 72.0f + BobZ));
    }
}
