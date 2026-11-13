#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TUCTypes.h"
#include "TUCBlockoutBuilder.generated.h"

class AActor;
class APointLight;
class ATUCInteractionActor;
class ATUCPuzzleDirector;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class USceneComponent;
class UStaticMesh;

/**
 * Construye en runtime los cuatro sets de la campaña.
 * - Decorado: mallas del kit TUC (Tools/Kit) colocadas desde TUCLayoutData.h con instancias.
 * - Colisión: cajas invisibles con la misma forma que el decorado (fiables aunque cambie el arte).
 * Si el kit no está importado, las cajas se muestran con colores para que el juego siga siendo jugable.
 */
UCLASS()
class THEUNFINISHEDCUT_API ATUCBlockoutBuilder : public AActor
{
    GENERATED_BODY()

public:
    ATUCBlockoutBuilder();

    /** Construye todo el mundo una sola vez. El GameMode la llama antes de generar personajes. */
    void BuildWorld(ATUCPuzzleDirector* Director);

    /** Muestra/oculta la "fractura" del cine (pilares y luces del mundo paralelo). */
    void SetParallelLook(bool bEnabled);

    /** Restaura fragmentos y fractura para una partida nueva. */
    void ResetWorld();

private:
    UPROPERTY(VisibleAnywhere, Category="TUC|World")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY()
    TObjectPtr<UStaticMesh> CubeMesh;

    UPROPERTY()
    TObjectPtr<UMaterialInterface> BaseShapeMaterial;

    UPROPERTY()
    TMap<FName, TObjectPtr<UInstancedStaticMeshComponent>> VisualComponents;

    UPROPERTY()
    TArray<TObjectPtr<AActor>> ParallelActors;

    UPROPERTY()
    TArray<TObjectPtr<APointLight>> ParallelLights;

    UPROPERTY()
    TArray<TObjectPtr<ATUCInteractionActor>> Fragments;

    bool bBuilt = false;
    bool bKitAvailable = false;

    AActor* SpawnCollider(const FString& Name, const FVector& Location, const FVector& Scale, const FLinearColor& Color);
    AActor* SpawnKitActor(const TCHAR* AssetName, const FVector& Location, float Yaw, const FVector& FallbackScale, const FLinearColor& FallbackColor);
    UInstancedStaticMeshComponent* GetVisualComponent(const TCHAR* AssetName);
    ATUCInteractionActor* SpawnInteraction(ATUCPuzzleDirector* Director, ETUCInteractionKind Kind, const FVector& Location, const FLinearColor& Color, const FVector& Scale, const TCHAR* KitAsset, float Yaw, int32 FragmentId = INDEX_NONE);
    APointLight* SpawnLight(const FVector& Location, const FLinearColor& Color, float Intensity, float Radius);
    void SpawnSign(const FString& Text, const FVector& Location, float Yaw, const FColor& Color, float WorldSize);
    void SpawnRoomColliders(const FString& Prefix, float CenterX, float HalfLengthX, const FLinearColor& FloorColor, const FLinearColor& WallColor);
    void SpawnSky();
};
