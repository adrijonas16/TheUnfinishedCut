#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TUCHUD.generated.h"

class UFont;

/** HUD dibujado con Canvas: portada/prólogo, objetivo, prompts, detección, pausa y epílogo. */
UCLASS()
class THEUNFINISHEDCUT_API ATUCHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

private:
    float UIScale = 1.0f;
    FString ShownChapter;
    double ChapterCardStart = -100.0;

    /** HAlign: 0 = izquierda, 0.5 = centrado, 1 = derecha (respecto a X). Devuelve la altura dibujada. */
    float DrawLabel(const FString& Text, float X, float Y, const FLinearColor& Color, UFont* Font, float Scale, float HAlign = 0.0f);
    TArray<FString> WrapLines(const FString& Text, UFont* Font, float Scale, float MaxWidth);
    void MeasureText(const FString& Text, UFont* Font, float Scale, float& OutWidth, float& OutHeight);

    void DrawIntro(const class ATUCPuzzleDirector* Director);
    void DrawEnding(const class ATUCPuzzleDirector* Director);
    void DrawGameplay(const class ATUCPuzzleDirector* Director);
};
