#include "TUCGameMode.h"

#include "TUCBlockoutBuilder.h"
#include "TUCCharacterBase.h"
#include "TUCHUD.h"
#include "TUCIvoCharacter.h"
#include "TUCNiloCharacter.h"
#include "TUCPlayerController.h"
#include "TUCPuzzleDirector.h"
#include "TUCWatcher.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    constexpr float CheckpointGraceSeconds = 2.5f;
    constexpr float IvoSpawnZ = 95.0f;
    const FVector NiloLocalOffset(-60.0f, 110.0f, 0.0f);
}

ATUCGameMode::ATUCGameMode()
{
    DefaultPawnClass = nullptr;
    PlayerControllerClass = ATUCPlayerController::StaticClass();
    HUDClass = ATUCHUD::StaticClass();
}

void ATUCGameMode::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // Orden explícito (la v1.0 dependía del orden de BeginPlay y no aplicaba el guardado al cargar):
    // 1) estado lógico, 2) mundo, 3) personajes, 4) amenazas, 5) checkpoint.
    PuzzleDirector = World->SpawnActor<ATUCPuzzleDirector>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (PuzzleDirector)
    {
        PuzzleDirector->InitializeFromSave();
    }

    BlockoutBuilder = World->SpawnActor<ATUCBlockoutBuilder>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (BlockoutBuilder)
    {
        BlockoutBuilder->BuildWorld(PuzzleDirector);
    }
    if (PuzzleDirector && PuzzleDirector->IsParallelWorldActive())
    {
        ActivateParallelWorld();
    }

    World->SpawnActor<ATUCIvoCharacter>(FVector(-900.0f, -70.0f, IvoSpawnZ), FRotator::ZeroRotator, SpawnParams);
    World->SpawnActor<ATUCNiloCharacter>(FVector(-960.0f, 40.0f, 50.0f), FRotator::ZeroRotator, SpawnParams);

    SpawnWatchers();
    ResetPlayersToCheckpoint();
}

void ATUCGameMode::SpawnWatchers()
{
    struct FWatcherSpec
    {
        float X;
        int32 MinStep;
        int32 MaxStep;
    };

    // Una patrulla por capítulo, solo activa mientras se juega ese capítulo.
    // Ningún checkpoint queda dentro de su radio de visión.
    const FWatcherSpec Specs[] =
    {
        { 1250.0f, 0, 2 },   // Cine: vigila el proyector y el conducto
        { 2400.0f, 3, 4 },   // Años 80: entre el rollo y la tarima
        { 3650.0f, 5, 7 },   // Futuro: cruza la ruta terminal → ducto → proyector
        { 5150.0f, 8, 8 },   // Archivo: custodia el núcleo
    };

    for (const FWatcherSpec& Spec : Specs)
    {
        const FVector PatrolA(Spec.X, -330.0f, 55.0f);
        const FVector PatrolB(Spec.X, 330.0f, 55.0f);
        if (ATUCWatcher* Watcher = GetWorld()->SpawnActor<ATUCWatcher>(PatrolA, FRotator::ZeroRotator))
        {
            Watcher->ConfigurePatrol(PatrolA, PatrolB, Spec.MinStep, Spec.MaxStep);
        }
    }
}

void ATUCGameMode::GetCheckpoint(int32 Step, FVector& OutIvoLocation, float& OutYaw)
{
    OutYaw = 0.0f;
    switch (Step)
    {
        case 0:  OutIvoLocation = FVector(-900.0f, -70.0f, IvoSpawnZ); break;  // Calle, frente al cine
        case 1:  OutIvoLocation = FVector(300.0f, -250.0f, IvoSpawnZ); break;  // Junto a la consola
        case 2:  OutIvoLocation = FVector(600.0f, -150.0f, IvoSpawnZ); break;  // Vestíbulo, fuera de la patrulla
        case 3:  OutIvoLocation = FVector(1780.0f, -150.0f, IvoSpawnZ); break; // Llegada a los años 80
        case 4:  OutIvoLocation = FVector(1800.0f, 150.0f, IvoSpawnZ); break;  // Junto al rollo recuperado
        case 5:  OutIvoLocation = FVector(3150.0f, -120.0f, IvoSpawnZ); break; // Llegada al futuro
        case 6:  OutIvoLocation = FVector(3150.0f, -250.0f, IvoSpawnZ); break; // Junto al terminal
        case 7:  OutIvoLocation = FVector(4200.0f, 150.0f, IvoSpawnZ); OutYaw = -90.0f; break; // Tras el ducto
        case 8:  OutIvoLocation = FVector(4580.0f, 0.0f, IvoSpawnZ); break;    // Vestíbulo del Archivo
        default: OutIvoLocation = FVector(5150.0f, 0.0f, IvoSpawnZ); break;    // Sala conocida / finales
    }
}

void ATUCGameMode::ResetPlayersToCheckpoint()
{
    const int32 Step = PuzzleDirector ? PuzzleDirector->GetProgressStep() : 0;

    FVector IvoLocation;
    float Yaw = 0.0f;
    GetCheckpoint(Step, IvoLocation, Yaw);
    const FRotator Facing(0.0f, Yaw, 0.0f);
    const FVector NiloLocation = IvoLocation + Facing.RotateVector(NiloLocalOffset) + FVector(0.0f, 0.0f, -45.0f);

    for (TActorIterator<ATUCCharacterBase> It(GetWorld()); It; ++It)
    {
        ATUCCharacterBase* Character = *It;
        const bool bIsIvo = Character->GetCharacterType() == ETUCCharacterType::Ivo;
        Character->SetActorLocationAndRotation(bIsIvo ? IvoLocation : NiloLocation, Facing, false, nullptr, ETeleportType::TeleportPhysics);
        Character->GetCharacterMovement()->StopMovementImmediately();

        if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
        {
            PlayerController->SetControlRotation(Facing);
        }
    }

    LastCheckpointResetTime = GetWorld()->GetTimeSeconds();
}

bool ATUCGameMode::IsInCheckpointGrace() const
{
    const UWorld* World = GetWorld();
    return World && (World->GetTimeSeconds() - LastCheckpointResetTime) < CheckpointGraceSeconds;
}

void ATUCGameMode::ActivateParallelWorld()
{
    if (BlockoutBuilder)
    {
        BlockoutBuilder->SetParallelLook(true);
    }
}

void ATUCGameMode::TravelToCampaignStage(int32 Stage)
{
    // La tarjeta de capítulo la dibuja el HUD al detectar el cambio de capítulo.
    ActivateParallelWorld();
    ResetPlayersToCheckpoint();
}

void ATUCGameMode::RestartCampaign()
{
    if (PuzzleDirector)
    {
        PuzzleDirector->ResetProgress();
    }
    if (BlockoutBuilder)
    {
        BlockoutBuilder->ResetWorld();
    }
    for (TActorIterator<ATUCCharacterBase> It(GetWorld()); It; ++It)
    {
        It->SetFollowPartner(true);
    }
    ResetPlayersToCheckpoint();
}

void ATUCGameMode::HandleCharacterFell(ATUCCharacterBase* Character)
{
    if (!Character)
    {
        return;
    }

    if (Character->IsPlayerControlled())
    {
        if (PuzzleDirector)
        {
            PuzzleDirector->PushMessage(TEXT("Fuera del set — la película rebobina al último punto seguro."), FLinearColor(1.0f, 0.5f, 0.2f), 3.0f);
        }
        ResetPlayersToCheckpoint();
    }
    else if (Character->GetPartner())
    {
        Character->RegroupNear(Character->GetPartner());
    }
}
