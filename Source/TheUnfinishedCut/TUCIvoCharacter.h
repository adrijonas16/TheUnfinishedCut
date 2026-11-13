#pragma once

#include "CoreMinimal.h"
#include "TUCCharacterBase.h"
#include "TUCIvoCharacter.generated.h"

class UCameraComponent;
class USpotLightComponent;
class UAnimSequence;
class USkeletalMeshComponent;
class UStaticMeshComponent;

UCLASS()
class THEUNFINISHEDCUT_API ATUCIvoCharacter : public ATUCCharacterBase
{
    GENERATED_BODY()

public:
    ATUCIvoCharacter();

    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category="TUC|Camera")
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    UPROPERTY(VisibleAnywhere, Category="TUC|Tools")
    TObjectPtr<USpotLightComponent> Flashlight;

    /** Cuerpo de placeholder: invisible para Ivo en primera persona, visible cuando se controla a Nilo. */
    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<UStaticMeshComponent> BodyPlaceholder;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<UStaticMeshComponent> HeadPlaceholder;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<UStaticMeshComponent> FlashlightPlaceholder;

    /** Personaje realista animado de Meshy (esqueleto UE5 Mannequin: caminar y correr). */
    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<USkeletalMeshComponent> AnimatedBody;

    UPROPERTY()
    TObjectPtr<UAnimSequence> WalkAnim;

    UPROPERTY()
    TObjectPtr<UAnimSequence> RunAnim;

    UPROPERTY()
    TObjectPtr<UAnimSequence> CurrentAnim;

    bool TryUseAnimatedBody();
    void UpdateBodyAnimation();
    float HeadBobPhase = 0.0f;

    /** Modelo del kit (SM_Ivo). Invisible en su propia primera persona. */
    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<UStaticMeshComponent> KitBody;
};
