using UnrealBuildTool;
using System.Collections.Generic;

public class TheUnfinishedCutEditorTarget : TargetRules
{
    public TheUnfinishedCutEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("TheUnfinishedCut");
    }
}
