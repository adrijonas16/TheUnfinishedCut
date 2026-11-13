#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TUCGameMode.generated.h"

class ATUCBlockoutBuilder;
class ATUCCharacterBase;
class ATUCPuzzleDirector;

UCLASS()
class THEUNFINISHEDCUT_API ATUCGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATUCGameMode();
    virtual void BeginPlay() override;

    void ActivateParallelWorld();

    void TravelToCampaignStage(int32 Stage);

    UFUNCTION(BlueprintCallable, Category="TUC|Checkpoint")
    void ResetPlayersToCheckpoint();

    /** Nueva partida en el mismo mundo: borra guardado, restaura piezas y vuelve al inicio. */
    void RestartCampaign();

    /** Un personaje cayó fuera del set: el jugador rebobina al checkpoint, el compañero se reagrupa. */
    void HandleCharacterFell(ATUCCharacterBase* Character);

    /** Tras un rebobinado, las patrullas no detectan durante un instante (evita bucles de detección). */
    bool IsInCheckpointGrace() const;

private:
    UPROPERTY()
    TObjectPtr<ATUCBlockoutBuilder> BlockoutBuilder;

    UPROPERTY()
    TObjectPtr<ATUCPuzzleDirector> PuzzleDirector;

    double LastCheckpointResetTime = -100.0;

    static void GetCheckpoint(int32 Step, FVector& OutIvoLocation, float& OutYaw);
    void SpawnWatchers();
};
