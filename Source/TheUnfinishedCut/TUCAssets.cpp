#include "TUCAssets.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/MeshComponent.h"
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
        ApplyKitMaterials(Component, Mesh);
        return true;
    }

    namespace
    {
        UMaterialInterface* LoadMaterial(const FString& PackagePath)
        {
            return LoadAsset<UMaterialInterface>(*PackagePath);
        }

        FString CleanSlotName(FString Name)
        {
            // "M_Brick.001", "M_Brick_001", "M_Brick_skin" (Blender/FBX) -> "M_Brick"
            int32 Dot = INDEX_NONE;
            if (Name.FindChar(TEXT('.'), Dot))
            {
                Name = Name.Left(Dot);
            }
            int32 Under = INDEX_NONE;
            if (Name.FindLastChar(TEXT('_'), Under) && Under > 0)
            {
                const FString Tail = Name.Mid(Under + 1);
                if (Tail.IsNumeric() || Tail.Equals(TEXT("skin"), ESearchCase::IgnoreCase))
                {
                    Name = Name.Left(Under);
                }
            }
            return Name;
        }
    }

    int32 ApplyKitMaterials(UMeshComponent* Component, const UStaticMesh* Mesh)
    {
        if (!Component || !Mesh)
        {
            return 0;
        }
        const UMaterial* DefaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
        const TArray<FStaticMaterial>& Slots = Mesh->GetStaticMaterials();
        int32 Assigned = 0;
        for (int32 Index = 0; Index < Slots.Num(); ++Index)
        {
            const FString Slot = CleanSlotName(Slots[Index].MaterialSlotName.ToString());

            UMaterialInterface* Material = nullptr;
            if (Slot.StartsWith(TEXT("M_")))
            {
                Material = LoadMaterial(FString::Printf(TEXT("/Game/TUC/Kit/Materials/MI_%s"), *Slot));
            }
            if (!Material)
            {
                const UMaterialInterface* Current = Slots[Index].MaterialInterface;
                const bool bIsDefault = !Current || Current->GetMaterial() == DefaultMaterial;
                if (bIsDefault)
                {
                    // Reemplazos Meshy: un solo material PBR por malla.
                    Material = LoadMaterial(FString::Printf(TEXT("/Game/TUC/Kit/Overrides/M_%s"), *Mesh->GetName()));
                }
            }
            if (Material)
            {
                Component->SetMaterial(Index, Material);
                ++Assigned;
            }
        }
        return Assigned;
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
