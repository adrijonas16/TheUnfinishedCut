#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TUCInteractable.h"
#include "TUCTypes.h"
#include "TUCInteractionActor.generated.h"

class UBoxComponent;
class UMaterialInterface;
class UPointLightComponent;
class UStaticMeshComponent;
class ATUCPuzzleDirector;

/**
 * Pieza interactuable de la campaña. La caja raíz da colisión y volumen de selección;
 * la malla es solo visual (cubo o asset importado ajustado por sus bounds).
 */
UCLASS()
class THEUNFINISHEDCUT_API ATUCInteractionActor : public AActor, public ITheUnfinishedCutInteractable
{
    GENERATED_BODY()

public:
    ATUCInteractionActor();

    virtual void Tick(float DeltaSeconds) override;

    /** Location = centro del volumen; Scale = tamaño en unidades de cubo (1 = 100 cm). */
    /** KitAsset: malla del kit (p. ej. TEXT("SM_Terminal")) o nullptr para usar el cubo de respaldo. */
    void Configure(ATUCPuzzleDirector* InDirector, ETUCInteractionKind NewKind, const FLinearColor& NewColor, const FVector& Scale,
        int32 InFragmentId = INDEX_NONE, const TCHAR* KitAsset = nullptr, float KitYaw = 0.0f);

    /** Oculta el fragmento ya recogido (o lo restaura en una partida nueva). */
    void SetConsumed(bool bNewConsumed);

    ETUCInteractionKind GetKind() const { return InteractionKind; }
    int32 GetFragmentId() const { return FragmentId; }
    bool IsConsumed() const { return bConsumed; }

    virtual bool CanInteract_Implementation(ATUCCharacterBase* Interactor) const override;
    virtual void Interact_Implementation(ATUCCharacterBase* Interactor) override;
    virtual FText GetInteractionText_Implementation(ATUCCharacterBase* Interactor) const override;

private:
    UPROPERTY(VisibleAnywhere, Category="TUC|Interaction")
    TObjectPtr<UBoxComponent> InteractionBox;

    UPROPERTY(VisibleAnywhere, Category="TUC|Interaction")
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY(VisibleAnywhere, Category="TUC|Interaction")
    TObjectPtr<UPointLightComponent> AccentLight;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BaseShapeMaterial;

    UPROPERTY(EditAnywhere, Category="TUC|Interaction")
    ETUCInteractionKind InteractionKind = ETUCInteractionKind::PowerConsole;

    UPROPERTY(VisibleAnywhere, Category="TUC|Interaction")
    int32 FragmentId = INDEX_NONE;

    TWeakObjectPtr<ATUCPuzzleDirector> Director;

    bool bConsumed = false;
    float BaseLightIntensity = 1800.0f;

    ATUCPuzzleDirector* ResolveDirector() const;
    FText GetBaseLabel() const;
    bool TryUseImportedProjector(const FVector& RequestedScale);
    bool TryUseKitMesh(const TCHAR* KitAsset, float KitYaw, const FVector& RequestedScale, const FLinearColor& AccentColor);
};
