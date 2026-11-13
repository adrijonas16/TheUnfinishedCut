#include "TUCNiloCharacter.h"

#include "TUCAssets.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Misc/PackageName.h"

namespace
{
    const TCHAR* AnimatedCatPackage = TEXT("/Game/TUC/Characters/Nilo/SK_Nilo");
    const TCHAR* AnimatedCatWalkPackage = TEXT("/Game/TUC/Characters/Nilo/A_Nilo_Walk");
    const TCHAR* AnimatedCatFolder = TEXT("/Game/TUC/Characters/Nilo");
    const TCHAR* ImportedCatPackage = TEXT("/Game/TUC/Characters/Nilo/SM_Nilo_Base_Optimized");
    const TCHAR* SphereShape = TEXT("/Engine/BasicShapes/Sphere.Sphere");
    const TCHAR* ConeShape = TEXT("/Engine/BasicShapes/Cone.Cone");
    const TCHAR* CylinderShape = TEXT("/Engine/BasicShapes/Cylinder.Cylinder");
}

ATUCNiloCharacter::ATUCNiloCharacter()
{
    CharacterType = ETUCCharacterType::Nilo;
    GetCapsuleComponent()->InitCapsuleSize(24.0f, 34.0f);
    GetCharacterMovement()->MaxWalkSpeed = 520.0f;
    GetCharacterMovement()->MaxStepHeight = 35.0f;
    bUseControllerRotationYaw = true;
    BaseEyeHeight = 18.0f;
    InteractionDistance = 170.0f;
    FollowDistance = 200.0f;

    // Malla importada (se asigna en BeginPlay si el asset existe).
    CatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CatMesh"));
    CatMesh->SetupAttachment(GetCapsuleComponent());
    CatMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CatMesh->SetVisibility(false);

    AnimatedCat = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AnimatedCat"));
    AnimatedCat->SetupAttachment(GetCapsuleComponent());
    AnimatedCat->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AnimatedCat->SetVisibility(false);
    AnimatedCat->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

    // Placeholder low-poly de gato esmoquin con collar rojo (11_ARTE / mockup), sin depender de assets.
    PlaceholderBody = CreatePlaceholderPart(TEXT("PlaceholderBody"), SphereShape,
        FVector(-4.0f, 0.0f, -12.0f), FRotator::ZeroRotator, FVector(0.62f, 0.30f, 0.32f));
    PlaceholderHead = CreatePlaceholderPart(TEXT("PlaceholderHead"), SphereShape,
        FVector(30.0f, 0.0f, 8.0f), FRotator::ZeroRotator, FVector(0.24f, 0.24f, 0.22f));
    PlaceholderEarL = CreatePlaceholderPart(TEXT("PlaceholderEarL"), ConeShape,
        FVector(30.0f, -7.0f, 22.0f), FRotator(0.0f, 0.0f, -15.0f), FVector(0.07f, 0.07f, 0.1f));
    PlaceholderEarR = CreatePlaceholderPart(TEXT("PlaceholderEarR"), ConeShape,
        FVector(30.0f, 7.0f, 22.0f), FRotator(0.0f, 0.0f, 15.0f), FVector(0.07f, 0.07f, 0.1f));
    PlaceholderTail = CreatePlaceholderPart(TEXT("PlaceholderTail"), CylinderShape,
        FVector(-40.0f, 0.0f, 2.0f), FRotator(-40.0f, 0.0f, 0.0f), FVector(0.05f, 0.05f, 0.38f));
    PlaceholderCollar = CreatePlaceholderPart(TEXT("PlaceholderCollar"), CylinderShape,
        FVector(22.0f, 0.0f, 0.0f), FRotator(-70.0f, 0.0f, 0.0f), FVector(0.2f, 0.2f, 0.04f));

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetCapsuleComponent());
    CameraBoom->TargetArmLength = 300.0f;
    CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bDoCollisionTest = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
}

void ATUCNiloCharacter::BeginPlay()
{
    Super::BeginPlay();

    const FLinearColor Black(0.02f, 0.02f, 0.025f);
    TintPart(PlaceholderBody, Black);
    TintPart(PlaceholderHead, Black);
    TintPart(PlaceholderEarL, Black);
    TintPart(PlaceholderEarR, Black);
    TintPart(PlaceholderTail, Black);
    TintPart(PlaceholderCollar, FLinearColor(0.75f, 0.03f, 0.03f));

    if (TryUseAnimatedCat() || TryUseImportedCat())
    {
        PlaceholderBody->SetVisibility(false);
        PlaceholderHead->SetVisibility(false);
        PlaceholderEarL->SetVisibility(false);
        PlaceholderEarR->SetVisibility(false);
        PlaceholderTail->SetVisibility(false);
        PlaceholderCollar->SetVisibility(false);
    }
}

bool ATUCNiloCharacter::TryUseImportedCat()
{
    // 1) Nilo del kit TUC (gato esmoquin con collar y campanilla, mira a +X).
    const float FeetZ = -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    if (TUCAssets::FitMesh(CatMesh, TUCAssets::LoadMesh(TUCAssets::KitMesh(TEXT("SM_Nilo"))), ImportedCatLength, false, 0.0f, FeetZ))
    {
        SetBobComponent(CatMesh, 1.5f, 15.0f);
        return true;
    }

    // 2) Gato de Meshy (siamés, referencia de escala). Carga solo si el paquete existe.
    UStaticMesh* Cat = TUCAssets::LoadMesh(ImportedCatPackage);
    if (!Cat)
    {
        return false;
    }

    // Escala y apoyo calculados desde los bounds (m o cm, pivote donde sea).
    if (!TUCAssets::FitMesh(CatMesh, Cat, ImportedCatLength, false, ImportedCatYawOffset, FeetZ))
    {
        return false;
    }
    SetBobComponent(CatMesh, 1.5f, 15.0f);
    return true;
}

bool ATUCNiloCharacter::TryUseAnimatedCat()
{
    USkeletalMesh* CatSkeletal = TUCAssets::LoadAsset<USkeletalMesh>(AnimatedCatPackage);
    if (!CatSkeletal)
    {
        CatSkeletal = TUCAssets::FindFirstAsset<USkeletalMesh>(AnimatedCatFolder);
    }
    if (!CatSkeletal)
    {
        return false;
    }
    WalkAnim = TUCAssets::LoadAsset<UAnimSequence>(AnimatedCatWalkPackage);
    if (!WalkAnim)
    {
        WalkAnim = TUCAssets::FindFirstAsset<UAnimSequence>(AnimatedCatFolder);
    }
    if (WalkAnim && WalkAnim->GetSkeleton() != CatSkeletal->GetSkeleton())
    {
        UE_LOG(LogTemp, Warning, TEXT("TUC: la animación %s no usa el esqueleto de %s"), *WalkAnim->GetName(), *CatSkeletal->GetName());
        WalkAnim = nullptr;
    }
    UE_LOG(LogTemp, Log, TEXT("TUC: Nilo animado = %s, caminar = %s"), *CatSkeletal->GetName(), WalkAnim ? *WalkAnim->GetName() : TEXT("(ninguna)"));

    // 70 cm de largo, patas en la base de la cápsula, girado para mirar a +X (modelos Meshy miran a +Y).
    const FBoxSphereBounds Bounds = CatSkeletal->GetBounds();
    const float LongestHorizontal = 2.0f * FMath::Max(Bounds.BoxExtent.X, Bounds.BoxExtent.Y);
    if (LongestHorizontal <= KINDA_SMALL_NUMBER)
    {
        return false;
    }
    const float Fit = ImportedCatLength / LongestHorizontal;
    const FRotator YawFix(0.0f, -90.0f, 0.0f);
    const FVector RotatedOrigin = YawFix.RotateVector(Bounds.Origin * Fit);
    const float FeetZ = -GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
    const float MeshBottom = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Fit;

    AnimatedCat->SetSkeletalMeshAsset(CatSkeletal);
    AnimatedCat->SetRelativeScale3D(FVector(Fit));
    AnimatedCat->SetRelativeRotation(YawFix);
    AnimatedCat->SetRelativeLocation(FVector(-RotatedOrigin.X, -RotatedOrigin.Y, FeetZ - MeshBottom));
    AnimatedCat->SetAnimationMode(EAnimationMode::AnimationSingleNode);
    AnimatedCat->SetVisibility(true);
    CatMesh->SetVisibility(false);
    if (!WalkAnim)
    {
        // Sin animación importada: al menos balanceo al caminar.
        SetBobComponent(AnimatedCat, 2.0f, 15.0f);
    }
    UpdateCatAnimation();
    return true;
}

void ATUCNiloCharacter::UpdateCatAnimation()
{
    if (!AnimatedCat || !AnimatedCat->IsVisible() || !WalkAnim)
    {
        return;
    }
    if (!bAnimStarted)
    {
        AnimatedCat->PlayAnimation(WalkAnim, true);
        bAnimStarted = true;
    }

    const float Speed = GetVelocity().Size2D();
    if (Speed < 15.0f)
    {
        AnimatedCat->SetPlayRate(0.0f);
        AnimatedCat->SetPosition(0.0f, false);
    }
    else
    {
        AnimatedCat->SetPlayRate(FMath::Clamp(Speed / 300.0f, 0.6f, 2.2f));
    }
}

void ATUCNiloCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateCatAnimation();
}
