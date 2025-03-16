// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class TheLeviathanAxeTarget : TargetRules
{
	public TheLeviathanAxeTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.AddRange( new string[] { "TheLeviathanAxe", "UI", "AxeCombat", "Interfaces", "AI", "CustomComponents"} );
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
    }
}
