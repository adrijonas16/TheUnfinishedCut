#pragma once

#include "CoreMinimal.h"
#include "TUCCharacterBase.h"
#include "TUCNiloCharacter.generated.h"

class UAnimSequence;
class UCameraComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class THEUNFINISHEDCUT_API ATUCNiloCharacter : public ATUCCharacterBase
{
    GENERATED_BODY()

public:
    ATUCNiloCharacter();

    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;

    /** Largo final del gato importado (cm), medido sobre su eje horizontal más largo. */
    UPROPERTY(EditDefaultsOnly, Category="TUC|Visual")
    float ImportedCatLength = 70.0f;

    /** Giro para que el frente del FBX mire a +X (Blender -Y → Unreal). */
    UPROPERTY(EditDefaultsOnly, Category="TUC|Visual")
    float ImportedCatYawOffset = -90.0f;

private:
    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<UStaticMeshComponent> CatMesh;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<UStaticMeshComponent> PlaceholderBody;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<UStaticMeshComponent> PlaceholderHead;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<UStaticMeshComponent> PlaceholderEarL;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<UStaticMeshComponent> PlaceholderEarR;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<UStaticMeshComponent> PlaceholderTail;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<UStaticMeshComponent> PlaceholderCollar;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<UStaticMeshComponent> PlaceholderBell;

    /** Rig procedural low-poly articulado de Nilo (4 patas, cola y cuerpo con ciclo de caminar felino). */
    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<USceneComponent> NiloRigRoot;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<USceneComponent> LegFLPivot;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<USceneComponent> LegFRPivot;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<USceneComponent> LegBLPivot;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<USceneComponent> LegBRPivot;

    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<USceneComponent> TailPivot;

    UPROPERTY()
    TArray<TObjectPtr<UStaticMeshComponent>> NiloRigParts;

    float CatWalkPhase = 0.0f;
    void UpdateProceduralCatWalk(float DeltaSeconds);

    UPROPERTY(VisibleAnywhere, Category="TUC|Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, Category="TUC|Camera")
    TObjectPtr<UCameraComponent> FollowCamera;

    /** Gato esmoquin animado (Meshy, CC0): caminar. */
    UPROPERTY(VisibleAnywhere, Category="TUC|Visual")
    TObjectPtr<USkeletalMeshComponent> AnimatedCat;

    UPROPERTY()
    TObjectPtr<UAnimSequence> WalkAnim;

    bool bAnimStarted = false;

    bool TryUseImportedCat();
    bool TryUseAnimatedCat();
    void UpdateCatAnimation();
};
