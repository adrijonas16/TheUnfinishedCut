#include "TUCCharacterBase.h"

#include "TUCGameMode.h"
#include "TUCInteractable.h"
#include "TUCInteractionActor.h"
#include "TUCPlayerController.h"
#include "TUCPuzzleDirector.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ATUCCharacterBase::ATUCCharacterBase()
{
    PrimaryActorTick.bCanEverTick = true;
    AutoPossessAI = EAutoPossessAI::Disabled;
    GetCharacterMovement()->bRunPhysicsWithNoController = true;

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (ShapeMaterial.Succeeded())
    {
        BaseShapeMaterial = ShapeMaterial.Object;
    }
}

UStaticMeshComponent* ATUCCharacterBase::CreatePlaceholderPart(const FName& PartName, const TCHAR* MeshPath, const FVector& Location, const FRotator& Rotation, const FVector& Scale)
{
    // Solo puede llamarse desde constructores (CreateDefaultSubobject / ConstructorHelpers).
    UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(PartName);
    Part->SetupAttachment(GetCapsuleComponent());
    Part->SetRelativeLocation(Location);
    Part->SetRelativeRotation(Rotation);
    Part->SetRelativeScale3D(Scale);
    Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Part->SetGenerateOverlapEvents(false);
    Part->SetCanEverAffectNavigation(false);

    ConstructorHelpers::FObjectFinder<UStaticMesh> ShapeMesh(MeshPath);
    if (ShapeMesh.Succeeded())
    {
        Part->SetStaticMesh(ShapeMesh.Object);
    }
    return Part;
}

void ATUCCharacterBase::TintPart(UStaticMeshComponent* Part, const FLinearColor& Color)
{
    if (!Part || !BaseShapeMaterial)
    {
        return;
    }
    if (UMaterialInstanceDynamic* Tint = UMaterialInstanceDynamic::Create(BaseShapeMaterial, this))
    {
        Tint->SetVectorParameterValue(TEXT("Color"), Color);
        Part->SetMaterial(0, Tint);
    }
}

void ATUCCharacterBase::SetBobComponent(USceneComponent* Component, float Amplitude, float Rate)
{
    BobComponent = Component;
    BobAmplitude = Amplitude;
    BobRate = Rate;
    BobBaseZ = Component ? Component->GetRelativeLocation().Z : 0.0f;
}

void ATUCCharacterBase::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateCompanionFollow(DeltaSeconds);

    // Balanceo al caminar en lugar de animación esquelética.
    if (BobComponent)
    {
        const float Speed = GetVelocity().Size2D();
        const float SpeedAlpha = FMath::Clamp(Speed / 400.0f, 0.0f, 1.0f);
        BobPhase += DeltaSeconds * BobRate * (0.4f + SpeedAlpha);
        FVector Relative = BobComponent->GetRelativeLocation();
        Relative.Z = BobBaseZ + FMath::Abs(FMath::Sin(BobPhase)) * BobAmplitude * SpeedAlpha;
        BobComponent->SetRelativeLocation(Relative);
    }

    FallCheckCooldown = FMath::Max(0.0f, FallCheckCooldown - DeltaSeconds);
    if (FallCheckCooldown <= 0.0f && GetActorLocation().Z < FallResetZ)
    {
        FallCheckCooldown = 1.0f;
        if (ATUCGameMode* GameMode = Cast<ATUCGameMode>(UGameplayStatics::GetGameMode(this)))
        {
            GameMode->HandleCharacterFell(this);
        }
    }
}

void ATUCCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    PlayerInputComponent->BindAxis("MoveForward", this, &ATUCCharacterBase::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ATUCCharacterBase::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &ATUCCharacterBase::Turn);
    PlayerInputComponent->BindAxis("LookUp", this, &ATUCCharacterBase::LookUp);
    PlayerInputComponent->BindAxis("TurnRate", this, &ATUCCharacterBase::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUpRate", this, &ATUCCharacterBase::LookUpAtRate);
    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ATUCCharacterBase::TryInteract);
    PlayerInputComponent->BindAction("SwitchCharacter", IE_Pressed, this, &ATUCCharacterBase::RequestCharacterSwitch);
    PlayerInputComponent->BindAction("ToggleFollow", IE_Pressed, this, &ATUCCharacterBase::RequestToggleFollow);
}

void ATUCCharacterBase::SetPartner(ATUCCharacterBase* NewPartner)
{
    Partner = NewPartner;
}

void ATUCCharacterBase::MoveForward(float Value)
{
    if (!FMath::IsNearlyZero(Value))
    {
        AddMovementInput(GetActorForwardVector(), Value);
    }
}

void ATUCCharacterBase::MoveRight(float Value)
{
    if (!FMath::IsNearlyZero(Value))
    {
        AddMovementInput(GetActorRightVector(), Value);
    }
}

void ATUCCharacterBase::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void ATUCCharacterBase::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}

void ATUCCharacterBase::TurnAtRate(float Rate)
{
    if (!FMath::IsNearlyZero(Rate) && GetWorld())
    {
        AddControllerYawInput(Rate * GamepadTurnRate * GetWorld()->GetDeltaSeconds());
    }
}

void ATUCCharacterBase::LookUpAtRate(float Rate)
{
    if (!FMath::IsNearlyZero(Rate) && GetWorld())
    {
        AddControllerPitchInput(Rate * GamepadTurnRate * GetWorld()->GetDeltaSeconds());
    }
}

bool ATUCCharacterBase::IsGameplayInputAllowed() const
{
    const ATUCPlayerController* TUCController = Cast<ATUCPlayerController>(GetController());
    return TUCController && !TUCController->IsIntroVisible() && !TUCController->IsPauseMenuVisible();
}

void ATUCCharacterBase::TryInteract()
{
    if (!IsGameplayInputAllowed())
    {
        return;
    }

    AActor* Target = FindFocusedInteractable();
    if (!Target)
    {
        return;
    }

    if (ITheUnfinishedCutInteractable::Execute_CanInteract(Target, this))
    {
        ITheUnfinishedCutInteractable::Execute_Interact(Target, this);
    }
    else if (ATUCPuzzleDirector* Director = ATUCPuzzleDirector::Get(this))
    {
        const FText Reason = ITheUnfinishedCutInteractable::Execute_GetInteractionText(Target, this);
        if (!Reason.IsEmpty())
        {
            Director->PushMessage(Reason.ToString(), FLinearColor(1.0f, 0.55f, 0.2f), 2.5f);
        }
    }
}

AActor* ATUCCharacterBase::FindFocusedInteractable() const
{
    const UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    // Selección por proximidad + orientación de la vista. No depende de la altura exacta
    // de la cámara ni de apuntar al píxel (la v1.0 trazaba una línea horizontal por encima
    // de todas las piezas y ningún objetivo era alcanzable).
    const FVector Eye = GetPawnViewLocation();
    const FRotator ViewRotation = IsPlayerControlled() ? GetControlRotation() : GetActorRotation();
    FVector View2D = ViewRotation.Vector();
    View2D.Z = 0.0f;
    View2D = View2D.GetSafeNormal();

    FCollisionQueryParams Params(SCENE_QUERY_STAT(TUCInteractionSight), false, this);
    if (Partner)
    {
        Params.AddIgnoredActor(Partner);
    }

    AActor* Best = nullptr;
    float BestScore = -TNumericLimits<float>::Max();

    for (TActorIterator<ATUCInteractionActor> It(World); It; ++It)
    {
        ATUCInteractionActor* Candidate = *It;
        if (!Candidate || Candidate->IsHidden() || Candidate->IsConsumed())
        {
            continue;
        }

        const FBox Box = Candidate->GetComponentsBoundingBox();
        if (!Box.IsValid)
        {
            continue;
        }

        const float Distance = FVector::Dist(Eye, Box.GetClosestPointTo(Eye));
        if (Distance > InteractionDistance)
        {
            continue;
        }

        FVector ToCenter = Box.GetCenter() - Eye;
        ToCenter.Z = 0.0f;
        const float Facing = ToCenter.IsNearlyZero() ? 1.0f : FVector::DotProduct(View2D, ToCenter.GetSafeNormal());
        if (Facing < 0.3f && Distance > 70.0f)
        {
            continue;
        }

        FHitResult Hit;
        if (World->LineTraceSingleByChannel(Hit, Eye, Box.GetCenter(), ECC_Visibility, Params) && Hit.GetActor() != Candidate)
        {
            continue;
        }

        const float Score = Facing - 0.5f * (Distance / FMath::Max(1.0f, InteractionDistance));
        if (Score > BestScore)
        {
            BestScore = Score;
            Best = Candidate;
        }
    }

    return Best;
}

bool ATUCCharacterBase::GetFocusedInteraction(FText& OutPrompt, bool& bOutCanInteract) const
{
    OutPrompt = FText::GetEmpty();
    bOutCanInteract = false;

    AActor* Target = FindFocusedInteractable();
    if (!Target)
    {
        return false;
    }

    ATUCCharacterBase* MutableThis = const_cast<ATUCCharacterBase*>(this);
    OutPrompt = ITheUnfinishedCutInteractable::Execute_GetInteractionText(Target, MutableThis);
    bOutCanInteract = ITheUnfinishedCutInteractable::Execute_CanInteract(Target, MutableThis);
    return !OutPrompt.IsEmpty();
}

void ATUCCharacterBase::RequestCharacterSwitch()
{
    if (!IsGameplayInputAllowed())
    {
        return;
    }
    if (ATUCPlayerController* TUCController = Cast<ATUCPlayerController>(GetController()))
    {
        TUCController->SwitchCharacter();
    }
}

void ATUCCharacterBase::RequestToggleFollow()
{
    if (IsGameplayInputAllowed() && Partner)
    {
        Partner->ToggleFollow();
    }
}

void ATUCCharacterBase::ToggleFollow()
{
    bFollowPartner = !bFollowPartner;
    if (ATUCPuzzleDirector* Director = ATUCPuzzleDirector::Get(this))
    {
        const FString Who = CharacterType == ETUCCharacterType::Ivo ? TEXT("IVO") : TEXT("NILO");
        Director->PushMessage(FString::Printf(TEXT("%s: %s"), *Who, bFollowPartner ? TEXT("TE SIGUE") : TEXT("ESPERA AQUÍ")),
            FLinearColor(0.4f, 0.9f, 1.0f), 2.0f);
    }
}

void ATUCCharacterBase::UpdateCompanionFollow(float DeltaSeconds)
{
    if (IsPlayerControlled() || !Partner || !bFollowPartner)
    {
        return;
    }

    const FVector ToPartner = Partner->GetActorLocation() - GetActorLocation();
    const float Distance2D = ToPartner.Size2D();

    // Recuperación: si quedó muy lejos o en otro nivel de altura, se reagrupa (02_GDD, IA acompañante).
    if (Distance2D > RegroupDistance || FMath::Abs(ToPartner.Z) > 400.0f)
    {
        RegroupNear(Partner);
        return;
    }

    if (Distance2D > FollowDistance)
    {
        const FVector Direction = ToPartner.GetSafeNormal2D();
        const float Speed = Distance2D > FollowDistance * 2.0f ? 1.0f : 0.6f;
        GetCharacterMovement()->AddInputVector(Direction * Speed, true);
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), Direction.Rotation(), DeltaSeconds, 7.0f));
    }
}

void ATUCCharacterBase::RegroupNear(const ATUCCharacterBase* Anchor)
{
    if (!Anchor)
    {
        return;
    }

    const FVector Base = Anchor->GetActorLocation();
    const FVector Back = -Anchor->GetActorForwardVector() * 130.0f;
    const FVector Side = Anchor->GetActorRightVector() * 90.0f;
    const float FeetZ = Base.Z - Anchor->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const float MyHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

    const FVector Offsets[] = { Back + Side, Back - Side, Side * 1.4f, -Side * 1.4f, Back * 1.3f };
    for (const FVector& Offset : Offsets)
    {
        FVector Destination = Base + Offset;
        Destination.Z = FeetZ + MyHalfHeight + 5.0f;
        if (TeleportTo(Destination, Anchor->GetActorRotation(), false, false))
        {
            GetCharacterMovement()->StopMovementImmediately();
            return;
        }
    }

    FVector Fallback = Base + Back;
    Fallback.Z = FeetZ + MyHalfHeight + 5.0f;
    TeleportTo(Fallback, Anchor->GetActorRotation(), false, true);
    GetCharacterMovement()->StopMovementImmediately();
}
