#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TUCTypes.h"
#include "TUCPuzzleDirector.generated.h"

class ATUCCharacterBase;

/**
 * Fuente única de verdad de la campaña: paso actual, fragmentos del Ancla, final y flags narrativos.
 * La lógica cambia primero aquí y después se aplican efectos (contrato de 10_CONTRATOS_ESTADOS).
 */
UCLASS()
class THEUNFINISHEDCUT_API ATUCPuzzleDirector : public AActor
{
    GENERATED_BODY()

public:
    ATUCPuzzleDirector();

    static ATUCPuzzleDirector* Get(const UObject* WorldContextObject);

    /** Carga el guardado. Idempotente: el GameMode la llama explícitamente antes de construir el mundo. */
    void InitializeFromSave();

    /** Borra el guardado y vuelve al paso 0 (nueva partida sin recargar el nivel). */
    void ResetProgress();

    bool CanPerform(ETUCInteractionKind Kind, const ATUCCharacterBase* Character, int32 FragmentId = INDEX_NONE) const;
    ETUCInteractionState GetInteractionState(ETUCInteractionKind Kind, const ATUCCharacterBase* Character, int32 FragmentId = INDEX_NONE) const;
    void Perform(ETUCInteractionKind Kind, ATUCCharacterBase* Character, int32 FragmentId = INDEX_NONE);

    /** true si esta interacción es la que hace avanzar la campaña ahora mismo (para resaltar su luz). */
    bool IsCurrentObjective(ETUCInteractionKind Kind) const;

    /** Personaje necesario para la interacción, o false si cualquiera sirve. */
    static bool GetRequiredCharacter(ETUCInteractionKind Kind, ETUCCharacterType& OutType);

    UFUNCTION(BlueprintPure, Category="TUC|Puzzle")
    bool IsParallelWorldActive() const { return ProgressStep >= 3; }

    UFUNCTION(BlueprintPure, Category="TUC|Puzzle")
    int32 GetProgressStep() const { return ProgressStep; }

    UFUNCTION(BlueprintPure, Category="TUC|Puzzle")
    FText GetCurrentObjective() const;

    UFUNCTION(BlueprintPure, Category="TUC|Puzzle")
    FText GetChapterTitle() const;

    UFUNCTION(BlueprintPure, Category="TUC|Puzzle")
    int32 GetMemoryCount() const { return MemoryCount; }

    bool IsFragmentCollected(int32 FragmentId) const;

    UFUNCTION(BlueprintPure, Category="TUC|Puzzle")
    ETUCEnding GetEnding() const { return Ending; }

    UFUNCTION(BlueprintPure, Category="TUC|Puzzle")
    FText GetEndingTitle() const;

    UFUNCTION(BlueprintPure, Category="TUC|Puzzle")
    FText GetEndingEpilogue() const;

    bool HasSeenPrologue() const { return bPrologueSeen; }
    void MarkPrologueSeen();

    /** Mensajes en pantalla (sustituyen a los mensajes de depuración, que no aparecen en Shipping). */
    void PushMessage(const FString& Message, const FLinearColor& Color, float Duration = 5.0f);
    bool GetActiveMessage(FText& OutMessage, FLinearColor& OutColor, float& OutAlpha) const;

private:
    UPROPERTY(VisibleAnywhere, Category="TUC|Puzzle")
    int32 ProgressStep = 0;

    UPROPERTY(VisibleAnywhere, Category="TUC|Puzzle")
    int32 MemoryCount = 0;

    UPROPERTY(VisibleAnywhere, Category="TUC|Puzzle")
    int32 FragmentMask = 0;

    UPROPERTY(VisibleAnywhere, Category="TUC|Puzzle")
    ETUCEnding Ending = ETUCEnding::None;

    UPROPERTY(VisibleAnywhere, Category="TUC|Puzzle")
    bool bPrologueSeen = false;

    bool bInitialized = false;

    FString MessageText;
    FLinearColor MessageColor = FLinearColor::White;
    double MessageStartTime = -100.0;
    float MessageDuration = 0.0f;

    static int32 GetRequiredStep(ETUCInteractionKind Kind);
    void AdvanceTo(int32 NewStep, const FString& Message, const FLinearColor& Color, bool bTravel);
    void SaveProgress() const;
};
