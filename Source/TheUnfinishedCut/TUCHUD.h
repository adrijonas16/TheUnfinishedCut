#pragma once

#include "CoreMinimal.h"
#include "Fonts/SlateFontInfo.h"
#include "GameFramework/HUD.h"
#include "TUCHUD.generated.h"

class ATUCInteractionActor;
class ATUCPlayerController;
class ATUCPuzzleDirector;

/**
 * HUD dibujado con Canvas y fuentes Slate (nítidas a cualquier tamaño):
 * menú principal, prólogo, cómo jugar, pausa, objetivo guiado, marcador y final.
 */
UCLASS()
class THEUNFINISHEDCUT_API ATUCHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

    /** Índice del botón de menú bajo el cursor (coordenadas de viewport), o INDEX_NONE. */
    int32 GetMenuItemAt(const FVector2D& Position) const;

private:
    float UIScale = 1.0f;
    FString ShownChapter;
    double ChapterCardStart = -100.0;
    TArray<FBox2D> MenuItemRects;
    FVector2D LastMouse = FVector2D(-1.0, -1.0);

    // ── texto ──
    FSlateFontInfo MakeFont(float Size, bool bBold) const;
    FVector2D Measure(const FString& Text, const FSlateFontInfo& Font) const;
    /** Dibuja texto; HAlign 0 = izquierda, 0.5 = centro, 1 = derecha. Devuelve la altura. */
    float DrawTextAt(const FString& Text, float X, float Y, const FLinearColor& Color, float Size, bool bBold = false, float HAlign = 0.0f);
    /** Texto con salto de línea automático. Devuelve la altura total. */
    float DrawWrapped(const FString& Text, float X, float Y, float MaxWidth, const FLinearColor& Color, float Size, bool bBold = false, float HAlign = 0.0f, float LineGap = 4.0f);
    TArray<FString> WrapLines(const FString& Text, const FSlateFontInfo& Font, float MaxWidth) const;
    void DrawPanel(float X, float Y, float W, float H, const FLinearColor& Fill, const FLinearColor& Border, float BorderSize = 2.0f);

    // ── pantallas ──
    void DrawMenuScreen(ATUCPlayerController* Controller, const ATUCPuzzleDirector* Director);
    void DrawMenuButtons(ATUCPlayerController* Controller, float CenterX, float TopY, float Width);
    float DrawHowToContent(float X, float Y, float Width);
    void DrawEnding(const ATUCPuzzleDirector* Director);
    void DrawGameplay(ATUCPlayerController* Controller, const ATUCPuzzleDirector* Director);
    void DrawObjectiveMarker(const ATUCInteractionActor* Target, const ATUCPuzzleDirector* Director);
};
