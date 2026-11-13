#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TUCTypes.h"
#include "TUCSaveGame.generated.h"

UCLASS()
class THEUNFINISHEDCUT_API UTUCSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    /** 1 = v1.0 (sin máscara de fragmentos). 2 = v1.1. El valor por defecto es 1 para que
        los guardados antiguos (que no tienen este campo) se reconozcan como v1.0. */
    UPROPERTY(SaveGame)
    int32 SaveVersion = 1;

    UPROPERTY(SaveGame)
    int32 ProgressStep = 0;

    UPROPERTY(SaveGame)
    int32 MemoryCount = 0;

    /** Bit N = fragmento N recogido. Evita recoger el mismo fragmento otra vez tras recargar. */
    UPROPERTY(SaveGame)
    int32 FragmentMask = 0;

    /** Flag narrativo preview_seen: el prólogo CORTE 001 ya se mostró. */
    UPROPERTY(SaveGame)
    bool bPrologueSeen = false;

    UPROPERTY(SaveGame)
    ETUCEnding Ending = ETUCEnding::None;
};
