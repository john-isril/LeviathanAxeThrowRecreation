using UnrealBuildTool;
 
public class AxeCombat : ModuleRules
{
	public AxeCombat(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "Niagara", "Interfaces"});
 
		PublicIncludePaths.AddRange(new string[] {"AxeCombat/Public"});
		PrivateIncludePaths.AddRange(new string[] {"AxeCombat/Private"});
	}
}