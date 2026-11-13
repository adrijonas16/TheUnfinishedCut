#include "TUCAssets.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/PackageName.h"

namespace TUCAssets
{
    FString KitMesh(const TCHAR* AssetName)
    {
        return FString::Printf(TEXT("/Game/TUC/Kit/Meshes/%s"), AssetName);
    }

    bool IsKitAvailable()
    {
        return FPackageName::DoesPackageExist(KitMesh(TEXT("SM_Floor_Carpet")));
    }

    UStaticMesh* LoadMesh(const FString& PackagePath)
    {
        if (PackagePath.IsEmpty() || !FPackageName::DoesPackageExist(PackagePath))
        {
            return nullptr;
        }
        const FString ObjectPath = PackagePath + TEXT(".") + FPackageName::GetShortName(PackagePath);
        return LoadObject<UStaticMesh>(nullptr, *ObjectPath);
    }

    bool FitMesh(UStaticMeshComponent* Component, UStaticMesh* Mesh, float TargetSize, bool bByHeight, float YawOffset, float FloorZ)
    {
        if (!Component || !Mesh)
        {
            return false;
        }

        const FBoxSphereBounds Bounds = Mesh->GetBounds();
        const float Reference = bByHeight
            ? 2.0f * Bounds.BoxExtent.Z
            : 2.0f * FMath::Max(Bounds.BoxExtent.X, Bounds.BoxExtent.Y);
        if (Reference <= KINDA_SMALL_NUMBER)
        {
            return false;
        }

        const float Fit = TargetSize / Reference;
        const FRotator YawFix(0.0f, YawOffset, 0.0f);
        const FVector RotatedOrigin = YawFix.RotateVector(Bounds.Origin * Fit);
        const float MeshBottom = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Fit;

        Component->SetStaticMesh(Mesh);
        Component->SetRelativeScale3D(FVector(Fit));
        Component->SetRelativeRotation(YawFix);
        Component->SetRelativeLocation(FVector(-RotatedOrigin.X, -RotatedOrigin.Y, FloorZ - MeshBottom));
        Component->SetVisibility(true);
        return true;
    }

    UMaterialInstanceDynamic* TintAccent(UStaticMeshComponent* Component, const FLinearColor& Color)
    {
        if (!Component)
        {
            return nullptr;
        }
        const int32 SlotIndex = Component->GetMaterialIndex(TEXT("M_AccentGlow"));
        if (SlotIndex == INDEX_NONE)
        {
            return nullptr;
        }
        UMaterialInstanceDynamic* Accent = Component->CreateDynamicMaterialInstance(SlotIndex);
        if (Accent)
        {
            Accent->SetVectorParameterValue(TEXT("Tint"), Color);
        }
        return Accent;
    }
}
