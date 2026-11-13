#include "TUCHUD.h"

#include "TUCCharacterBase.h"
#include "TUCInteractionActor.h"
#include "TUCPlayerController.h"
#include "TUCPuzzleDirector.h"
#include "TUCWatcher.h"
#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"

namespace
{
    const FLinearColor HudYellow(1.0f, 0.82f, 0.25f);
    const FLinearColor HudCyan(0.30f, 0.85f, 1.0f);
    const FLinearColor HudGreen(0.35f, 1.0f, 0.45f);
    const FLinearColor HudOrange(1.0f, 0.62f, 0.15f);
    const FLinearColor HudRed(1.0f, 0.28f, 0.22f);
    const FLinearColor HudMuted(0.72f, 0.76f, 0.84f);
    const FLinearColor HudPanel(0.02f, 0.025f, 0.04f, 0.78f);
    const FLinearColor HudPanelBorder(1.0f, 0.82f, 0.25f, 0.35f);

    FLinearColor ColorFor(ETUCCharacterType Type)
    {
        return Type == ETUCCharacterType::Ivo ? HudCyan : HudGreen;
    }

    const TCHAR* NameFor(ETUCCharacterType Type)
    {
        return Type == ETUCCharacterType::Ivo ? TEXT("IVO") : TEXT("NILO");
    }
}

// ───────────────────────── texto ─────────────────────────

FSlateFontInfo ATUCHUD::MakeFont(float Size, bool bBold) const
{
    const int32 Points = FMath::Max(8, FMath::RoundToInt(Size * UIScale));
    return FCoreStyle::GetDefaultFontStyle(bBold ? TEXT("Bold") : TEXT("Regular"), Points);
}

FVector2D ATUCHUD::Measure(const FString& Text, const FSlateFontInfo& Font) const
{
    if (FSlateApplication::IsInitialized() && FSlateApplication::Get().GetRenderer())
    {
        const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
        return FVector2D(FontMeasure->Measure(Text, Font));
    }
    const float Pixels = Font.Size * 96.0f / 72.0f;
    return FVector2D(Text.Len() * Pixels * 0.55f, Pixels * 1.25f);
}

float ATUCHUD::DrawTextAt(const FString& Text, float X, float Y, const FLinearColor& Color, float Size, bool bBold, float HAlign)
{
    if (!Canvas || Text.IsEmpty())
    {
        return 0.0f;
    }
    const FSlateFontInfo Font = MakeFont(Size, bBold);
    const FVector2D Dimensions = Measure(Text, Font);
    FCanvasTextItem Item(FVector2D(X - Dimensions.X * HAlign, Y), FText::FromString(Text), Font, Color);
    Item.EnableShadow(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f), FVector2D(2.0f, 2.0f));
    Canvas->DrawItem(Item);
    return static_cast<float>(Dimensions.Y);
}

TArray<FString> ATUCHUD::WrapLines(const FString& Text, const FSlateFontInfo& Font, float MaxWidth) const
{
    TArray<FString> Words;
    Text.ParseIntoArray(Words, TEXT(" "), true);

    TArray<FString> Lines;
    FString Current;
    for (const FString& Word : Words)
    {
        const FString Candidate = Current.IsEmpty() ? Word : Current + TEXT(" ") + Word;
        if (Measure(Candidate, Font).X > MaxWidth && !Current.IsEmpty())
        {
            Lines.Add(Current);
            Current = Word;
        }
        else
        {
            Current = Candidate;
        }
    }
    if (!Current.IsEmpty())
    {
        Lines.Add(Current);
    }
    return Lines;
}

float ATUCHUD::DrawWrapped(const FString& Text, float X, float Y, float MaxWidth, const FLinearColor& Color, float Size, bool bBold, float HAlign, float LineGap)
{
    const FSlateFontInfo Font = MakeFont(Size, bBold);
    float Total = 0.0f;
    for (const FString& Line : WrapLines(Text, Font, MaxWidth))
    {
        Total += DrawTextAt(Line, X, Y + Total, Color, Size, bBold, HAlign) + LineGap * UIScale;
    }
    return Total;
}

void ATUCHUD::DrawPanel(float X, float Y, float W, float H, const FLinearColor& Fill, const FLinearColor& Border, float BorderSize)
{
    DrawRect(Fill, X, Y, W, H);
    if (Border.A > 0.0f)
    {
        const float B = BorderSize * UIScale;
        DrawRect(Border, X, Y, W, B);
        DrawRect(Border, X, Y + H - B, W, B);
        DrawRect(Border, X, Y, B, H);
        DrawRect(Border, X + W - B, Y, B, H);
    }
}

int32 ATUCHUD::GetMenuItemAt(const FVector2D& Position) const
{
    for (int32 Index = 0; Index < MenuItemRects.Num(); ++Index)
    {
        if (MenuItemRects[Index].IsInside(Position))
        {
            return Index;
        }
    }
    return INDEX_NONE;
}

// ───────────────────────── entrada ─────────────────────────

void ATUCHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas)
    {
        return;
    }

    UIScale = FMath::Clamp(Canvas->ClipY / 1080.0f, 0.7f, 2.0f);
    MenuItemRects.Reset();

    ATUCPlayerController* Controller = Cast<ATUCPlayerController>(GetOwningPlayerController());
    const ATUCPuzzleDirector* Director = ATUCPuzzleDirector::Get(this);

    if (Controller && Controller->IsIntroVisible())
    {
        if (Director)
        {
            ShownChapter = Director->GetChapterTitle().ToString();
        }
        DrawMenuScreen(Controller, Director);
        return;
    }

    if (Director && Director->GetProgressStep() >= TUC::FinalStep)
    {
        DrawEnding(Director);
        return;
    }

    DrawGameplay(Controller, Director);

    if (Controller && Controller->GetMenuScreen() != ETUCMenuScreen::None)
    {
        DrawMenuScreen(Controller, Director);
    }
}

// ───────────────────────── menús ─────────────────────────

void ATUCHUD::DrawMenuButtons(ATUCPlayerController* Controller, float CenterX, float TopY, float Width)
{
    TArray<FTUCMenuItem> Items;
    Controller->GetMenuItems(Items);

    // Ratón: si se movió, el botón bajo el cursor pasa a estar seleccionado.
    float MouseX = 0.0f;
    float MouseY = 0.0f;
    const bool bHasMouse = Controller->GetMousePosition(MouseX, MouseY);
    const FVector2D Mouse(MouseX, MouseY);
    const bool bMouseMoved = bHasMouse && FVector2D::DistSquared(Mouse, LastMouse) > 4.0;
    LastMouse = Mouse;

    const float Height = 66.0f * UIScale;
    const float Gap = 14.0f * UIScale;
    float Y = TopY;
    for (int32 Index = 0; Index < Items.Num(); ++Index)
    {
        const FBox2D Rect(FVector2D(CenterX - Width * 0.5f, Y), FVector2D(CenterX + Width * 0.5f, Y + Height));
        MenuItemRects.Add(Rect);
        if (bMouseMoved && Rect.IsInside(Mouse))
        {
            Controller->SetMenuSelection(Index);
        }
        const bool bSelected = Controller->GetMenuSelection() == Index;

        DrawPanel(Rect.Min.X, Rect.Min.Y, Width, Height,
            bSelected ? FLinearColor(1.0f, 0.78f, 0.2f, 0.95f) : FLinearColor(0.05f, 0.06f, 0.09f, 0.88f),
            bSelected ? FLinearColor(1.0f, 0.95f, 0.7f, 1.0f) : FLinearColor(1.0f, 1.0f, 1.0f, 0.18f));

        const FLinearColor LabelColor = bSelected ? FLinearColor(0.03f, 0.03f, 0.05f) : FLinearColor::White;
        if (Items[Index].Hint.IsEmpty())
        {
            DrawTextAt(Items[Index].Label, CenterX, Y + Height * 0.5f - 17.0f * UIScale, LabelColor, 22.0f, true, 0.5f);
        }
        else
        {
            DrawTextAt(Items[Index].Label, CenterX, Y + 7.0f * UIScale, LabelColor, 21.0f, true, 0.5f);
            DrawTextAt(Items[Index].Hint, CenterX, Y + 38.0f * UIScale,
                bSelected ? FLinearColor(0.15f, 0.12f, 0.05f) : HudMuted, 13.0f, false, 0.5f);
        }
        Y += Height + Gap;
    }
}

float ATUCHUD::DrawHowToContent(float X, float Y, float Width)
{
    const float Start = Y;
    const float Gap = 14.0f * UIScale;
    const float Col = Width;

    Y += DrawTextAt(TEXT("CÓMO JUGAR"), X + Col * 0.5f, Y, HudYellow, 34.0f, true, 0.5f) + Gap * 1.5f;

    Y += DrawTextAt(TEXT("TU META"), X, Y, HudOrange, 18.0f, true) + 4.0f * UIScale;
    Y += DrawWrapped(TEXT("Ivo y su gato Nilo quedaron atrapados dentro de las películas de un cine abandonado. Atraviesa 4 películas resolviendo puzles entre los dos y elige cómo termina la historia."),
        X, Y, Col, FLinearColor::White, 17.0f) + Gap;

    Y += DrawTextAt(TEXT("DOS PERSONAJES — PULSA TAB PARA CAMBIAR"), X, Y, HudOrange, 18.0f, true) + 4.0f * UIScale;
    Y += DrawWrapped(TEXT("IVO (cámara en primera persona): usa consolas, terminales y proyectores. Sus objetos brillan en CELESTE o VIOLETA."),
        X, Y, Col, HudCyan, 17.0f) + 4.0f * UIScale;
    Y += DrawWrapped(TEXT("NILO (cámara detrás del gato): cabe por rejillas y ductos y recoge rollos de película. Sus objetos brillan en VERDE."),
        X, Y, Col, HudGreen, 17.0f) + 4.0f * UIScale;
    Y += DrawWrapped(TEXT("Con F el otro personaje te sigue o se queda esperando."),
        X, Y, Col, HudMuted, 17.0f) + Gap;

    Y += DrawTextAt(TEXT("CÓMO SABER QUÉ HACER"), X, Y, HudOrange, 18.0f, true) + 4.0f * UIScale;
    Y += DrawWrapped(TEXT("1. Arriba a la izquierda siempre está el OBJETIVO, dónde está y QUIÉN debe hacerlo."),
        X, Y, Col, FLinearColor::White, 17.0f) + 2.0f * UIScale;
    Y += DrawWrapped(TEXT("2. Un rombo de color flota sobre el objetivo con la distancia en metros. Si queda fuera de la pantalla, una flecha en el borde te indica hacia dónde girar."),
        X, Y, Col, FLinearColor::White, 17.0f) + 2.0f * UIScale;
    Y += DrawWrapped(TEXT("3. Al acercarte aparece el aviso [E]. Púlsalo para usar el objeto."),
        X, Y, Col, FLinearColor::White, 17.0f) + Gap;

    Y += DrawTextAt(TEXT("CUIDADO"), X, Y, HudRed, 18.0f, true) + 4.0f * UIScale;
    Y += DrawWrapped(TEXT("Los centinelas proyectan un cono de luz roja. Si te ven demasiado tiempo, la cinta se rebobina al último punto de control. Los fragmentos amarillos del Ancla son opcionales y abren un final secreto."),
        X, Y, Col, FLinearColor::White, 17.0f) + Gap;

    Y += DrawTextAt(TEXT("CONTROLES"), X, Y, HudOrange, 18.0f, true) + 4.0f * UIScale;
    const TCHAR* Controls[][2] =
    {
        { TEXT("WASD / stick izquierdo"), TEXT("moverse") },
        { TEXT("Ratón / stick derecho"), TEXT("mirar") },
        { TEXT("E / R2 / A"), TEXT("usar objeto") },
        { TEXT("TAB / L1"), TEXT("cambiar entre Ivo y Nilo") },
        { TEXT("F / Q / Triángulo"), TEXT("que el otro te siga o espere") },
        { TEXT("P / Options"), TEXT("pausa y menú") },
    };
    for (const auto& Row : Controls)
    {
        DrawTextAt(Row[0], X, Y, HudYellow, 16.0f, true);
        Y += DrawTextAt(Row[1], X + Col * 0.42f, Y, FLinearColor::White, 16.0f) + 3.0f * UIScale;
    }
    return Y - Start;
}

void ATUCHUD::DrawMenuScreen(ATUCPlayerController* Controller, const ATUCPuzzleDirector* Director)
{
    const float W = Canvas->ClipX;
    const float H = Canvas->ClipY;
    const float CenterX = W * 0.5f;
    const ETUCMenuScreen Screen = Controller->GetMenuScreen();

    DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, Screen == ETUCMenuScreen::Pause ? 0.62f : 0.80f), 0.0f, 0.0f, W, H);

    if (Screen == ETUCMenuScreen::HowTo)
    {
        const float PanelW = FMath::Min(1100.0f * UIScale, W * 0.86f);
        const float PanelX = CenterX - PanelW * 0.5f;
        const float PanelY = H * 0.05f;
        const float PanelH = H * 0.83f;
        DrawPanel(PanelX, PanelY, PanelW, PanelH, HudPanel, HudPanelBorder);
        DrawHowToContent(PanelX + 40.0f * UIScale, PanelY + 28.0f * UIScale, PanelW - 80.0f * UIScale);
        DrawMenuButtons(Controller, CenterX, PanelY + PanelH + 18.0f * UIScale, 360.0f * UIScale);
        return;
    }

    if (Screen == ETUCMenuScreen::Prologue)
    {
        float Y = H * 0.20f;
        Y += DrawTextAt(TEXT("CORTE 001 — FINAL REGISTRADO"), CenterX, Y, HudOrange, 22.0f, true, 0.5f) + 30.0f * UIScale;
        const TCHAR* Lines[] =
        {
            TEXT("Un proyector suena en una sala que nadie recuerda."),
            TEXT("Ivo cruza una puerta. Mira atrás. Espera."),
            TEXT("En su mano, una campanilla. Nilo no aparece."),
            TEXT("La cinta se rebobina..."),
        };
        for (const TCHAR* Line : Lines)
        {
            Y += DrawTextAt(Line, CenterX, Y, FLinearColor(0.88f, 0.9f, 0.95f), 26.0f, false, 0.5f) + 14.0f * UIScale;
        }
        Y += 26.0f * UIScale;
        Y += DrawWrapped(TEXT("Ayuda a Ivo y a Nilo a escapar juntos de las películas. Lee el OBJETIVO arriba a la izquierda y sigue el rombo de color."),
            CenterX, Y, FMath::Min(900.0f * UIScale, W * 0.8f), HudYellow, 19.0f, true, 0.5f) + 30.0f * UIScale;
        DrawMenuButtons(Controller, CenterX, Y, 380.0f * UIScale);
        return;
    }

    if (Screen == ETUCMenuScreen::Pause)
    {
        float Y = H * 0.18f;
        Y += DrawTextAt(TEXT("PAUSA"), CenterX, Y, HudYellow, 48.0f, true, 0.5f) + 10.0f * UIScale;
        if (Director)
        {
            Y += DrawTextAt(Director->GetChapterTitle().ToString(), CenterX, Y, HudMuted, 18.0f, false, 0.5f) + 6.0f * UIScale;
            Y += DrawWrapped(FString::Printf(TEXT("Objetivo: %s"), *Director->GetCurrentObjective().ToString()),
                CenterX, Y, W * 0.6f, FLinearColor::White, 18.0f, true, 0.5f) + 30.0f * UIScale;
        }
        DrawMenuButtons(Controller, CenterX, Y, 420.0f * UIScale);
        return;
    }

    // Menú principal.
    float Y = H * 0.14f;
    Y += DrawTextAt(TEXT("THE UNFINISHED CUT"), CenterX, Y, HudYellow, 64.0f, true, 0.5f) + 6.0f * UIScale;
    Y += DrawTextAt(TEXT("IVO + NILO  ·  UNA AVENTURA ENTRE PELÍCULAS"), CenterX, Y, HudCyan, 20.0f, true, 0.5f) + 8.0f * UIScale;
    Y += DrawTextAt(TEXT("Un chico y su gato, atrapados en un cine donde cada sala es otra película."), CenterX, Y, HudMuted, 17.0f, false, 0.5f) + 50.0f * UIScale;

    DrawMenuButtons(Controller, CenterX, Y, 460.0f * UIScale);

    if (Director && Director->GetMemoryCount() > 0)
    {
        DrawTextAt(FString::Printf(TEXT("Fragmentos del Ancla: %d / %d"), Director->GetMemoryCount(), TUC::FragmentCount),
            CenterX, H - 110.0f * UIScale, HudOrange, 16.0f, false, 0.5f);
    }
    DrawTextAt(TEXT("Ratón o flechas para elegir  ·  clic o ENTER para aceptar  ·  la partida se guarda sola"),
        CenterX, H - 70.0f * UIScale, HudMuted, 15.0f, false, 0.5f);
}

void ATUCHUD::DrawEnding(const ATUCPuzzleDirector* Director)
{
    const float W = Canvas->ClipX;
    const float H = Canvas->ClipY;
    const float CenterX = W * 0.5f;
    DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.82f), 0.0f, 0.0f, W, H);

    float Y = H * 0.25f;
    Y += DrawTextAt(Director->GetEndingTitle().ToString(), CenterX, Y, HudYellow, 48.0f, true, 0.5f) + 30.0f * UIScale;
    Y += DrawWrapped(Director->GetEndingEpilogue().ToString(), CenterX, Y, W * 0.6f, FLinearColor::White, 22.0f, false, 0.5f, 8.0f) + 36.0f * UIScale;
    if (Director->GetEnding() != ETUCEnding::Together && Director->GetMemoryCount() < TUC::FragmentCount)
    {
        Y += DrawTextAt(FString::Printf(TEXT("Existe otro final. Reúne los fragmentos amarillos del Ancla (%d/%d)."), Director->GetMemoryCount(), TUC::FragmentCount),
            CenterX, Y, HudOrange, 18.0f, false, 0.5f) + 10.0f * UIScale;
    }
    Y += DrawTextAt(TEXT("THE UNFINISHED CUT  ·  FIN  ·  Gracias por jugar"), CenterX, Y, HudCyan, 20.0f, true, 0.5f) + 30.0f * UIScale;
    DrawTextAt(TEXT("ENTER / A — volver al menú principal"), CenterX, Y, FLinearColor::White, 20.0f, true, 0.5f);
}

// ───────────────────────── juego ─────────────────────────

void ATUCHUD::DrawGameplay(ATUCPlayerController* Controller, const ATUCPuzzleDirector* Director)
{
    const float W = Canvas->ClipX;
    const float H = Canvas->ClipY;
    const float CenterX = W * 0.5f;
    const float CenterY = H * 0.5f;
    const float Margin = 28.0f * UIScale;
    const double Now = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;
    const float Pulse = 0.65f + 0.35f * FMath::Sin(static_cast<float>(Now) * 5.0f);

    const ATUCCharacterBase* Character = Controller ? Cast<ATUCCharacterBase>(Controller->GetPawn()) : nullptr;
    const ETUCCharacterType Active = Character ? Character->GetCharacterType() : ETUCCharacterType::Ivo;

    // Objetivo actual (actor) y quién debe usarlo.
    const ATUCInteractionActor* Target = nullptr;
    if (Director)
    {
        for (TActorIterator<ATUCInteractionActor> It(GetWorld()); It; ++It)
        {
            if (!It->IsConsumed() && Director->IsCurrentObjective(It->GetKind()))
            {
                Target = *It;
                break;
            }
        }
    }
    ETUCCharacterType Required = ETUCCharacterType::Ivo;
    const bool bHasRequired = Target && ATUCPuzzleDirector::GetRequiredCharacter(Target->GetKind(), Required);

    // ── Tarjeta de objetivo (arriba a la izquierda) ──
    if (Director)
    {
        const float CardW = FMath::Min(640.0f * UIScale, W * 0.42f);
        const float Pad = 20.0f * UIScale;
        const float TextW = CardW - Pad * 2.0f;
        const FString Objective = Director->GetCurrentObjective().ToString();
        const FString Detail = Director->GetObjectiveDetail().ToString();
        const bool bWrongCharacter = bHasRequired && Required != Active;

        // Altura estimada a partir del texto envuelto.
        const float ObjH = WrapLines(Objective, MakeFont(24.0f, true), TextW).Num() * Measure(TEXT("Ag"), MakeFont(24.0f, true)).Y;
        const float DetH = WrapLines(Detail, MakeFont(17.0f, false), TextW).Num() * (Measure(TEXT("Ag"), MakeFont(17.0f, false)).Y + 3.0f * UIScale);
        const float CardH = Pad * 2.0f + 30.0f * UIScale + 26.0f * UIScale + ObjH + 10.0f * UIScale + DetH
            + (bHasRequired ? 46.0f * UIScale : 0.0f) + (bWrongCharacter ? 40.0f * UIScale : 0.0f) + 28.0f * UIScale;

        DrawPanel(Margin, Margin, CardW, CardH, HudPanel, HudPanelBorder);
        DrawRect(HudYellow, Margin, Margin, 6.0f * UIScale, CardH);

        const float X = Margin + Pad;
        float Y = Margin + Pad;
        Y += DrawTextAt(Director->GetChapterTitle().ToString(), X, Y, HudYellow, 16.0f, true) + 4.0f * UIScale;
        Y += DrawTextAt(TEXT("OBJETIVO"), X, Y, HudMuted, 13.0f, true) + 4.0f * UIScale;
        Y += DrawWrapped(Objective, X, Y, TextW, FLinearColor::White, 24.0f, true, 0.0f, 0.0f) + 8.0f * UIScale;
        Y += DrawWrapped(Detail, X, Y, TextW, FLinearColor(0.85f, 0.88f, 0.94f), 17.0f, false, 0.0f, 3.0f) + 8.0f * UIScale;

        if (bHasRequired)
        {
            const float ChipH = 34.0f * UIScale;
            DrawTextAt(TEXT("LO HACE:"), X, Y + 6.0f * UIScale, HudMuted, 15.0f, true);
            const float ChipX = X + 100.0f * UIScale;
            const float ChipW = 110.0f * UIScale;
            DrawRect(ColorFor(Required), ChipX, Y, ChipW, ChipH);
            DrawTextAt(NameFor(Required), ChipX + ChipW * 0.5f, Y + 3.0f * UIScale, FLinearColor(0.02f, 0.02f, 0.03f), 18.0f, true, 0.5f);
            if (!bWrongCharacter)
            {
                DrawTextAt(TEXT("(ya estás con él)"), ChipX + ChipW + 14.0f * UIScale, Y + 6.0f * UIScale, HudGreen, 15.0f, true);
            }
            Y += ChipH + 12.0f * UIScale;
        }
        if (bWrongCharacter)
        {
            FLinearColor WarnColor = HudOrange;
            WarnColor.A = Pulse;
            Y += DrawTextAt(FString::Printf(TEXT("Estás con %s: pulsa TAB para cambiar a %s"), NameFor(Active), NameFor(Required)),
                X, Y, WarnColor, 19.0f, true) + 10.0f * UIScale;
        }
        DrawTextAt(FString::Printf(TEXT("Fragmentos del Ancla (opcional): %d / %d"), Director->GetMemoryCount(), TUC::FragmentCount),
            X, Y, HudOrange, 14.0f, false);
    }

    // ── Personaje activo (arriba a la derecha) ──
    if (Character)
    {
        const float BoxW = 300.0f * UIScale;
        const float BoxH = 112.0f * UIScale;
        const float BoxX = W - Margin - BoxW;
        DrawPanel(BoxX, Margin, BoxW, BoxH, HudPanel, FLinearColor(ColorFor(Active).R, ColorFor(Active).G, ColorFor(Active).B, 0.6f));
        DrawTextAt(TEXT("JUGANDO CON"), BoxX + 18.0f * UIScale, Margin + 12.0f * UIScale, HudMuted, 13.0f, true);
        DrawTextAt(NameFor(Active), BoxX + 18.0f * UIScale, Margin + 30.0f * UIScale, ColorFor(Active), 32.0f, true);
        if (const ATUCCharacterBase* Partner = Character->GetPartner())
        {
            const ETUCCharacterType Other = Partner->GetCharacterType();
            DrawTextAt(FString::Printf(TEXT("%s %s  ·  TAB cambiar"), NameFor(Other), Partner->IsFollowingPartner() ? TEXT("te sigue") : TEXT("espera (F)")),
                BoxX + 18.0f * UIScale, Margin + 80.0f * UIScale, HudMuted, 14.0f, false);
        }

        if (Active == ETUCCharacterType::Ivo)
        {
            const float Dot = 6.0f * UIScale;
            DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f), CenterX - Dot * 0.5f - 1.0f, CenterY - Dot * 0.5f - 1.0f, Dot + 2.0f, Dot + 2.0f);
            DrawRect(FLinearColor::White, CenterX - Dot * 0.5f, CenterY - Dot * 0.5f, Dot, Dot);
        }

        // ── Aviso de interacción ──
        FText Prompt;
        bool bCanInteract = false;
        if (Character->GetFocusedInteraction(Prompt, bCanInteract))
        {
            const FString PromptText = Prompt.ToString();
            const FVector2D Size = Measure(PromptText, MakeFont(24.0f, true));
            const float KeyW = bCanInteract ? 56.0f * UIScale : 0.0f;
            const float BoxWidth = static_cast<float>(Size.X) + KeyW + 48.0f * UIScale;
            const float BoxHeight = static_cast<float>(Size.Y) + 26.0f * UIScale;
            const float PX = CenterX - BoxWidth * 0.5f;
            const float PY = H * 0.70f;
            DrawPanel(PX, PY, BoxWidth, BoxHeight, HudPanel, bCanInteract ? HudYellow : FLinearColor(1.0f, 1.0f, 1.0f, 0.25f));
            float TextX = PX + 24.0f * UIScale;
            if (bCanInteract)
            {
                DrawRect(HudYellow, TextX, PY + BoxHeight * 0.5f - 20.0f * UIScale, 40.0f * UIScale, 40.0f * UIScale);
                DrawTextAt(TEXT("E"), TextX + 20.0f * UIScale, PY + BoxHeight * 0.5f - 17.0f * UIScale, FLinearColor(0.02f, 0.02f, 0.03f), 22.0f, true, 0.5f);
                TextX += KeyW;
            }
            DrawTextAt(PromptText, TextX, PY + 13.0f * UIScale, bCanInteract ? FLinearColor::White : HudMuted, 24.0f, true);
        }
    }

    // ── Marcador del objetivo ──
    if (Target && Controller && Character)
    {
        DrawObjectiveMarker(Target, Director);
    }

    if (Director)
    {
        // Tarjeta de capítulo al cambiar de película.
        const FString Chapter = Director->GetChapterTitle().ToString();
        if (Chapter != ShownChapter)
        {
            ShownChapter = Chapter;
            ChapterCardStart = Now;
        }
        const double CardAge = Now - ChapterCardStart;
        if (CardAge >= 0.0 && CardAge < 4.0)
        {
            const float Alpha = static_cast<float>(FMath::Clamp(FMath::Min(CardAge / 0.4, (4.0 - CardAge) / 0.8), 0.0, 1.0));
            FLinearColor CardColor = HudYellow;
            CardColor.A = Alpha;
            DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f * Alpha), 0.0f, CenterY - 80.0f * UIScale, W, 130.0f * UIScale);
            DrawTextAt(Chapter, CenterX, CenterY - 50.0f * UIScale, CardColor, 40.0f, true, 0.5f);
        }

        // Mensajes de historia / feedback (centro superior, con fondo).
        FText Message;
        FLinearColor MessageColor;
        float MessageAlpha = 1.0f;
        if (Director->GetActiveMessage(Message, MessageColor, MessageAlpha))
        {
            const FSlateFontInfo Font = MakeFont(22.0f, true);
            const float MaxW = FMath::Min(900.0f * UIScale, W * 0.5f);
            const TArray<FString> Lines = WrapLines(Message.ToString(), Font, MaxW);
            const float LineH = static_cast<float>(Measure(TEXT("Ag"), Font).Y) + 4.0f * UIScale;
            const float BoxH = LineH * FMath::Min(Lines.Num(), 3) + 24.0f * UIScale;
            const float BoxY = H * 0.16f;
            DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f * MessageAlpha), CenterX - MaxW * 0.5f - 20.0f * UIScale, BoxY, MaxW + 40.0f * UIScale, BoxH);
            MessageColor.A = MessageAlpha;
            float MY = BoxY + 12.0f * UIScale;
            for (int32 LineIndex = 0; LineIndex < Lines.Num() && LineIndex < 3; ++LineIndex)
            {
                MY += DrawTextAt(Lines[LineIndex], CenterX, MY, MessageColor, 22.0f, true, 0.5f) + 4.0f * UIScale;
            }
        }
    }

    // ── Tutorial de los primeros segundos de una partida nueva ──
    if (Controller && Director && Director->GetProgressStep() == 0)
    {
        const float Seconds = Controller->GetSecondsSinceGameplayStart();
        if (Seconds >= 4.0f && Seconds < 26.0f)
        {
            const float Alpha = FMath::Clamp(FMath::Min((Seconds - 4.0f) / 0.5f, (26.0f - Seconds) / 1.0f), 0.0f, 1.0f);
            const float TW = FMath::Min(760.0f * UIScale, W * 0.6f);
            const float TH = 168.0f * UIScale;
            const float TX = CenterX - TW * 0.5f;
            const float TY = H * 0.36f;
            DrawPanel(TX, TY, TW, TH, FLinearColor(0.02f, 0.025f, 0.04f, 0.85f * Alpha), FLinearColor(1.0f, 0.82f, 0.25f, 0.6f * Alpha));
            const FLinearColor TextColor(1.0f, 1.0f, 1.0f, Alpha);
            float Y = TY + 16.0f * UIScale;
            Y += DrawTextAt(TEXT("CÓMO EMPEZAR"), CenterX, Y, FLinearColor(1.0f, 0.82f, 0.25f, Alpha), 20.0f, true, 0.5f) + 8.0f * UIScale;
            Y += DrawTextAt(TEXT("1. Lee tu OBJETIVO arriba a la izquierda."), TX + 30.0f * UIScale, Y, TextColor, 18.0f) + 4.0f * UIScale;
            Y += DrawTextAt(TEXT("2. Camina (WASD) hacia el rombo de color: marca a dónde ir."), TX + 30.0f * UIScale, Y, TextColor, 18.0f) + 4.0f * UIScale;
            DrawTextAt(TEXT("3. Cuando veas el aviso [E], púlsalo. TAB cambia entre Ivo y Nilo."), TX + 30.0f * UIScale, Y, TextColor, 18.0f);
        }
    }

    // ── Medidor de detección ──
    float MaxDetection = 0.0f;
    for (TActorIterator<ATUCWatcher> It(GetWorld()); It; ++It)
    {
        MaxDetection = FMath::Max(MaxDetection, It->GetDetection());
    }
    if (MaxDetection > 0.02f)
    {
        const float BarWidth = 460.0f * UIScale;
        const float BarHeight = 16.0f * UIScale;
        const float BarY = H * 0.08f;
        FLinearColor Warn = HudRed;
        Warn.A = Pulse;
        DrawTextAt(TEXT("¡TE ESTÁN VIENDO! SAL DE LA LUZ ROJA"), CenterX, BarY - 34.0f * UIScale, Warn, 20.0f, true, 0.5f);
        DrawRect(FLinearColor(0.1f, 0.0f, 0.0f, 0.75f), CenterX - BarWidth * 0.5f, BarY, BarWidth, BarHeight);
        DrawRect(FLinearColor(1.0f, 0.1f, 0.05f, 0.95f), CenterX - BarWidth * 0.5f, BarY, BarWidth * MaxDetection, BarHeight);
    }

    // ── Controles (abajo) ──
    const FString Controls = TEXT("WASD mover  ·  Ratón mirar  ·  E usar  ·  TAB cambiar personaje  ·  F seguir/esperar  ·  P menú");
    const FVector2D ControlsSize = Measure(Controls, MakeFont(15.0f, false));
    DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.5f), CenterX - ControlsSize.X * 0.5f - 16.0f * UIScale, H - 52.0f * UIScale,
        static_cast<float>(ControlsSize.X) + 32.0f * UIScale, static_cast<float>(ControlsSize.Y) + 12.0f * UIScale);
    DrawTextAt(Controls, CenterX, H - 46.0f * UIScale, HudMuted, 15.0f, false, 0.5f);
}

void ATUCHUD::DrawObjectiveMarker(const ATUCInteractionActor* Target, const ATUCPuzzleDirector* Director)
{
    APlayerController* PlayerController = GetOwningPlayerController();
    const APawn* Pawn = PlayerController ? PlayerController->GetPawn() : nullptr;
    if (!Pawn)
    {
        return;
    }

    const float W = Canvas->ClipX;
    const float H = Canvas->ClipY;
    const float CenterX = W * 0.5f;
    const float CenterY = H * 0.5f;

    FVector Origin;
    FVector Extent;
    Target->GetActorBounds(true, Origin, Extent);
    const FVector MarkerWorld = Origin + FVector(0.0f, 0.0f, Extent.Z + 50.0f);
    const float Meters = static_cast<float>(FVector::Dist(Pawn->GetActorLocation(), MarkerWorld) / 100.0);

    ETUCCharacterType Required = ETUCCharacterType::Ivo;
    const bool bHasRequired = ATUCPuzzleDirector::GetRequiredCharacter(Target->GetKind(), Required);
    const FLinearColor MarkerColor = bHasRequired ? ColorFor(Required) : HudYellow;
    const FString Label = bHasRequired ? FString::Printf(TEXT("%s  ·  %.0f m"), NameFor(Required), Meters) : FString::Printf(TEXT("%.0f m"), Meters);

    FVector ViewLocation;
    FRotator ViewRotation;
    PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
    const FVector Local = ViewRotation.UnrotateVector(MarkerWorld - ViewLocation);

    FVector2D Screen;
    const float EdgeMargin = 90.0f * UIScale;
    const bool bProjected = Local.X > 1.0f && PlayerController->ProjectWorldLocationToScreen(MarkerWorld, Screen, true);
    const bool bOnScreen = bProjected && Screen.X > EdgeMargin && Screen.X < W - EdgeMargin && Screen.Y > EdgeMargin && Screen.Y < H - EdgeMargin;
    const float Pulse = 0.75f + 0.25f * FMath::Sin(static_cast<float>(GetWorld()->GetRealTimeSeconds()) * 5.0f);

    if (bOnScreen)
    {
        const float SX = static_cast<float>(Screen.X);
        const float SY = static_cast<float>(Screen.Y);
        // Rombo dibujado con franjas horizontales.
        const float Radius = 18.0f * UIScale;
        const int32 Rows = 18;
        for (int32 Row = 0; Row < Rows; ++Row)
        {
            const float T = (Row + 0.5f) / Rows * 2.0f - 1.0f;
            const float HalfWidth = Radius * (1.0f - FMath::Abs(T));
            const float RowY = SY + T * Radius;
            DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.7f), SX - HalfWidth - 3.0f, RowY - Radius / Rows - 1.0f, HalfWidth * 2.0f + 6.0f, 2.0f * Radius / Rows + 2.0f);
        }
        for (int32 Row = 0; Row < Rows; ++Row)
        {
            const float T = (Row + 0.5f) / Rows * 2.0f - 1.0f;
            const float HalfWidth = Radius * (1.0f - FMath::Abs(T));
            const float RowY = SY + T * Radius;
            DrawRect(MarkerColor * Pulse, SX - HalfWidth, RowY - Radius / Rows, HalfWidth * 2.0f, 2.0f * Radius / Rows + 0.5f);
        }
        const FVector2D Size = Measure(Label, MakeFont(18.0f, true));
        DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.65f), SX - Size.X * 0.5f - 10.0f * UIScale, SY + Radius + 6.0f * UIScale,
            static_cast<float>(Size.X) + 20.0f * UIScale, static_cast<float>(Size.Y) + 6.0f * UIScale);
        DrawTextAt(Label, SX, SY + Radius + 9.0f * UIScale, MarkerColor, 18.0f, true, 0.5f);
        return;
    }

    // Fuera de pantalla (o detrás): flecha en el borde hacia donde girar.
    float DirX = static_cast<float>(Local.Y);
    float DirY = static_cast<float>(-Local.Z);
    if (Local.X < 0.0f && FMath::Abs(DirX) < 1.0f)
    {
        DirX = 1.0f; // justo detrás: indicar girar a la derecha
    }
    const float DirLength = FMath::Sqrt(DirX * DirX + DirY * DirY);
    if (DirLength < KINDA_SMALL_NUMBER)
    {
        DirX = 1.0f;
        DirY = 0.0f;
    }
    else
    {
        DirX /= DirLength;
        DirY /= DirLength;
    }
    const float HalfW = CenterX - EdgeMargin;
    const float HalfH = CenterY - EdgeMargin;
    const float ToEdgeX = FMath::Abs(DirX) > KINDA_SMALL_NUMBER ? HalfW / FMath::Abs(DirX) : BIG_NUMBER;
    const float ToEdgeY = FMath::Abs(DirY) > KINDA_SMALL_NUMBER ? HalfH / FMath::Abs(DirY) : BIG_NUMBER;
    const float ScaleToEdge = FMath::Min(ToEdgeX, ToEdgeY);
    const float EdgeX = CenterX + DirX * ScaleToEdge;
    const float EdgeY = CenterY + DirY * ScaleToEdge;

    const bool bHorizontal = FMath::Abs(DirX) >= FMath::Abs(DirY);
    const FString Arrow = bHorizontal ? (DirX > 0.0f ? TEXT("GIRA  >>") : TEXT("<<  GIRA")) : (DirY > 0.0f ? TEXT("ABAJO") : TEXT("ARRIBA"));
    const float BoxW = 190.0f * UIScale;
    const float BoxH = 78.0f * UIScale;
    DrawPanel(EdgeX - BoxW * 0.5f, EdgeY - BoxH * 0.5f, BoxW, BoxH, FLinearColor(0.0f, 0.0f, 0.0f, 0.7f), MarkerColor * Pulse, 3.0f);
    DrawTextAt(Arrow, EdgeX, EdgeY - 32.0f * UIScale, MarkerColor, 22.0f, true, 0.5f);
    DrawTextAt(Label, EdgeX, EdgeY + 4.0f * UIScale, FLinearColor::White, 16.0f, true, 0.5f);
}
