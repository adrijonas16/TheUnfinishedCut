using UnrealBuildTool;
using System.Collections.Generic;

public class TheUnfinishedCutTarget : TargetRules
{
    public TheUnfinishedCutTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("TheUnfinishedCut");
    }
}
