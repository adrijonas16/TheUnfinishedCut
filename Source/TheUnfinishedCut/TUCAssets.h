#pragma once

#include "CoreMinimal.h"
#include "Misc/PackageName.h"

class UMaterialInstanceDynamic;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Acceso a los assets importados por Tools/import_external_assets.py.
 * Todo es tolerante a fallos: si un asset no existe, el juego usa su placeholder.
 */
namespace TUCAssets
{
    /** Ruta de paquete de una malla del kit, p. ej. KitMesh(TEXT("SM_Ivo")) = /Game/TUC/Kit/Meshes/SM_Ivo. */
    FString KitMesh(const TCHAR* AssetName);

    /** true si el kit ya fue importado (comprueba un asset de referencia; sin caché para PIE). */
    bool IsKitAvailable();

    /** Carga una malla por ruta de paquete sin escribir errores si no existe. */
    UStaticMesh* LoadMesh(const FString& PackagePath);

    /**
     * Asigna la malla al componente, la escala para que mida TargetSize (alto si bByHeight,
     * si no, largo horizontal) y apoya su base en FloorZ (coordenadas relativas al padre).
     */
    bool FitMesh(UStaticMeshComponent* Component, UStaticMesh* Mesh, float TargetSize, bool bByHeight, float YawOffset, float FloorZ);

    /** Carga cualquier asset por ruta de paquete (p. ej. /Game/TUC/Characters/Ivo/SK_Ivo) si existe. */
    template <typename T>
    T* LoadAsset(const TCHAR* PackagePath)
    {
        if (!FPackageName::DoesPackageExist(PackagePath))
        {
            return nullptr;
        }
        const FString ObjectPath = FString(PackagePath) + TEXT(".") + FPackageName::GetShortName(PackagePath);
        return LoadObject<T>(nullptr, *ObjectPath);
    }

    /** Tiñe el slot M_AccentGlow (parámetro Tint del material maestro). Devuelve el MID o nullptr. */
    UMaterialInstanceDynamic* TintAccent(UStaticMeshComponent* Component, const FLinearColor& Color);
}
