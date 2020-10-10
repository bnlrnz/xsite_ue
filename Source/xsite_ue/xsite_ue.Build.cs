// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;
using System;

public class xsite_ue : ModuleRules
{
	public xsite_ue(ReadOnlyTargetRules Target) : base(Target)
	{
		
		//DefaultBuildSettings = BuildSettingsVersion.V2;

		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "InputCore", "Json", "JsonUtilities", "ProceduralMeshComponent", "UMG", "RHI", "RenderCore", "ImageWrapper", "Engine"});

		// Uncomment if you are using Slate UI
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // we need to enable normal c++ exceptions to package unreal with vrpn
        bEnableExceptions = true;

        // include and link VRPN Library
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "vrpn.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "quat.lib"));
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "libvrpn_mac.a")); 
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "libquat_mac.a"));
        }
        else
        {
        	// Linux
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "libvrpn.a")); 
            PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "lib", "libquat.a"));
        }
	}
}
