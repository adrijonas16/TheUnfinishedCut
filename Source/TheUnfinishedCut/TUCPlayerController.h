#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TUCPlayerController.generated.h"

class ATUCCharacterBase;

/** Pantallas de menú dibujadas por ATUCHUD. */
enum class ETUCMenuScreen : uint8
{
    None,       // jugando
    Main,       // portada / menú principal
    Prologue,   // texto del prólogo antes de empezar
    HowTo,      // cómo jugar
    Pause       // pausa
};

enum class ETUCMenuAction : uint8
{
    Continue,
    NewGame,
    HowTo,
    Quit,
    Begin,
    Back,
    Resume,
    Restart,
    MainMenu
};

struct FTUCMenuItem
{
    FString Label;
    FString Hint;
    ETUCMenuAction Action = ETUCMenuAction::Back;
};

UCLASS()
class THEUNFINISHEDCUT_API ATUCPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    UFUNCTION(BlueprintCallable, Category="TUC|Characters")
    void SwitchCharacter();

    /** Vuelve al menú principal con Ivo poseído y el movimiento bloqueado. */
    void ShowIntro();

    /** true mientras se ve el menú principal, el prólogo o "cómo jugar" abierto desde la portada. */
    bool IsIntroVisible() const { return bIntroVisible; }
    /** true con el menú de pausa (o "cómo jugar" abierto desde la pausa). */
    bool IsPauseMenuVisible() const { return bPauseMenuVisible; }

    ETUCMenuScreen GetMenuScreen() const { return MenuScreen; }
    void GetMenuItems(TArray<FTUCMenuItem>& OutItems) const;
    int32 GetMenuSelection() const { return MenuSelection; }
    void SetMenuSelection(int32 Index);
    void ActivateMenuItem(int32 Index);

    /** Segundos (tiempo real) desde que empezó la partida actual; para el tutorial en pantalla. */
    float GetSecondsSinceGameplayStart() const;

private:
    void FindAndLinkCharacters();
    void OpenMenu(ETUCMenuScreen Screen);
    void EnterGameplay(bool bNewGame);
    void ApplyMenuInputMode(bool bMenuOpen);

    void MenuUp();
    void MenuDown();
    void MenuConfirm();
    void MenuBack();
    void MenuClick();
    void TogglePauseMenu();

    bool bIntroVisible = true;
    bool bPauseMenuVisible = false;
    ETUCMenuScreen MenuScreen = ETUCMenuScreen::Main;
    ETUCMenuScreen HowToReturn = ETUCMenuScreen::Main;
    int32 MenuSelection = 0;
    int32 LinkAttempts = 0;
    uint64 IntroClosedFrame = 0;
    uint64 MenuOpenedFrame = 0;
    double GameplayStartTime = -1000.0;

    FTimerHandle LinkTimer;
};
