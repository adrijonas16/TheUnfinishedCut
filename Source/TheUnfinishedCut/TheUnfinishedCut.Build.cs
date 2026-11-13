using UnrealBuildTool;

public class TheUnfinishedCut : ModuleRules
{
    public TheUnfinishedCut(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine", "InputCore", "AssetRegistry", "Slate", "SlateCore"
        });
    }
}
