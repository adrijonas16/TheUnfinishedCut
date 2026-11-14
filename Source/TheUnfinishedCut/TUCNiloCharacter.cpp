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

    CatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CatMesh"));
    CatMesh->SetupAttachment(GetCapsuleComponent());
    CatMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CatMesh->SetVisibility(false);

    AnimatedCat = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("AnimatedCat"));
    AnimatedCat->SetupAttachment(GetCapsuleComponent());
    AnimatedCat->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AnimatedCat->SetVisibility(false);

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
        FVector(19.0f, 0.0f, -1.0f), FRotator(-65.0f, 0.0f, 0.0f), FVector(0.18f, 0.18f, 0.035f));
    PlaceholderBell = CreatePlaceholderPart(TEXT("PlaceholderBell"), SphereShape,
        FVector(25.0f, 0.0f, -6.0f), FRotator::ZeroRotator, FVector(0.055f, 0.055f, 0.055f));

    // ───────── Rig articulado low-poly de Nilo (fiel a SM_Nilo del mockup, con 4 patas y cola animadas) ─────────
    NiloRigRoot = CreateDefaultSubobject<USceneComponent>(TEXT("NiloRigRoot"));
    NiloRigRoot->SetupAttachment(GetCapsuleComponent());
    NiloRigRoot->SetRelativeLocation(FVector(0.0f, 0.0f, -34.0f)); // Base de las patas en -34 cm

    LegFLPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LegFLPivot"));
    LegFLPivot->SetupAttachment(NiloRigRoot);
    LegFLPivot->SetRelativeLocation(FVector(14.0f, -6.5f, 18.0f));

    LegFRPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LegFRPivot"));
    LegFRPivot->SetupAttachment(NiloRigRoot);
    LegFRPivot->SetRelativeLocation(FVector(14.0f, 6.5f, 18.0f));

    LegBLPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LegBLPivot"));
    LegBLPivot->SetupAttachment(NiloRigRoot);
    LegBLPivot->SetRelativeLocation(FVector(-14.0f, -6.5f, 18.0f));

    LegBRPivot = CreateDefaultSubobject<USceneComponent>(TEXT("LegBRPivot"));
    LegBRPivot->SetupAttachment(NiloRigRoot);
    LegBRPivot->SetRelativeLocation(FVector(-14.0f, 6.5f, 18.0f));

    TailPivot = CreateDefaultSubobject<USceneComponent>(TEXT("TailPivot"));
    TailPivot->SetupAttachment(NiloRigRoot);
    TailPivot->SetRelativeLocation(FVector(-21.0f, 0.0f, 26.0f));

    auto AddCatPart = [this](const TCHAR* Name, USceneComponent* Parent, const TCHAR* ShapePath, const FVector& Loc, const FRotator& Rot, const FVector& Scale, const TCHAR* MatTag)
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
        Comp->ComponentTags.Add(FName(MatTag));
        NiloRigParts.Add(Comp);
        return Comp;
    };

    // 4 patas articuladas (delanteras con calcetines blancos, traseras negras)
    AddCatPart(TEXT("Nilo_LegFL"), LegFLPivot, CylinderShape, FVector(0.0f, 0.0f, -8.5f), FRotator::ZeroRotator, FVector(0.06f, 0.06f, 0.17f), TEXT("M_CatBlack"));
    AddCatPart(TEXT("Nilo_PawFL"), LegFLPivot, SphereShape, FVector(1.5f, 0.0f, -16.5f), FRotator::ZeroRotator, FVector(0.075f, 0.06f, 0.045f), TEXT("M_CatWhite"));

    AddCatPart(TEXT("Nilo_LegFR"), LegFRPivot, CylinderShape, FVector(0.0f, 0.0f, -8.5f), FRotator::ZeroRotator, FVector(0.06f, 0.06f, 0.17f), TEXT("M_CatBlack"));
    AddCatPart(TEXT("Nilo_PawFR"), LegFRPivot, SphereShape, FVector(1.5f, 0.0f, -16.5f), FRotator::ZeroRotator, FVector(0.075f, 0.06f, 0.045f), TEXT("M_CatWhite"));

    AddCatPart(TEXT("Nilo_LegBL"), LegBLPivot, CylinderShape, FVector(0.0f, 0.0f, -8.5f), FRotator::ZeroRotator, FVector(0.06f, 0.06f, 0.17f), TEXT("M_CatBlack"));
    AddCatPart(TEXT("Nilo_PawBL"), LegBLPivot, SphereShape, FVector(1.5f, 0.0f, -16.5f), FRotator::ZeroRotator, FVector(0.075f, 0.06f, 0.045f), TEXT("M_CatBlack"));

    AddCatPart(TEXT("Nilo_LegBR"), LegBRPivot, CylinderShape, FVector(0.0f, 0.0f, -8.5f), FRotator::ZeroRotator, FVector(0.06f, 0.06f, 0.17f), TEXT("M_CatBlack"));
    AddCatPart(TEXT("Nilo_PawBR"), LegBRPivot, SphereShape, FVector(1.5f, 0.0f, -16.5f), FRotator::ZeroRotator, FVector(0.075f, 0.06f, 0.045f), TEXT("M_CatBlack"));

    // Cola curvada hacia arriba con punta blanca (relativa a TailPivot)
    AddCatPart(TEXT("Nilo_Tail1"), TailPivot, CylinderShape, FVector(-6.0f, 0.0f, 6.0f), FRotator(-42.0f, 0.0f, 0.0f), FVector(0.048f, 0.048f, 0.18f), TEXT("M_CatBlack"));
    AddCatPart(TEXT("Nilo_Tail2"), TailPivot, CylinderShape, FVector(-12.5f, 0.0f, 18.5f), FRotator(-18.0f, 0.0f, 0.0f), FVector(0.045f, 0.045f, 0.18f), TEXT("M_CatBlack"));
    AddCatPart(TEXT("Nilo_TailTip"), TailPivot, SphereShape, FVector(-14.5f, 0.0f, 27.5f), FRotator::ZeroRotator, FVector(0.055f, 0.055f, 0.055f), TEXT("M_CatWhite"));

    // Cuerpo esmoquin, pecho blanco, cabeza, hocico blanco, nariz rosa, ojos ámbar, orejas, collar rojo y campanilla dorada
    AddCatPart(TEXT("Nilo_Body"), NiloRigRoot, SphereShape, FVector(0.0f, 0.0f, 24.0f), FRotator::ZeroRotator, FVector(0.48f, 0.22f, 0.22f), TEXT("M_CatBlack"));
    AddCatPart(TEXT("Nilo_Chest"), NiloRigRoot, SphereShape, FVector(14.5f, 0.0f, 22.0f), FRotator::ZeroRotator, FVector(0.19f, 0.18f, 0.20f), TEXT("M_CatWhite"));
    AddCatPart(TEXT("Nilo_Head"), NiloRigRoot, SphereShape, FVector(27.0f, 0.0f, 38.0f), FRotator::ZeroRotator, FVector(0.20f, 0.19f, 0.18f), TEXT("M_CatBlack"));
    AddCatPart(TEXT("Nilo_Muzzle"), NiloRigRoot, SphereShape, FVector(34.5f, 0.0f, 35.0f), FRotator::ZeroRotator, FVector(0.085f, 0.10f, 0.075f), TEXT("M_CatWhite"));
    AddCatPart(TEXT("Nilo_Nose"), NiloRigRoot, SphereShape, FVector(38.3f, 0.0f, 36.5f), FRotator::ZeroRotator, FVector(0.026f, 0.03f, 0.022f), TEXT("M_CatPink"));

    AddCatPart(TEXT("Nilo_EyeL"), NiloRigRoot, SphereShape, FVector(34.5f, -4.2f, 40.0f), FRotator::ZeroRotator, FVector(0.034f, 0.044f, 0.044f), TEXT("M_CatEye"));
    AddCatPart(TEXT("Nilo_PupilL"), NiloRigRoot, SphereShape, FVector(36.0f, -4.2f, 40.0f), FRotator::ZeroRotator, FVector(0.014f, 0.014f, 0.032f), TEXT("M_Black"));
    AddCatPart(TEXT("Nilo_EyeR"), NiloRigRoot, SphereShape, FVector(34.5f, 4.2f, 40.0f), FRotator::ZeroRotator, FVector(0.034f, 0.044f, 0.044f), TEXT("M_CatEye"));
    AddCatPart(TEXT("Nilo_PupilR"), NiloRigRoot, SphereShape, FVector(36.0f, 4.2f, 40.0f), FRotator::ZeroRotator, FVector(0.014f, 0.014f, 0.032f), TEXT("M_Black"));

    AddCatPart(TEXT("Nilo_EarL"), NiloRigRoot, ConeShape, FVector(25.0f, -5.6f, 47.0f), FRotator(0.0f, 0.0f, -12.0f), FVector(0.08f, 0.08f, 0.09f), TEXT("M_CatBlack"));
    AddCatPart(TEXT("Nilo_EarInnerL"), NiloRigRoot, ConeShape, FVector(26.2f, -5.4f, 46.5f), FRotator(0.0f, 0.0f, -12.0f), FVector(0.044f, 0.044f, 0.055f), TEXT("M_CatPink"));
    AddCatPart(TEXT("Nilo_EarR"), NiloRigRoot, ConeShape, FVector(25.0f, 5.6f, 47.0f), FRotator(0.0f, 0.0f, 12.0f), FVector(0.08f, 0.08f, 0.09f), TEXT("M_CatBlack"));
    AddCatPart(TEXT("Nilo_EarInnerR"), NiloRigRoot, ConeShape, FVector(26.2f, 5.4f, 46.5f), FRotator(0.0f, 0.0f, 12.0f), FVector(0.044f, 0.044f, 0.055f), TEXT("M_CatPink"));

    // Collar rojo y campanilla dorada
    AddCatPart(TEXT("Nilo_Collar"), NiloRigRoot, CylinderShape, FVector(20.5f, 0.0f, 31.5f), FRotator(-60.0f, 0.0f, 0.0f), FVector(0.165f, 0.165f, 0.032f), TEXT("M_Collar"));
    AddCatPart(TEXT("Nilo_Bell"), NiloRigRoot, SphereShape, FVector(26.2f, 0.0f, 26.2f), FRotator::ZeroRotator, FVector(0.045f, 0.045f, 0.045f), TEXT("M_Gold"));

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

    PlaceholderBody->SetVisibility(false);
    PlaceholderHead->SetVisibility(false);
    PlaceholderEarL->SetVisibility(false);
    PlaceholderEarR->SetVisibility(false);
    PlaceholderTail->SetVisibility(false);
    PlaceholderCollar->SetVisibility(false);
    PlaceholderBell->SetVisibility(false);
    CatMesh->SetVisibility(false);
    AnimatedCat->SetVisibility(false);

    // Aplicar materiales del kit (MI_M_CatBlack, MI_M_CatWhite, MI_M_CatPink, MI_M_CatEye, MI_M_Collar, MI_M_Gold)
    for (UStaticMeshComponent* Part : NiloRigParts)
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
            FLinearColor Fallback(0.02f, 0.02f, 0.025f);
            if (Tag == TEXT("M_CatWhite")) Fallback = FLinearColor(0.90f, 0.90f, 0.88f);
            else if (Tag == TEXT("M_CatPink")) Fallback = FLinearColor(0.85f, 0.42f, 0.48f);
            else if (Tag == TEXT("M_CatEye")) Fallback = FLinearColor(1.0f, 0.62f, 0.08f);
            else if (Tag == TEXT("M_Collar")) Fallback = FLinearColor(0.80f, 0.03f, 0.03f);
            else if (Tag == TEXT("M_Gold")) Fallback = FLinearColor(0.90f, 0.65f, 0.15f);
            TintPart(Part, Fallback);
        }
    }
}

void ATUCNiloCharacter::UpdateProceduralCatWalk(float DeltaSeconds)
{
    const float Speed = GetVelocity().Size2D();
    const float Alpha = FMath::Clamp(Speed / 360.0f, 0.0f, 1.0f);
    if (Alpha > 0.02f)
    {
        CatWalkPhase += DeltaSeconds * (11.0f + 7.0f * Alpha);
    }
    else
    {
        CatWalkPhase = FMath::FInterpTo(CatWalkPhase, 0.0f, DeltaSeconds, 10.0f);
    }

    // Paso diagonal felino: (FL + BR) en fase, (FR + BL) en contrafase
    const float SwingA = FMath::Sin(CatWalkPhase) * 32.0f * Alpha;
    const float SwingB = -SwingA;
    const float BounceZ = FMath::Abs(FMath::Sin(CatWalkPhase)) * 1.8f * Alpha;

    // La cola se balancea suavemente incluso en reposo (vida extra para Nilo)
    const float IdleTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    const float TailSway = FMath::Sin(IdleTime * 3.2f + CatWalkPhase * 0.5f) * (10.0f + 12.0f * Alpha);

    if (LegFLPivot) LegFLPivot->SetRelativeRotation(FRotator(SwingA, 0.0f, 0.0f));
    if (LegBRPivot) LegBRPivot->SetRelativeRotation(FRotator(SwingA, 0.0f, 0.0f));
    if (LegFRPivot) LegFRPivot->SetRelativeRotation(FRotator(SwingB, 0.0f, 0.0f));
    if (LegBLPivot) LegBLPivot->SetRelativeRotation(FRotator(SwingB, 0.0f, 0.0f));
    if (TailPivot) TailPivot->SetRelativeRotation(FRotator(0.0f, 0.0f, TailSway));
    if (NiloRigRoot) NiloRigRoot->SetRelativeLocation(FVector(0.0f, 0.0f, -34.0f + BounceZ));
}

bool ATUCNiloCharacter::TryUseImportedCat()
{
    return false;
}

bool ATUCNiloCharacter::TryUseAnimatedCat()
{
    return false;
}

void ATUCNiloCharacter::UpdateCatAnimation()
{
}

void ATUCNiloCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateProceduralCatWalk(DeltaSeconds);
}
