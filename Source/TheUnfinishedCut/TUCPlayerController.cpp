#include "TUCPlayerController.h"

#include "TUCCharacterBase.h"
#include "TUCGameMode.h"
#include "TUCHUD.h"
#include "TUCIvoCharacter.h"
#include "TUCNiloCharacter.h"
#include "TUCPuzzleDirector.h"
#include "Components/InputComponent.h"
#include "CoreGlobals.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

void ATUCPlayerController::BeginPlay()
{
    Super::BeginPlay();
    // Reintenta hasta encontrar a Ivo y Nilo (los genera el GameMode durante BeginPlay).
    GetWorldTimerManager().SetTimer(LinkTimer, this, &ATUCPlayerController::FindAndLinkCharacters, 0.1f, true, 0.1f);
}

void ATUCPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (!InputComponent)
    {
        return;
    }

    // Las acciones de menú no consumen la tecla: así W/S, A del mando, etc. siguen llegando al personaje.
    auto Bind = [this](const TCHAR* Action, void (ATUCPlayerController::*Handler)())
    {
        FInputActionBinding& Binding = InputComponent->BindAction(Action, IE_Pressed, this, Handler);
        Binding.bExecuteWhenPaused = true;
        Binding.bConsumeInput = false;
    };
    Bind(TEXT("StartCampaign"), &ATUCPlayerController::MenuConfirm);
    Bind(TEXT("RestartPrototype"), &ATUCPlayerController::MenuBack);
    Bind(TEXT("PauseCampaign"), &ATUCPlayerController::TogglePauseMenu);
    Bind(TEXT("MenuUp"), &ATUCPlayerController::MenuUp);
    Bind(TEXT("MenuDown"), &ATUCPlayerController::MenuDown);
    Bind(TEXT("MenuClick"), &ATUCPlayerController::MenuClick);
}

void ATUCPlayerController::FindAndLinkCharacters()
{
    ++LinkAttempts;

    ATUCIvoCharacter* Ivo = nullptr;
    ATUCNiloCharacter* Nilo = nullptr;
    for (TActorIterator<ATUCIvoCharacter> It(GetWorld()); It; ++It) { Ivo = *It; break; }
    for (TActorIterator<ATUCNiloCharacter> It(GetWorld()); It; ++It) { Nilo = *It; break; }

    if (!Ivo || !Nilo)
    {
        if (LinkAttempts > 50)
        {
            GetWorldTimerManager().ClearTimer(LinkTimer);
            UE_LOG(LogTemp, Error, TEXT("TUC: no se encontraron Ivo y Nilo. ¿El mapa usa TUCGameMode como GameMode?"));
        }
        return;
    }

    GetWorldTimerManager().ClearTimer(LinkTimer);
    Ivo->SetPartner(Nilo);
    Nilo->SetPartner(Ivo);
    ShowIntro();
}

void ATUCPlayerController::ShowIntro()
{
    ATUCIvoCharacter* Ivo = nullptr;
    for (TActorIterator<ATUCIvoCharacter> It(GetWorld()); It; ++It) { Ivo = *It; break; }
    if (Ivo && GetPawn() != Ivo)
    {
        Possess(Ivo);
    }
    if (Ivo)
    {
        SetControlRotation(FRotator(0.0f, Ivo->GetActorRotation().Yaw, 0.0f));
    }

    bPauseMenuVisible = false;
    SetPause(false);
    bIntroVisible = true;
    OpenMenu(ETUCMenuScreen::Main);
}

void ATUCPlayerController::ApplyMenuInputMode(bool bMenuOpen)
{
    if (bMenuOpen)
    {
        SetShowMouseCursor(true);
        FInputModeGameAndUI Mode;
        Mode.SetHideCursorDuringCapture(false);
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(Mode);
        ResetIgnoreInputFlags();
        SetIgnoreMoveInput(true);
        SetIgnoreLookInput(true);
    }
    else
    {
        SetShowMouseCursor(false);
        SetInputMode(FInputModeGameOnly());
        ResetIgnoreInputFlags();
    }
}

void ATUCPlayerController::OpenMenu(ETUCMenuScreen Screen)
{
    MenuScreen = Screen;
    MenuSelection = 0;
    MenuOpenedFrame = GFrameCounter;
    ApplyMenuInputMode(Screen != ETUCMenuScreen::None);
}

void ATUCPlayerController::GetMenuItems(TArray<FTUCMenuItem>& OutItems) const
{
    OutItems.Reset();
    const ATUCPuzzleDirector* Director = ATUCPuzzleDirector::Get(this);
    auto Add = [&OutItems](const FString& Label, const FString& Hint, ETUCMenuAction Action)
    {
        FTUCMenuItem Item;
        Item.Label = Label;
        Item.Hint = Hint;
        Item.Action = Action;
        OutItems.Add(Item);
    };

    switch (MenuScreen)
    {
        case ETUCMenuScreen::Main:
            if (Director && Director->GetProgressStep() > 0 && Director->GetProgressStep() < TUC::FinalStep)
            {
                Add(TEXT("CONTINUAR"), Director->GetChapterTitle().ToString(), ETUCMenuAction::Continue);
            }
            Add(TEXT("NUEVA PARTIDA"), TEXT("Empieza la historia desde el principio"), ETUCMenuAction::NewGame);
            Add(TEXT("CÓMO JUGAR"), TEXT("Controles y qué hay que hacer"), ETUCMenuAction::HowTo);
            Add(TEXT("SALIR"), TEXT("Cerrar el juego"), ETUCMenuAction::Quit);
            break;
        case ETUCMenuScreen::Prologue:
            Add(TEXT("EMPEZAR"), TEXT(""), ETUCMenuAction::Begin);
            Add(TEXT("CÓMO JUGAR"), TEXT(""), ETUCMenuAction::HowTo);
            break;
        case ETUCMenuScreen::HowTo:
            Add(TEXT("VOLVER"), TEXT(""), ETUCMenuAction::Back);
            break;
        case ETUCMenuScreen::Pause:
            Add(TEXT("SEGUIR JUGANDO"), TEXT(""), ETUCMenuAction::Resume);
            Add(TEXT("CÓMO JUGAR"), TEXT(""), ETUCMenuAction::HowTo);
            Add(TEXT("EMPEZAR DE NUEVO"), TEXT("Borra el progreso guardado"), ETUCMenuAction::Restart);
            Add(TEXT("MENÚ PRINCIPAL"), TEXT(""), ETUCMenuAction::MainMenu);
            Add(TEXT("SALIR DEL JUEGO"), TEXT(""), ETUCMenuAction::Quit);
            break;
        default:
            break;
    }
}

void ATUCPlayerController::SetMenuSelection(int32 Index)
{
    TArray<FTUCMenuItem> Items;
    GetMenuItems(Items);
    if (Items.Num() > 0)
    {
        MenuSelection = FMath::Clamp(Index, 0, Items.Num() - 1);
    }
}

void ATUCPlayerController::ActivateMenuItem(int32 Index)
{
    TArray<FTUCMenuItem> Items;
    GetMenuItems(Items);
    if (!Items.IsValidIndex(Index))
    {
        return;
    }

    ATUCGameMode* GameMode = Cast<ATUCGameMode>(UGameplayStatics::GetGameMode(this));
    switch (Items[Index].Action)
    {
        case ETUCMenuAction::Continue:
            EnterGameplay(false);
            break;
        case ETUCMenuAction::NewGame:
            if (GameMode)
            {
                GameMode->RestartCampaign();
            }
            OpenMenu(ETUCMenuScreen::Prologue);
            break;
        case ETUCMenuAction::Begin:
            EnterGameplay(true);
            break;
        case ETUCMenuAction::HowTo:
            HowToReturn = MenuScreen;
            OpenMenu(ETUCMenuScreen::HowTo);
            break;
        case ETUCMenuAction::Back:
            OpenMenu(HowToReturn);
            break;
        case ETUCMenuAction::Resume:
            bPauseMenuVisible = false;
            SetPause(false);
            OpenMenu(ETUCMenuScreen::None);
            break;
        case ETUCMenuAction::Restart:
            SetPause(false);
            bPauseMenuVisible = false;
            if (GameMode)
            {
                GameMode->RestartCampaign();
            }
            ShowIntro();
            OpenMenu(ETUCMenuScreen::Prologue);
            break;
        case ETUCMenuAction::MainMenu:
            ShowIntro();
            break;
        case ETUCMenuAction::Quit:
            UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
            break;
        default:
            break;
    }
}

void ATUCPlayerController::EnterGameplay(bool bNewGame)
{
    bIntroVisible = false;
    bPauseMenuVisible = false;
    SetPause(false);
    IntroClosedFrame = GFrameCounter;
    OpenMenu(ETUCMenuScreen::None);

    const UWorld* World = GetWorld();
    GameplayStartTime = (bNewGame && World) ? World->GetRealTimeSeconds() : -1000.0;

    if (ATUCPuzzleDirector* Director = ATUCPuzzleDirector::Get(this))
    {
        Director->MarkPrologueSeen();
        Director->PushMessage(Director->GetChapterTitle().ToString(), FLinearColor(1.0f, 0.85f, 0.3f), 4.0f);
    }
}

float ATUCPlayerController::GetSecondsSinceGameplayStart() const
{
    const UWorld* World = GetWorld();
    return World ? static_cast<float>(World->GetRealTimeSeconds() - GameplayStartTime) : 1000.0f;
}

void ATUCPlayerController::SwitchCharacter()
{
    if (bIntroVisible || bPauseMenuVisible)
    {
        return;
    }

    ATUCCharacterBase* Current = Cast<ATUCCharacterBase>(GetPawn());
    if (!Current || !Current->GetPartner())
    {
        return;
    }

    ATUCCharacterBase* Target = Current->GetPartner();
    const FRotator TargetView(0.0f, Target->GetActorRotation().Yaw, 0.0f);
    Possess(Target);
    SetControlRotation(TargetView);
    ResetIgnoreInputFlags();
}

void ATUCPlayerController::MenuUp()
{
    if (MenuScreen != ETUCMenuScreen::None)
    {
        TArray<FTUCMenuItem> Items;
        GetMenuItems(Items);
        if (Items.Num() > 0)
        {
            MenuSelection = (MenuSelection + Items.Num() - 1) % Items.Num();
        }
    }
}

void ATUCPlayerController::MenuDown()
{
    if (MenuScreen != ETUCMenuScreen::None)
    {
        TArray<FTUCMenuItem> Items;
        GetMenuItems(Items);
        if (Items.Num() > 0)
        {
            MenuSelection = (MenuSelection + 1) % Items.Num();
        }
    }
}

void ATUCPlayerController::MenuConfirm()
{
    // Ignorar la misma pulsación que abrió el menú (Options del mando abre la pausa y también confirma).
    if (GFrameCounter == MenuOpenedFrame || GFrameCounter == IntroClosedFrame)
    {
        return;
    }

    if (MenuScreen != ETUCMenuScreen::None)
    {
        ActivateMenuItem(MenuSelection);
        return;
    }

    // Pantalla de final: ENTER vuelve al menú principal.
    const ATUCPuzzleDirector* Director = ATUCPuzzleDirector::Get(this);
    if (Director && Director->GetProgressStep() >= TUC::FinalStep)
    {
        ShowIntro();
    }
}

void ATUCPlayerController::MenuBack()
{
    switch (MenuScreen)
    {
        case ETUCMenuScreen::HowTo:
            OpenMenu(HowToReturn);
            break;
        case ETUCMenuScreen::Pause:
            bPauseMenuVisible = false;
            SetPause(false);
            OpenMenu(ETUCMenuScreen::None);
            break;
        case ETUCMenuScreen::Prologue:
            OpenMenu(ETUCMenuScreen::Main);
            break;
        default:
            break;
    }
}

void ATUCPlayerController::MenuClick()
{
    if (MenuScreen == ETUCMenuScreen::None || GFrameCounter == MenuOpenedFrame)
    {
        return;
    }
    const ATUCHUD* HUD = Cast<ATUCHUD>(GetHUD());
    float MouseX = 0.0f;
    float MouseY = 0.0f;
    if (HUD && GetMousePosition(MouseX, MouseY))
    {
        const int32 Index = HUD->GetMenuItemAt(FVector2D(MouseX, MouseY));
        if (Index != INDEX_NONE)
        {
            MenuSelection = Index;
            ActivateMenuItem(Index);
        }
    }
}

void ATUCPlayerController::TogglePauseMenu()
{
    // En mando, Options abre la partida y también pausa: ignorar la pulsación que cerró la portada.
    if (bIntroVisible || GFrameCounter == IntroClosedFrame || GFrameCounter == MenuOpenedFrame)
    {
        return;
    }

    if (MenuScreen == ETUCMenuScreen::None)
    {
        const ATUCPuzzleDirector* Director = ATUCPuzzleDirector::Get(this);
        if (Director && Director->GetProgressStep() >= TUC::FinalStep)
        {
            return;
        }
        bPauseMenuVisible = true;
        SetPause(true);
        OpenMenu(ETUCMenuScreen::Pause);
    }
    else if (MenuScreen == ETUCMenuScreen::HowTo)
    {
        OpenMenu(HowToReturn);
    }
    else if (MenuScreen == ETUCMenuScreen::Pause)
    {
        bPauseMenuVisible = false;
        SetPause(false);
        OpenMenu(ETUCMenuScreen::None);
    }
}
