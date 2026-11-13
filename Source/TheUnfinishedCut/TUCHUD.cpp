#include "TUCHUD.h"

#include "TUCCharacterBase.h"
#include "TUCInteractionActor.h"
#include "TUCPlayerController.h"
#include "TUCPuzzleDirector.h"
#include "TUCWatcher.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace
{
    const FLinearColor HudYellow(1.0f, 0.82f, 0.25f);
    const FLinearColor HudCyan(0.25f, 0.85f, 1.0f);
    const FLinearColor HudOrange(1.0f, 0.65f, 0.15f);
    const FLinearColor HudMuted(0.62f, 0.68f, 0.78f);
    const FLinearColor HudPanel(0.0f, 0.0f, 0.0f, 0.55f);
}

void ATUCHUD::MeasureText(const FString& Text, UFont* Font, float Scale, float& OutWidth, float& OutHeight)
{
    OutWidth = 0.0f;
    OutHeight = 0.0f;
    if (Font)
    {
        GetTextSize(Text, OutWidth, OutHeight, Font, Scale);
    }
}

float ATUCHUD::DrawLabel(const FString& Text, float X, float Y, const FLinearColor& Color, UFont* Font, float Scale, float HAlign)
{
    if (!Canvas || !Font || Text.IsEmpty())
    {
        return 0.0f;
    }

    float Width = 0.0f;
    float Height = 0.0f;
    MeasureText(Text, Font, Scale, Width, Height);

    FFontRenderInfo RenderInfo;
    RenderInfo.bEnableShadow = true;
    Canvas->SetDrawColor(Color.ToFColor(true));
    Canvas->DrawText(Font, Text, X - Width * HAlign, Y, Scale, Scale, RenderInfo);
    return Height;
}

TArray<FString> ATUCHUD::WrapLines(const FString& Text, UFont* Font, float Scale, float MaxWidth)
{
    TArray<FString> Words;
    Text.ParseIntoArray(Words, TEXT(" "), true);

    TArray<FString> Lines;
    FString Current;
    for (const FString& Word : Words)
    {
        const FString Candidate = Current.IsEmpty() ? Word : Current + TEXT(" ") + Word;
        float Width = 0.0f;
        float Height = 0.0f;
        MeasureText(Candidate, Font, Scale, Width, Height);
        if (Width > MaxWidth && !Current.IsEmpty())
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

void ATUCHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas || !GEngine)
    {
        return;
    }

    UIScale = FMath::Max(0.75f, Canvas->ClipY / 1080.0f);
    const ATUCPlayerController* TUCController = Cast<ATUCPlayerController>(GetOwningPlayerController());
    const ATUCPuzzleDirector* Director = ATUCPuzzleDirector::Get(this);

    if (TUCController && TUCController->IsIntroVisible())
    {
        if (Director)
        {
            ShownChapter = Director->GetChapterTitle().ToString();
        }
        DrawIntro(Director);
        return;
    }

    if (Director && Director->GetProgressStep() >= TUC::FinalStep)
    {
        DrawEnding(Director);
        return;
    }

    DrawGameplay(Director);

    if (TUCController && TUCController->IsPauseMenuVisible())
    {
        const float CenterX = Canvas->ClipX * 0.5f;
        const float CenterY = Canvas->ClipY * 0.5f;
        DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f), 0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY);
        DrawLabel(TEXT("PAUSA"), CenterX, CenterY - 70.0f * UIScale, HudYellow, GEngine->GetLargeFont(), 1.4f * UIScale, 0.5f);
        DrawLabel(TEXT("P — continuar     ·     BACKSPACE — nueva partida"), CenterX, CenterY, FLinearColor::White, GEngine->GetMediumFont(), UIScale, 0.5f);
    }
}

void ATUCHUD::DrawIntro(const ATUCPuzzleDirector* Director)
{
    const float CenterX = Canvas->ClipX * 0.5f;
    float Y = Canvas->ClipY * 0.22f;
    DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.82f), 0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY);

    UFont* Large = GEngine->GetLargeFont();
    UFont* Medium = GEngine->GetMediumFont();
    UFont* Small = GEngine->GetSmallFont();

    if (!Director || !Director->HasSeenPrologue())
    {
        // P0 — prólogo: registro de un universo posible, no una predicción (03_BIBLIA, 04_GUION).
        Y += DrawLabel(TEXT("CORTE 001 — FINAL REGISTRADO"), CenterX, Y, HudOrange, Medium, UIScale, 0.5f) + 18.0f * UIScale;
        const TCHAR* PrologueLines[] =
        {
            TEXT("Un proyector suena en una sala que nadie recuerda."),
            TEXT("Ivo cruza una puerta. Mira atrás. Espera."),
            TEXT("En su mano, una campanilla. Nilo no aparece."),
            TEXT("La cinta se rebobina..."),
        };
        for (const TCHAR* Line : PrologueLines)
        {
            Y += DrawLabel(Line, CenterX, Y, HudMuted, Medium, 0.9f * UIScale, 0.5f) + 8.0f * UIScale;
        }
        Y += 40.0f * UIScale;
    }

    Y += DrawLabel(TEXT("THE UNFINISHED CUT"), CenterX, Y, HudYellow, Large, 1.6f * UIScale, 0.5f) + 10.0f * UIScale;
    Y += DrawLabel(TEXT("IVO + NILO  ·  UNA AVENTURA ENTRE PELÍCULAS"), CenterX, Y, HudCyan, Medium, 0.9f * UIScale, 0.5f) + 40.0f * UIScale;

    if (Director && Director->HasSeenPrologue() && Director->GetProgressStep() > 0 && Director->GetProgressStep() < TUC::FinalStep)
    {
        Y += DrawLabel(FString::Printf(TEXT("ENTER — CONTINUAR  (%s)"), *Director->GetChapterTitle().ToString()), CenterX, Y, FLinearColor::White, Medium, UIScale, 0.5f) + 8.0f * UIScale;
        Y += DrawLabel(FString::Printf(TEXT("Ancla %d/%d  ·  BACKSPACE — nueva partida"), Director->GetMemoryCount(), TUC::FragmentCount), CenterX, Y, HudMuted, Small, UIScale, 0.5f);
    }
    else
    {
        Y += DrawLabel(TEXT("ENTER — COMENZAR"), CenterX, Y, FLinearColor::White, Medium, UIScale, 0.5f) + 8.0f * UIScale;
        Y += DrawLabel(TEXT("La partida se guarda automáticamente"), CenterX, Y, HudMuted, Small, UIScale, 0.5f);
    }
}

void ATUCHUD::DrawEnding(const ATUCPuzzleDirector* Director)
{
    const float CenterX = Canvas->ClipX * 0.5f;
    float Y = Canvas->ClipY * 0.3f;
    DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.78f), 0.0f, 0.0f, Canvas->ClipX, Canvas->ClipY);

    UFont* Large = GEngine->GetLargeFont();
    UFont* Medium = GEngine->GetMediumFont();

    Y += DrawLabel(Director->GetEndingTitle().ToString(), CenterX, Y, HudYellow, Large, 1.4f * UIScale, 0.5f) + 26.0f * UIScale;
    for (const FString& Line : WrapLines(Director->GetEndingEpilogue().ToString(), Medium, 0.9f * UIScale, Canvas->ClipX * 0.55f))
    {
        Y += DrawLabel(Line, CenterX, Y, FLinearColor::White, Medium, 0.9f * UIScale, 0.5f) + 6.0f * UIScale;
    }
    Y += 30.0f * UIScale;
    if (Director->GetEnding() != ETUCEnding::Together && Director->GetMemoryCount() < TUC::FragmentCount)
    {
        Y += DrawLabel(FString::Printf(TEXT("Existe otro corte. Ancla %d/%d."), Director->GetMemoryCount(), TUC::FragmentCount),
            CenterX, Y, HudMuted, Medium, 0.85f * UIScale, 0.5f) + 6.0f * UIScale;
    }
    Y += DrawLabel(TEXT("THE UNFINISHED CUT  ·  FIN  ·  Gracias por jugar"), CenterX, Y, HudCyan, Medium, 0.85f * UIScale, 0.5f) + 10.0f * UIScale;
    DrawLabel(TEXT("BACKSPACE — NUEVA PARTIDA"), CenterX, Y, FLinearColor::White, Medium, 0.85f * UIScale, 0.5f);
}

void ATUCHUD::DrawGameplay(const ATUCPuzzleDirector* Director)
{
    const float CenterX = Canvas->ClipX * 0.5f;
    const float CenterY = Canvas->ClipY * 0.5f;
    const float Margin = 32.0f * UIScale;
    UFont* Large = GEngine->GetLargeFont();
    UFont* Medium = GEngine->GetMediumFont();
    UFont* Small = GEngine->GetSmallFont();

    const APlayerController* PlayerController = GetOwningPlayerController();
    const ATUCCharacterBase* Character = PlayerController ? Cast<ATUCCharacterBase>(PlayerController->GetPawn()) : nullptr;

    // Personaje activo y estado del compañero.
    if (Character)
    {
        const bool bIsIvo = Character->GetCharacterType() == ETUCCharacterType::Ivo;
        DrawLabel(bIsIvo ? TEXT("IVO  ·  PRIMERA PERSONA") : TEXT("NILO  ·  TERCERA PERSONA"), Margin, Margin,
            bIsIvo ? HudCyan : HudOrange, Medium, UIScale);

        if (const ATUCCharacterBase* Partner = Character->GetPartner())
        {
            const FString PartnerName = bIsIvo ? TEXT("Nilo") : TEXT("Ivo");
            DrawLabel(FString::Printf(TEXT("%s: %s   (F / Q)"), *PartnerName, Partner->IsFollowingPartner() ? TEXT("te sigue") : TEXT("espera")),
                Margin, Margin + 30.0f * UIScale, HudMuted, Small, UIScale);
        }

        if (bIsIvo)
        {
            DrawLabel(TEXT("+"), CenterX, CenterY - 10.0f * UIScale, FLinearColor::White, Medium, UIScale, 0.5f);
        }

        FText Prompt;
        bool bCanInteract = false;
        if (Character->GetFocusedInteraction(Prompt, bCanInteract))
        {
            const FString PromptText = bCanInteract ? FString::Printf(TEXT("[E]  %s"), *Prompt.ToString()) : Prompt.ToString();
            float Width = 0.0f;
            float Height = 0.0f;
            MeasureText(PromptText, Medium, UIScale, Width, Height);
            const float PromptY = Canvas->ClipY - 140.0f * UIScale;
            DrawRect(HudPanel, CenterX - Width * 0.5f - 14.0f, PromptY - 6.0f, Width + 28.0f, Height + 12.0f);
            DrawLabel(PromptText, CenterX, PromptY, bCanInteract ? HudYellow : HudMuted, Medium, UIScale, 0.5f);
        }
    }

    if (Director)
    {
        // Objetivo (arriba a la derecha, alineado al borde para cualquier resolución).
        const float RightX = Canvas->ClipX - Margin;
        float Y = Margin;
        const FString Chapter = Director->GetChapterTitle().ToString();
        Y += DrawLabel(Chapter, RightX, Y, HudYellow, Medium, UIScale, 1.0f) + 6.0f * UIScale;
        Y += DrawLabel(Director->GetCurrentObjective().ToString(), RightX, Y, FLinearColor::White, Small, 1.05f * UIScale, 1.0f) + 6.0f * UIScale;
        DrawLabel(FString::Printf(TEXT("ANCLA  %d / %d"), Director->GetMemoryCount(), TUC::FragmentCount), RightX, Y, HudOrange, Small, UIScale, 1.0f);

        // Tarjeta de capítulo al cambiar de película.
        const double Now = GetWorld() ? GetWorld()->GetRealTimeSeconds() : 0.0;
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
            DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f * Alpha), 0.0f, CenterY - 70.0f * UIScale, Canvas->ClipX, 110.0f * UIScale);
            DrawLabel(Chapter, CenterX, CenterY - 45.0f * UIScale, CardColor, Large, 1.3f * UIScale, 0.5f);
        }

        // Mensajes de historia / feedback.
        FText Message;
        FLinearColor MessageColor;
        float MessageAlpha = 1.0f;
        if (Director->GetActiveMessage(Message, MessageColor, MessageAlpha))
        {
            MessageColor.A = MessageAlpha;
            float MessageY = Canvas->ClipY * 0.17f;
            const TArray<FString> MessageLines = WrapLines(Message.ToString(), Medium, UIScale, Canvas->ClipX * 0.6f);
            for (int32 LineIndex = 0; LineIndex < MessageLines.Num() && LineIndex < 3; ++LineIndex)
            {
                MessageY += DrawLabel(MessageLines[LineIndex], CenterX, MessageY, MessageColor, Medium, UIScale, 0.5f) + 4.0f * UIScale;
            }
        }
    }

    // Marcador del objetivo actual: rombo + quién debe usarlo + distancia; flecha en el borde si está fuera de cámara.
    if (Director && PlayerController && Character)
    {
        const ATUCInteractionActor* Target = nullptr;
        for (TActorIterator<ATUCInteractionActor> It(GetWorld()); It; ++It)
        {
            if (!It->IsConsumed() && Director->IsCurrentObjective(It->GetKind()))
            {
                Target = *It;
                break;
            }
        }

        if (Target)
        {
            FVector Origin;
            FVector Extent;
            Target->GetActorBounds(true, Origin, Extent);
            const FVector MarkerWorld = Origin + FVector(0.0f, 0.0f, Extent.Z + 40.0f);
            const float Meters = static_cast<float>(FVector::Dist(Character->GetActorLocation(), MarkerWorld) / 100.0);

            ETUCCharacterType Required = ETUCCharacterType::Ivo;
            const bool bHasRequired = ATUCPuzzleDirector::GetRequiredCharacter(Target->GetKind(), Required);
            const FString Who = !bHasRequired ? FString(TEXT("")) : (Required == ETUCCharacterType::Ivo ? FString(TEXT("IVO · ")) : FString(TEXT("NILO · ")));
            const FLinearColor MarkerColor = !bHasRequired ? HudYellow : (Required == ETUCCharacterType::Ivo ? HudCyan : FLinearColor(0.3f, 1.0f, 0.4f));
            const FString Label = FString::Printf(TEXT("%s%.0f m"), *Who, Meters);

            FVector ViewLocation;
            FRotator ViewRotation;
            PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
            const FVector Local = ViewRotation.UnrotateVector(MarkerWorld - ViewLocation);

            FVector2D Screen;
            const float EdgeMargin = 60.0f * UIScale;
            const bool bProjected = Local.X > 1.0f && PlayerController->ProjectWorldLocationToScreen(MarkerWorld, Screen, true);
            const bool bOnScreen = bProjected && Screen.X > EdgeMargin && Screen.X < Canvas->ClipX - EdgeMargin
                && Screen.Y > EdgeMargin && Screen.Y < Canvas->ClipY - EdgeMargin;

            const float Size = 12.0f * UIScale;
            const float ScreenX = static_cast<float>(Screen.X);
            const float ScreenY = static_cast<float>(Screen.Y);
            if (bOnScreen)
            {
                const float Pulse = 0.75f + 0.25f * FMath::Sin(static_cast<float>(GetWorld()->GetRealTimeSeconds()) * 5.0f);
                DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f), ScreenX - Size * 0.5f - 2.0f, ScreenY - Size * 0.5f - 2.0f, Size + 4.0f, Size + 4.0f);
                DrawRect(MarkerColor * Pulse, ScreenX - Size * 0.5f, ScreenY - Size * 0.5f, Size, Size);
                DrawLabel(Label, ScreenX, ScreenY + Size, MarkerColor, Small, UIScale, 0.5f);
            }
            else
            {
                // Dirección en pantalla según la posición relativa a la cámara (también si está detrás).
                float DirX = static_cast<float>(Local.Y);
                float DirY = static_cast<float>(-Local.Z);
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
                const float HalfW = Canvas->ClipX * 0.5f - EdgeMargin;
                const float HalfH = Canvas->ClipY * 0.5f - EdgeMargin;
                const float ToEdgeX = FMath::Abs(DirX) > KINDA_SMALL_NUMBER ? HalfW / FMath::Abs(DirX) : BIG_NUMBER;
                const float ToEdgeY = FMath::Abs(DirY) > KINDA_SMALL_NUMBER ? HalfH / FMath::Abs(DirY) : BIG_NUMBER;
                const float ScaleToEdge = FMath::Min(ToEdgeX, ToEdgeY);
                const float EdgeX = CenterX + DirX * ScaleToEdge;
                const float EdgeY = CenterY + DirY * ScaleToEdge;
                const TCHAR* Arrow = FMath::Abs(DirX) >= FMath::Abs(DirY)
                    ? (DirX > 0.0f ? TEXT(">>") : TEXT("<<"))
                    : (DirY > 0.0f ? TEXT("vv") : TEXT("^^"));
                DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.6f), EdgeX - 50.0f * UIScale, EdgeY - 16.0f * UIScale, 100.0f * UIScale, 52.0f * UIScale);
                DrawLabel(Arrow, EdgeX, EdgeY - 14.0f * UIScale, MarkerColor, Medium, UIScale, 0.5f);
                DrawLabel(Label, EdgeX, EdgeY + 12.0f * UIScale, MarkerColor, Small, UIScale, 0.5f);
            }
        }
    }

    // Medidor de detección del centinela más alerta.
    float MaxDetection = 0.0f;
    for (TActorIterator<ATUCWatcher> It(GetWorld()); It; ++It)
    {
        MaxDetection = FMath::Max(MaxDetection, It->GetDetection());
    }
    if (MaxDetection > 0.02f)
    {
        const float BarWidth = 320.0f * UIScale;
        const float BarHeight = 10.0f * UIScale;
        const float BarY = Canvas->ClipY * 0.11f;
        DrawLabel(TEXT("TE ESTÁN VIENDO"), CenterX, BarY - 30.0f * UIScale, FLinearColor(1.0f, 0.25f, 0.2f), Small, UIScale, 0.5f);
        DrawRect(FLinearColor(0.1f, 0.0f, 0.0f, 0.7f), CenterX - BarWidth * 0.5f, BarY, BarWidth, BarHeight);
        DrawRect(FLinearColor(1.0f, 0.1f, 0.05f, 0.95f), CenterX - BarWidth * 0.5f, BarY, BarWidth * MaxDetection, BarHeight);
    }

    DrawLabel(TEXT("WASD mover · Ratón mirar · E interactuar · TAB cambiar · F/Q seguir/esperar · P pausa · BACKSPACE nueva partida"),
        Margin, Canvas->ClipY - 40.0f * UIScale, HudMuted, Small, 0.9f * UIScale);
}
