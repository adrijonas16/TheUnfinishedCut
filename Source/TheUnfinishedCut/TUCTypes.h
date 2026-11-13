#pragma once

#include "CoreMinimal.h"
#include "TUCTypes.generated.h"

UENUM(BlueprintType)
enum class ETUCCharacterType : uint8
{
    Ivo,
    Nilo
};

UENUM(BlueprintType)
enum class ETUCInteractionKind : uint8
{
    PowerConsole,
    VentLatch,
    OriginProjector,
    RetroFilm,
    RetroProjector,
    FutureTerminal,
    FutureDuct,
    FutureProjector,
    MemoryFragment,   // Fragmento del Ancla (requisito del final C)
    ArchiveCore,
    EndingIvo,        // Final A — Corte original: Ivo sale, Nilo sostiene la puerta
    EndingNilo,       // Final B — Corte invertido: Nilo sale, Ivo sostiene la puerta
    EndingTogether    // Final C — Corte inacabado: el Ancla sostiene la puerta, salen ambos
};

UENUM(BlueprintType)
enum class ETUCEnding : uint8
{
    None,
    IvoSolo,
    NiloSolo,
    Together
};

/** Estado de una interacción para el personaje que la mira. Lo usa el HUD para explicar por qué no funciona. */
UENUM(BlueprintType)
enum class ETUCInteractionState : uint8
{
    Available,
    WrongCharacter,
    NotYet,
    Done,
    MissingAnchor
};

namespace TUC
{
    /** Número de fragmentos del Ancla repartidos en la campaña. */
    constexpr int32 FragmentCount = 3;

    /** Paso final de la campaña (finales ya elegidos). */
    constexpr int32 FinalStep = 10;
}
