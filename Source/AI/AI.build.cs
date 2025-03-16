using UnrealBuildTool;
 
public class AI : ModuleRules
{
	public AI(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "Interfaces", "CustomComponents", "UMG", "AIModule", "NavigationSystem" });
 
		PublicIncludePaths.AddRange(new string[] {"AI/Public"});
		PrivateIncludePaths.AddRange(new string[] {"AI/Private"});
	}
}