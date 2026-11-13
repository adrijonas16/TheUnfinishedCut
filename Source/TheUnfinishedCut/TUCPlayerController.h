#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TUCPlayerController.generated.h"

class ATUCCharacterBase;

UCLASS()
class THEUNFINISHEDCUT_API ATUCPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    UFUNCTION(BlueprintCallable, Category="TUC|Characters")
    void SwitchCharacter();

    /** Vuelve a la portada (prólogo o "continuar") con Ivo poseído y el movimiento bloqueado. */
    void ShowIntro();

    bool IsIntroVisible() const { return bIntroVisible; }
    bool IsPauseMenuVisible() const { return bPauseMenuVisible; }

private:
    void FindAndLinkCharacters();
    void RestartCampaign();
    void StartCampaign();
    void TogglePauseMenu();

    bool bIntroVisible = true;
    bool bPauseMenuVisible = false;
    int32 LinkAttempts = 0;
    uint64 IntroClosedFrame = 0;

    FTimerHandle LinkTimer;
};
