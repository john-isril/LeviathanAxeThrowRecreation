using UnrealBuildTool;
 
public class Interfaces : ModuleRules
{
	public Interfaces(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine"});
 
		PublicIncludePaths.AddRange(new string[] {"Interfaces/Public"});
		PrivateIncludePaths.AddRange(new string[] {"Interfaces/Private"});
	}
}