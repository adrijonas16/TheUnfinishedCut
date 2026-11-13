#include "TUCIvoCharacter.h"

#include "TUCAssets.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/PackageName.h"

namespace
{
    const TCHAR* IvoMeshPackage = TEXT("/Game/TUC/Characters/Ivo/SK_Ivo");
    const TCHAR* IvoWalkPackage = TEXT("/Game/TUC/Characters/Ivo/A_Ivo_Walk");
    const TCHAR* IvoRunPackage = TEXT("/Game/TUC/Characters/Ivo/A_Ivo_Run");

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
    Flashlight->SetRelativeLocation(FVector(15.0f, 0.0f, -8.0f));
    Flashlight->Intensity = 8500.0f;
    Flashlight->AttenuationRadius = 1400.0f;
    Flashlight->InnerConeAngle = 18.0f;
    Flashlight->OuterConeAngle = 32.0f;
    Flashlight->SetLightColor(FLinearColor(1.0f, 0.86f, 0.62f));

    // Placeholder low-poly para verlo desde la cámara de Nilo (bOwnerNoSee lo oculta en primera persona).
    BodyPlaceholder = CreatePlaceholderPart(TEXT("BodyPlaceholder"), TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),
        FVector(0.0f, 0.0f, -18.0f), FRotator::ZeroRotator, FVector(0.62f, 0.62f, 1.48f));
    HeadPlaceholder = CreatePlaceholderPart(TEXT("HeadPlaceholder"), TEXT("/Engine/BasicShapes/Sphere.Sphere"),
        FVector(0.0f, 0.0f, 72.0f), FRotator::ZeroRotator, FVector(0.34f));
    FlashlightPlaceholder = CreatePlaceholderPart(TEXT("FlashlightPlaceholder"), TEXT("/Engine/BasicShapes/Cylinder.Cylinder"),
        FVector(34.0f, 22.0f, 22.0f), FRotator(-90.0f, 0.0f, 0.0f), FVector(0.07f, 0.07f, 0.3f));

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
    TintPart(BodyPlaceholder, FLinearColor(0.08f, 0.12f, 0.22f));
    TintPart(HeadPlaceholder, FLinearColor(0.55f, 0.38f, 0.28f));
    TintPart(FlashlightPlaceholder, FLinearColor(0.05f, 0.05f, 0.05f));

    // 1) Personaje realista animado (Meshy). 2) Modelo low-poly del kit. 3) Placeholder.
    const float FeetZ = -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    if (TryUseAnimatedBody())
    {
        BodyPlaceholder->SetVisibility(false);
        HeadPlaceholder->SetVisibility(false);
        FlashlightPlaceholder->SetVisibility(false);
    }
    else if (TUCAssets::FitMesh(KitBody, TUCAssets::LoadMesh(TUCAssets::KitMesh(TEXT("SM_Ivo"))), 182.0f, true, 0.0f, FeetZ))
    {
        BodyPlaceholder->SetVisibility(false);
        HeadPlaceholder->SetVisibility(false);
        FlashlightPlaceholder->SetVisibility(false);
        SetBobComponent(KitBody, 2.5f, 11.0f);
    }
}

bool ATUCIvoCharacter::TryUseAnimatedBody()
{
    USkeletalMesh* SkelMesh = TUCAssets::LoadAsset<USkeletalMesh>(IvoMeshPackage);
    if (!SkelMesh)
    {
        return false;
    }
    WalkAnim = TUCAssets::LoadAsset<UAnimSequence>(IvoWalkPackage);
    RunAnim = TUCAssets::LoadAsset<UAnimSequence>(IvoRunPackage);

    // Ajuste por bounds: 182 cm de alto, pies en la base de la cápsula.
    // Las mallas con esqueleto UE5 Mannequin miran a +Y: se giran -90° para mirar a +X.
    const FBoxSphereBounds Bounds = SkelMesh->GetBounds();
    const float Height = 2.0f * Bounds.BoxExtent.Z;
    if (Height <= KINDA_SMALL_NUMBER)
    {
        return false;
    }
    const float Fit = 182.0f / Height;
    const FRotator YawFix(0.0f, -90.0f, 0.0f);
    const FVector RotatedOrigin = YawFix.RotateVector(Bounds.Origin * Fit);
    const float FeetZ = -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    const float MeshBottom = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Fit;

    AnimatedBody->SetSkeletalMeshAsset(SkelMesh);
    AnimatedBody->SetRelativeScale3D(FVector(Fit));
    AnimatedBody->SetRelativeRotation(YawFix);
    AnimatedBody->SetRelativeLocation(FVector(-RotatedOrigin.X, -RotatedOrigin.Y, FeetZ - MeshBottom));
    AnimatedBody->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    AnimatedBody->SetVisibility(true);
    UpdateBodyAnimation();
    return true;
}

void ATUCIvoCharacter::UpdateBodyAnimation()
{
    if (!AnimatedBody || !AnimatedBody->IsVisible() || !WalkAnim)
    {
        return;
    }

    const float Speed = GetVelocity().Size2D();
    UAnimSequence* Wanted = (Speed > 380.0f && RunAnim) ? RunAnim.Get() : WalkAnim.Get();
    if (Wanted != CurrentAnim)
    {
        CurrentAnim = Wanted;
        AnimatedBody->PlayAnimation(Wanted, true);
    }

    // Quieto: primera pose de caminar congelada. En movimiento: velocidad de reproducción según el paso.
    if (Speed < 15.0f)
    {
        AnimatedBody->SetPlayRate(0.0f);
        AnimatedBody->SetPosition(0.0f, false);
    }
    else
    {
        const float Reference = (Wanted == RunAnim) ? 550.0f : 260.0f;
        AnimatedBody->SetPlayRate(FMath::Clamp(Speed / Reference, 0.5f, 1.6f));
    }
}

void ATUCIvoCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateBodyAnimation();
}
