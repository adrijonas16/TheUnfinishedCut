#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TUCTypes.h"
#include "TUCCharacterBase.generated.h"

class UMaterialInterface;
class UStaticMeshComponent;

UCLASS(Abstract)
class THEUNFINISHEDCUT_API ATUCCharacterBase : public ACharacter
{
    GENERATED_BODY()

public:
    ATUCCharacterBase();

    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category="TUC|Characters")
    void SetPartner(ATUCCharacterBase* NewPartner);

    UFUNCTION(BlueprintPure, Category="TUC|Characters")
    ATUCCharacterBase* GetPartner() const { return Partner; }

    UFUNCTION(BlueprintPure, Category="TUC|Characters")
    ETUCCharacterType GetCharacterType() const { return CharacterType; }

    UFUNCTION(BlueprintCallable, Category="TUC|Characters")
    void ToggleFollow();

    UFUNCTION(BlueprintCallable, Category="TUC|Characters")
    void SetFollowPartner(bool bNewFollow) { bFollowPartner = bNewFollow; }

    UFUNCTION(BlueprintPure, Category="TUC|Characters")
    bool IsFollowingPartner() const { return bFollowPartner; }

    /** Texto del objeto enfocado y si se puede usar ahora mismo. */
    bool GetFocusedInteraction(FText& OutPrompt, bool& bOutCanInteract) const;

    /** Teletransporta junto al compañero (recuperación anti-atasco del GDD). */
    void RegroupNear(const ATUCCharacterBase* Anchor);

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TUC|Characters")
    ETUCCharacterType CharacterType = ETUCCharacterType::Ivo;

    /** Distancia máxima desde los ojos hasta el borde del objeto. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TUC|Interaction")
    float InteractionDistance = 230.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TUC|Companion")
    float FollowDistance = 240.0f;

    /** Más allá de esta distancia el compañero se reagrupa por teletransporte. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TUC|Companion")
    float RegroupDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TUC|Companion")
    bool bFollowPartner = true;

    /** Altura bajo la cual se considera que el personaje cayó fuera del set. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TUC|Safety")
    float FallResetZ = -700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TUC|Input")
    float GamepadTurnRate = 150.0f;

    UPROPERTY()
    TObjectPtr<ATUCCharacterBase> Partner;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BaseShapeMaterial;

    /** Crea una pieza visual de placeholder (sin colisión) con color plano. */
    UStaticMeshComponent* CreatePlaceholderPart(const FName& PartName, const TCHAR* MeshPath, const FVector& Location, const FRotator& Rotation, const FVector& Scale);
    void TintPart(UStaticMeshComponent* Part, const FLinearColor& Color);

    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void TurnAtRate(float Rate);
    void LookUpAtRate(float Rate);
    void TryInteract();
    void RequestCharacterSwitch();
    void RequestToggleFollow();
    void UpdateCompanionFollow(float DeltaSeconds);
    bool IsGameplayInputAllowed() const;
    AActor* FindFocusedInteractable() const;

    /** Malla visual que se balancea al caminar (las mallas del kit no tienen rig ni animaciones). */
    UPROPERTY()
    TObjectPtr<USceneComponent> BobComponent;

    float BobBaseZ = 0.0f;
    float BobAmplitude = 2.5f;
    float BobRate = 11.0f;

    void SetBobComponent(USceneComponent* Component, float Amplitude, float Rate);

private:
    float FallCheckCooldown = 0.0f;
    float BobPhase = 0.0f;
};
