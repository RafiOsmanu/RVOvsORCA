// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class ORCAvsRVO : ModuleRules
{
	public ORCAvsRVO(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "NavigationSystem", "AIModule", "Niagara" });

        PublicIncludePaths.AddRange(new string[] { Path.Combine(ModuleDirectory, "../ThirdParty/CLP/include") });

        PublicAdditionalLibraries.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "../ThirdParty/CLP/lib/libClp.lib"),
                Path.Combine(ModuleDirectory, "../ThirdParty/CLP/lib/libCoinUtils.lib"),
                Path.Combine(ModuleDirectory, "../ThirdParty/CLP/lib/libOsi.lib"),
                Path.Combine(ModuleDirectory, "../ThirdParty/CLP/lib/libOsiClp.lib"),
                Path.Combine(ModuleDirectory, "../ThirdParty/CLP/lib/libOsiCommonTest.lib")
            });

        //Undefined MACROS
        PublicDefinitions.Add("PRICE_USE_OPENMP=1");
        PublicDefinitions.Add("ABOCA_LITE=1");
        PublicDefinitions.Add("CLP_HAS_ABC=1");
        PublicDefinitions.Add("__BYTE_ORDER=1");
        PublicDefinitions.Add("__LITTLE_ENDIAN=1");

    }

}
