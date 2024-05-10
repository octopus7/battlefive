// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class battlefive : ModuleRules
{
	public battlefive(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        // 인클루드는 상관없는듯 혹시 재생성 해야하나
        //PrivateIncludePathModuleNames.AddRange(new string[] { "EnhancedInput/Runtime", "EnhancedInput/Public", "EnhancedInput/Classes", "EnhancedInput/Private" });
        //PublicIncludePaths.AddRange(new string[] { "EnhancedInput/Runtime", "EnhancedInput/Public", "EnhancedInput/Classes", "EnhancedInput/Private" });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
