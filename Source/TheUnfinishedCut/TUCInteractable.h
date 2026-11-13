#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TUCTypes.h"
#include "TUCInteractable.generated.h"

class ATUCCharacterBase;

UINTERFACE(Blueprintable)
class THEUNFINISHEDCUT_API UTheUnfinishedCutInteractable : public UInterface
{
    GENERATED_BODY()
};

class THEUNFINISHEDCUT_API ITheUnfinishedCutInteractable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TUC|Interaction")
    bool CanInteract(ATUCCharacterBase* Interactor) const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TUC|Interaction")
    void Interact(ATUCCharacterBase* Interactor);

    /** Texto del prompt. Si la interacción no está disponible, explica qué falta (personaje, orden o Ancla). */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TUC|Interaction")
    FText GetInteractionText(ATUCCharacterBase* Interactor) const;
};
