#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TUCWatcher.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class UPointLightComponent;
class USceneComponent;
class USpotLightComponent;
class UStaticMeshComponent;

/**
 * Centinela del GDD (enemigo tipo 1): patrulla entre dos puntos, mira hacia donde camina
 * y detecta con un cono de visión visible en el suelo. La detección se llena poco a poco.
 */
UCLASS()
class THEUNFINISHEDCUT_API ATUCWatcher : public AActor
{
    GENERATED_BODY()

public:
    ATUCWatcher();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    /** Puntos de patrulla y rango de pasos de campaña en los que está activo. */
    void ConfigurePatrol(const FVector& InPatrolA, const FVector& InPatrolB, int32 InMinStep, int32 InMaxStep);

    /** 0..1: cuánto le falta para detectar al jugador (lo muestra el HUD). */
    float GetDetection() const { return bActive ? Detection : 0.0f; }

    bool IsWatcherActive() const { return bActive; }

private:
    UPROPERTY(VisibleAnywhere, Category="TUC|Watcher")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, Category="TUC|Watcher")
    TObjectPtr<UStaticMeshComponent> Body;

    UPROPERTY(VisibleAnywhere, Category="TUC|Watcher")
    TObjectPtr<UStaticMeshComponent> Visor;

    UPROPERTY(VisibleAnywhere, Category="TUC|Watcher")
    TObjectPtr<UPointLightComponent> DetectionLight;

    UPROPERTY(VisibleAnywhere, Category="TUC|Watcher")
    TObjectPtr<USpotLightComponent> VisionCone;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BaseShapeMaterial;

    /** Modelo del kit (SM_Watcher). Si existe, el cilindro queda solo como colisión invisible. */
    UPROPERTY(VisibleAnywhere, Category="TUC|Watcher")
    TObjectPtr<UStaticMeshComponent> KitVisual;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> VisorTint;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> KitAccent;

    float HoverPhase = 0.0f;

    /** El centinela mueve la cabeza de lado a lado mientras patrulla (su visión barre el pasillo). */
    UPROPERTY(EditAnywhere, Category="TUC|Watcher")
    float SweepDegrees = 20.0f;

    float SweepPhase = 0.0f;
    float KitBaseZ = 0.0f;

    UPROPERTY(EditAnywhere, Category="TUC|Watcher")
    float SightRange = 500.0f;

    UPROPERTY(EditAnywhere, Category="TUC|Watcher")
    float SightHalfAngle = 52.0f;

    UPROPERTY(EditAnywhere, Category="TUC|Watcher")
    float PatrolSpeed = 165.0f;

    /** Segundos de visión continua necesarios para detectar. */
    UPROPERTY(EditAnywhere, Category="TUC|Watcher")
    float TimeToDetect = 0.6f;

    FVector PatrolA = FVector::ZeroVector;
    FVector PatrolB = FVector::ZeroVector;
    int32 MinStep = 0;
    int32 MaxStep = 100;
    bool bTowardB = true;
    bool bConfigured = false;
    bool bActive = true;
    float Detection = 0.0f;

    void SetWatcherActive(bool bNewActive);
    void UpdatePatrol(float DeltaSeconds);
    void UpdateDetection(float DeltaSeconds);
    bool CanSeePlayer() const;
};
