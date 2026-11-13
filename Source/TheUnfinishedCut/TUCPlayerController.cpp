#include "TUCPlayerController.h"

#include "TUCCharacterBase.h"
#include "TUCGameMode.h"
#include "TUCIvoCharacter.h"
#include "TUCNiloCharacter.h"
#include "TUCPuzzleDirector.h"
#include "Components/InputComponent.h"
#include "CoreGlobals.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
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
    if (InputComponent)
    {
        FInputActionBinding& RestartBinding = InputComponent->BindAction("RestartPrototype", IE_Pressed, this, &ATUCPlayerController::RestartCampaign);
        RestartBinding.bExecuteWhenPaused = true;
        FInputActionBinding& StartBinding = InputComponent->BindAction("StartCampaign", IE_Pressed, this, &ATUCPlayerController::StartCampaign);
        StartBinding.bExecuteWhenPaused = true;
        FInputActionBinding& PauseBinding = InputComponent->BindAction("PauseCampaign", IE_Pressed, this, &ATUCPlayerController::TogglePauseMenu);
        PauseBinding.bExecuteWhenPaused = true;
    }
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

    if (bPauseMenuVisible)
    {
        bPauseMenuVisible = false;
        SetPause(false);
    }

    bIntroVisible = true;
    SetShowMouseCursor(false);
    SetInputMode(FInputModeGameOnly());
    ResetIgnoreInputFlags();
    SetIgnoreMoveInput(true);
    SetIgnoreLookInput(true);
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

void ATUCPlayerController::RestartCampaign()
{
    if (ATUCGameMode* GameMode = Cast<ATUCGameMode>(UGameplayStatics::GetGameMode(this)))
    {
        // Reinicio en el mismo mundo: no depende del nombre del mapa (PIE, Entry o mapa propio).
        GameMode->RestartCampaign();
    }
    ShowIntro();
}

void ATUCPlayerController::StartCampaign()
{
    if (!bIntroVisible)
    {
        return;
    }

    bIntroVisible = false;
    IntroClosedFrame = GFrameCounter;
    ResetIgnoreInputFlags();

    if (ATUCPuzzleDirector* Director = ATUCPuzzleDirector::Get(this))
    {
        Director->MarkPrologueSeen();
        Director->PushMessage(Director->GetChapterTitle().ToString(), FLinearColor(1.0f, 0.85f, 0.3f), 4.0f);
    }
}

void ATUCPlayerController::TogglePauseMenu()
{
    // En mando, Options abre la partida y también pausa: ignorar la pulsación que cerró la portada.
    if (bIntroVisible || GFrameCounter == IntroClosedFrame)
    {
        return;
    }

    bPauseMenuVisible = !bPauseMenuVisible;
    SetPause(bPauseMenuVisible);
}
