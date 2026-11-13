#include "TUCPuzzleDirector.h"

#include "TUCCharacterBase.h"
#include "TUCGameMode.h"
#include "TUCSaveGame.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    const TCHAR* SaveSlotName = TEXT("TUC_Campaign");
    const TCHAR* LegacySaveSlotName = TEXT("TUC_Prototype");
}

ATUCPuzzleDirector::ATUCPuzzleDirector()
{
    PrimaryActorTick.bCanEverTick = false;
}

ATUCPuzzleDirector* ATUCPuzzleDirector::Get(const UObject* WorldContextObject)
{
    return Cast<ATUCPuzzleDirector>(UGameplayStatics::GetActorOfClass(WorldContextObject, ATUCPuzzleDirector::StaticClass()));
}

void ATUCPuzzleDirector::InitializeFromSave()
{
    if (bInitialized)
    {
        return;
    }
    bInitialized = true;

    USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0);
    if (!Loaded)
    {
        // Guardados de la v1.0 usaban otro slot.
        Loaded = UGameplayStatics::LoadGameFromSlot(LegacySaveSlotName, 0);
    }

    if (const UTUCSaveGame* Save = Cast<UTUCSaveGame>(Loaded))
    {
        ProgressStep = FMath::Clamp(Save->ProgressStep, 0, TUC::FinalStep);
        Ending = Save->Ending;
        bPrologueSeen = Save->bPrologueSeen || ProgressStep > 0;

        if (Save->SaveVersion >= 2)
        {
            FragmentMask = Save->FragmentMask & ((1 << TUC::FragmentCount) - 1);
            MemoryCount = FMath::CountBits(static_cast<uint64>(FragmentMask));
        }
        else
        {
            // v1.0 solo guardaba el número: se reconstruye la máscara asumiendo los primeros fragmentos.
            MemoryCount = FMath::Clamp(Save->MemoryCount, 0, TUC::FragmentCount);
            FragmentMask = (1 << MemoryCount) - 1;
        }
    }
}

void ATUCPuzzleDirector::ResetProgress()
{
    UGameplayStatics::DeleteGameInSlot(SaveSlotName, 0);
    UGameplayStatics::DeleteGameInSlot(LegacySaveSlotName, 0);
    ProgressStep = 0;
    MemoryCount = 0;
    FragmentMask = 0;
    Ending = ETUCEnding::None;
    bPrologueSeen = false;
    bInitialized = true;
    MessageStartTime = -100.0;
}

int32 ATUCPuzzleDirector::GetRequiredStep(ETUCInteractionKind Kind)
{
    switch (Kind)
    {
        case ETUCInteractionKind::PowerConsole:    return 0;
        case ETUCInteractionKind::VentLatch:       return 1;
        case ETUCInteractionKind::OriginProjector: return 2;
        case ETUCInteractionKind::RetroFilm:       return 3;
        case ETUCInteractionKind::RetroProjector:  return 4;
        case ETUCInteractionKind::FutureTerminal:  return 5;
        case ETUCInteractionKind::FutureDuct:      return 6;
        case ETUCInteractionKind::FutureProjector: return 7;
        case ETUCInteractionKind::ArchiveCore:     return 8;
        case ETUCInteractionKind::EndingIvo:
        case ETUCInteractionKind::EndingNilo:
        case ETUCInteractionKind::EndingTogether:  return 9;
        default:                                   return INDEX_NONE;
    }
}

bool ATUCPuzzleDirector::GetRequiredCharacter(ETUCInteractionKind Kind, ETUCCharacterType& OutType)
{
    switch (Kind)
    {
        case ETUCInteractionKind::PowerConsole:
        case ETUCInteractionKind::OriginProjector:
        case ETUCInteractionKind::RetroProjector:
        case ETUCInteractionKind::FutureTerminal:
        case ETUCInteractionKind::FutureProjector:
            OutType = ETUCCharacterType::Ivo;
            return true;
        case ETUCInteractionKind::VentLatch:
        case ETUCInteractionKind::RetroFilm:
        case ETUCInteractionKind::FutureDuct:
            OutType = ETUCCharacterType::Nilo;
            return true;
        // Final A: Nilo mantiene el control para que Ivo salga. Final B: al revés (02_GDD, Finales).
        case ETUCInteractionKind::EndingIvo:
            OutType = ETUCCharacterType::Nilo;
            return true;
        case ETUCInteractionKind::EndingNilo:
            OutType = ETUCCharacterType::Ivo;
            return true;
        default:
            return false;
    }
}

ETUCInteractionState ATUCPuzzleDirector::GetInteractionState(ETUCInteractionKind Kind, const ATUCCharacterBase* Character, int32 FragmentId) const
{
    if (Kind == ETUCInteractionKind::MemoryFragment)
    {
        if (IsFragmentCollected(FragmentId) || ProgressStep >= TUC::FinalStep)
        {
            return ETUCInteractionState::Done;
        }
        return ProgressStep >= 3 ? ETUCInteractionState::Available : ETUCInteractionState::NotYet;
    }

    const int32 RequiredStep = GetRequiredStep(Kind);
    if (RequiredStep == INDEX_NONE || ProgressStep > RequiredStep)
    {
        return ETUCInteractionState::Done;
    }
    if (ProgressStep < RequiredStep)
    {
        return ETUCInteractionState::NotYet;
    }

    ETUCCharacterType RequiredType = ETUCCharacterType::Ivo;
    if (Character && GetRequiredCharacter(Kind, RequiredType) && Character->GetCharacterType() != RequiredType)
    {
        return ETUCInteractionState::WrongCharacter;
    }

    // Final C: requiere el Ancla completa (contrato can_attempt_C). Nunca se concede solo por azar.
    if (Kind == ETUCInteractionKind::EndingTogether && MemoryCount < TUC::FragmentCount)
    {
        return ETUCInteractionState::MissingAnchor;
    }

    return ETUCInteractionState::Available;
}

bool ATUCPuzzleDirector::CanPerform(ETUCInteractionKind Kind, const ATUCCharacterBase* Character, int32 FragmentId) const
{
    return Character && GetInteractionState(Kind, Character, FragmentId) == ETUCInteractionState::Available;
}

bool ATUCPuzzleDirector::IsCurrentObjective(ETUCInteractionKind Kind) const
{
    if (Kind == ETUCInteractionKind::MemoryFragment)
    {
        return false;
    }
    if (Kind == ETUCInteractionKind::EndingTogether && MemoryCount < TUC::FragmentCount)
    {
        return false;
    }
    return GetRequiredStep(Kind) == ProgressStep;
}

bool ATUCPuzzleDirector::IsFragmentCollected(int32 FragmentId) const
{
    return FragmentId >= 0 && FragmentId < TUC::FragmentCount && (FragmentMask & (1 << FragmentId)) != 0;
}

void ATUCPuzzleDirector::MarkPrologueSeen()
{
    if (!bPrologueSeen)
    {
        bPrologueSeen = true;
        SaveProgress();
    }
}

void ATUCPuzzleDirector::AdvanceTo(int32 NewStep, const FString& Message, const FLinearColor& Color, bool bTravel)
{
    ProgressStep = FMath::Clamp(NewStep, 0, TUC::FinalStep);
    SaveProgress();
    PushMessage(Message, Color, 6.0f);

    if (bTravel)
    {
        if (ATUCGameMode* GameMode = Cast<ATUCGameMode>(UGameplayStatics::GetGameMode(this)))
        {
            GameMode->TravelToCampaignStage(ProgressStep);
        }
    }
}

void ATUCPuzzleDirector::Perform(ETUCInteractionKind Kind, ATUCCharacterBase* Character, int32 FragmentId)
{
    if (!CanPerform(Kind, Character, FragmentId))
    {
        return;
    }

    switch (Kind)
    {
        case ETUCInteractionKind::PowerConsole:
            AdvanceTo(1, TEXT("ENERGÍA RESTAURADA — cambia a Nilo (Tab) y abre el conducto verde."), FLinearColor(0.2f, 0.9f, 1.0f), false);
            break;
        case ETUCInteractionKind::VentLatch:
            AdvanceTo(2, TEXT("CONDUCTO ABIERTO — vuelve con Ivo y enciende el proyector."), FLinearColor(0.3f, 1.0f, 0.4f), false);
            break;
        case ETUCInteractionKind::OriginProjector:
            AdvanceTo(3, TEXT("«Esto no es una película normal...» — el film los arrastra a un universo de neón."), FLinearColor(1.0f, 0.3f, 1.0f), true);
            break;
        case ETUCInteractionKind::RetroFilm:
            AdvanceTo(4, TEXT("ROLLO RECUPERADO — Nilo lo encontró. Llévalo al proyector de la tarima con Ivo."), FLinearColor(0.3f, 1.0f, 0.4f), false);
            break;
        case ETUCInteractionKind::RetroProjector:
            AdvanceTo(5, TEXT("ARCHIVO DE LOS ECOS — ACCESO 02. «Vamos juntos, Nilo.»"), FLinearColor(0.2f, 0.9f, 1.0f), true);
            break;
        case ETUCInteractionKind::FutureTerminal:
            AdvanceTo(6, TEXT("TERMINAL EN LÍNEA — Ivo estabiliza la energía. Nilo puede cruzar el ducto."), FLinearColor(0.2f, 0.9f, 1.0f), false);
            break;
        case ETUCInteractionKind::FutureDuct:
            AdvanceTo(7, TEXT("SEGURO DESACTIVADO — el registro muestra otro final del mismo pasillo. No hay un único corte."), FLinearColor(0.3f, 1.0f, 0.4f), false);
            break;
        case ETUCInteractionKind::FutureProjector:
            AdvanceTo(8, TEXT("EL MONTADOR: «El corte registrado garantiza estabilidad.»"), FLinearColor(1.0f, 0.3f, 1.0f), true);
            break;
        case ETUCInteractionKind::MemoryFragment:
            if (FragmentId >= 0 && FragmentId < TUC::FragmentCount)
            {
                FragmentMask |= (1 << FragmentId);
                MemoryCount = FMath::CountBits(static_cast<uint64>(FragmentMask));
                SaveProgress();
                PushMessage(MemoryCount >= TUC::FragmentCount
                    ? FString(TEXT("FRAGMENTO DEL ANCLA 3/3 — el Ancla está completa. Existe otra salida."))
                    : FString::Printf(TEXT("FRAGMENTO DEL ANCLA %d/%d"), MemoryCount, TUC::FragmentCount),
                    FLinearColor(1.0f, 0.8f, 0.2f), 5.0f);
            }
            break;
        case ETUCInteractionKind::ArchiveCore:
            AdvanceTo(9, TEXT("LA SALA CONOCIDA — la misma puerta del prólogo. Elige cómo termina el corte."), FLinearColor(1.0f, 0.85f, 0.3f), false);
            break;
        case ETUCInteractionKind::EndingIvo:
            Ending = ETUCEnding::IvoSolo;
            AdvanceTo(TUC::FinalStep, TEXT("FINAL A — CORTE ORIGINAL"), FLinearColor(0.2f, 0.9f, 1.0f), false);
            break;
        case ETUCInteractionKind::EndingNilo:
            Ending = ETUCEnding::NiloSolo;
            AdvanceTo(TUC::FinalStep, TEXT("FINAL B — CORTE INVERTIDO"), FLinearColor(1.0f, 0.6f, 0.1f), false);
            break;
        case ETUCInteractionKind::EndingTogether:
            Ending = ETUCEnding::Together;
            AdvanceTo(TUC::FinalStep, TEXT("FINAL C — CORTE INACABADO"), FLinearColor(1.0f, 0.85f, 0.3f), false);
            break;
        default:
            break;
    }
}

FText ATUCPuzzleDirector::GetCurrentObjective() const
{
    switch (ProgressStep)
    {
        case 0: return FText::FromString(TEXT("Entra al cine y restaura la energía con Ivo."));
        case 1: return FText::FromString(TEXT("Cambia a Nilo (Tab) y abre el conducto verde."));
        case 2: return FText::FromString(TEXT("Vuelve con Ivo y activa el proyector violeta."));
        case 3: return FText::FromString(TEXT("Con Nilo, recupera el rollo verde."));
        case 4: return FText::FromString(TEXT("Con Ivo, proyecta el rollo en la tarima."));
        case 5: return FText::FromString(TEXT("Con Ivo, reactiva el terminal celeste."));
        case 6: return FText::FromString(TEXT("Con Nilo, cruza el ducto de seguridad."));
        case 7: return FText::FromString(TEXT("Con Ivo, enciende el proyector del fondo."));
        case 8: return FText::FromString(TEXT("Activa el núcleo blanco del Archivo."));
        case 9: return MemoryCount >= TUC::FragmentCount
            ? FText::FromString(TEXT("Elige el final: A (Nilo), B (Ivo) o C (Ancla)."))
            : FText::FromString(TEXT("Elige el final: A (con Nilo) o B (con Ivo)."));
        case 10: return GetEndingTitle();
        default: return FText::FromString(TEXT("Busca una salida."));
    }
}

FText ATUCPuzzleDirector::GetObjectiveDetail() const
{
    switch (ProgressStep)
    {
        case 0: return FText::FromString(TEXT("Camina hacia el cine (al frente). La consola de energía brilla en celeste en la pared IZQUIERDA del vestíbulo. Acércate y pulsa E."));
        case 1: return FText::FromString(TEXT("Ivo es muy grande para la rejilla. Pulsa TAB para jugar con Nilo: la rejilla verde está abajo en la pared DERECHA, más al fondo."));
        case 2: return FText::FromString(TEXT("Pulsa TAB para volver a Ivo. El proyector violeta está al fondo del vestíbulo, frente a la pantalla. Evita la luz roja del centinela."));
        case 3: return FText::FromString(TEXT("Estás en la película de los 80. Con Nilo (TAB), busca el soporte con el rollo verde a la DERECHA de la sala."));
        case 4: return FText::FromString(TEXT("Con Ivo (TAB), sube a la tarima del fondo y usa el proyector violeta."));
        case 5: return FText::FromString(TEXT("Ciudad del futuro. Con Ivo, el terminal celeste está en la pared IZQUIERDA nada más entrar."));
        case 6: return FText::FromString(TEXT("Con Nilo (TAB), la compuerta verde del ducto está abajo en la pared DERECHA, más adelante."));
        case 7: return FText::FromString(TEXT("Con Ivo (TAB), el proyector violeta está al fondo del pasillo. No dejes que el centinela te vea."));
        case 8: return FText::FromString(TEXT("Último capítulo. El núcleo blanco brilla en el centro de la sala. Cualquiera de los dos puede activarlo."));
        case 9: return MemoryCount >= TUC::FragmentCount
            ? FText::FromString(TEXT("Tres pedestales al fondo. Izquierda: Final A (lo activa Nilo). Derecha: Final B (lo activa Ivo). Centro: Final C, juntos."))
            : FText::FromString(TEXT("Dos pedestales al fondo. Izquierda: Final A (lo activa Nilo). Derecha: Final B (lo activa Ivo). Con 3 fragmentos amarillos se abre un tercer final."));
        default: return FText::GetEmpty();
    }
}

FText ATUCPuzzleDirector::GetChapterTitle() const
{
    if (ProgressStep < 3) { return FText::FromString(TEXT("CAPÍTULO I — EL CINE ETERNAL WAVE")); }
    if (ProgressStep < 5) { return FText::FromString(TEXT("CAPÍTULO II — LA NOCHE DE NEÓN")); }
    if (ProgressStep < 8) { return FText::FromString(TEXT("CAPÍTULO III — CIUDAD SIN MAÑANA")); }
    return FText::FromString(TEXT("CAPÍTULO IV — ARCHIVO DE LOS ECOS"));
}

FText ATUCPuzzleDirector::GetEndingTitle() const
{
    switch (Ending)
    {
        case ETUCEnding::IvoSolo:  return FText::FromString(TEXT("FINAL A — CORTE ORIGINAL"));
        case ETUCEnding::NiloSolo: return FText::FromString(TEXT("FINAL B — CORTE INVERTIDO"));
        case ETUCEnding::Together: return FText::FromString(TEXT("FINAL C — CORTE INACABADO"));
        default:                   return FText::GetEmpty();
    }
}

FText ATUCPuzzleDirector::GetEndingEpilogue() const
{
    switch (Ending)
    {
        case ETUCEnding::IvoSolo:
            return FText::FromString(TEXT("Ivo cruza la puerta con la campanilla en la mano. Nilo sostiene la salida y se queda atrás. El encuadre es idéntico al del prólogo."));
        case ETUCEnding::NiloSolo:
            return FText::FromString(TEXT("Nilo cruza primero. Ivo sostiene el control y la imagen del prólogo se invierte: esta vez es el gato quien mira atrás."));
        case ETUCEnding::Together:
            return FText::FromString(TEXT("El Ancla sostiene la puerta sin nadie. Ivo y Nilo salen juntos y la película queda sin terminar."));
        default:
            return FText::GetEmpty();
    }
}

void ATUCPuzzleDirector::PushMessage(const FString& Message, const FLinearColor& Color, float Duration)
{
    MessageText = Message;
    MessageColor = Color;
    MessageDuration = FMath::Max(0.5f, Duration);
    const UWorld* World = GetWorld();
    MessageStartTime = World ? World->GetRealTimeSeconds() : 0.0;
}

bool ATUCPuzzleDirector::GetActiveMessage(FText& OutMessage, FLinearColor& OutColor, float& OutAlpha) const
{
    const UWorld* World = GetWorld();
    if (!World || MessageText.IsEmpty())
    {
        return false;
    }

    const double Elapsed = World->GetRealTimeSeconds() - MessageStartTime;
    if (Elapsed < 0.0 || Elapsed > MessageDuration)
    {
        return false;
    }

    OutMessage = FText::FromString(MessageText);
    OutColor = MessageColor;
    const double Remaining = MessageDuration - Elapsed;
    OutAlpha = static_cast<float>(FMath::Clamp(Remaining / 0.6, 0.0, 1.0));
    return true;
}

void ATUCPuzzleDirector::SaveProgress() const
{
    if (UTUCSaveGame* Save = Cast<UTUCSaveGame>(UGameplayStatics::CreateSaveGameObject(UTUCSaveGame::StaticClass())))
    {
        Save->SaveVersion = 2;
        Save->ProgressStep = ProgressStep;
        Save->MemoryCount = MemoryCount;
        Save->FragmentMask = FragmentMask;
        Save->bPrologueSeen = bPrologueSeen;
        Save->Ending = Ending;
        UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, 0);
    }
}
