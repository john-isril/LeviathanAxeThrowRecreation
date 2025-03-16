using UnrealBuildTool;
 
public class CustomComponents : ModuleRules
{
	public CustomComponents(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine"});
 
		PublicIncludePaths.AddRange(new string[] {"CustomComponents/Public"});
		PrivateIncludePaths.AddRange(new string[] {"CustomComponents/Private"});
	}
}