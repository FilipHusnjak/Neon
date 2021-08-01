workspace "Neon"
	architecture "x64"
	targetdir "build"
	
	configurations 
	{ 
		"Debug", 
		"Release"
	}

	flags
	{
		"MultiProcessorCompile"
	}

	startproject "NeonEditor"
	
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Neon/vendor/GLFW/include"
IncludeDir["ImGui"] = "Neon/vendor/ImGui"
IncludeDir["glm"] = "Neon/vendor/glm"
IncludeDir["entt"] = "Neon/vendor/entt/include"
IncludeDir["shaderc"] = "Neon/vendor/shaderc/include"
IncludeDir["SPIRV_Cross"] = "Neon/vendor/SPIRV-Cross"
IncludeDir["Vulkan"] = "Neon/vendor/Vulkan/1.2.148.1/include"
IncludeDir["PhysX"] = "Neon/vendor/PhysX/include"
IncludeDir["PhysX_internal"] = "Neon/vendor/PhysX/include/PhysX"

LibraryDir = {}

LibraryDir["Vulkan"] = "vendor/Vulkan/1.2.148.1/Lib/vulkan-1.lib"

-- Debug libs
LibraryDir["shaderc_debug"] = "vendor/shaderc/lib/Debug/shaderc.lib"
LibraryDir["shaderc_util_debug"] = "vendor/shaderc/lib/Debug/shaderc_util.lib"

LibraryDir["glslang_debug"] = "vendor/glslang/lib/Debug/glslangd.lib"
LibraryDir["glslang_MachineIndependent_debug"] = "vendor/glslang/lib/Debug/MachineIndependentd.lib"
LibraryDir["glslang_SPIRV_debug"] = "vendor/glslang/lib/Debug/SPIRVd.lib"
LibraryDir["glslang_OGLCompiler_debug"] = "vendor/glslang/lib/Debug/OGLCompilerd.lib"
LibraryDir["glslang_OSDependent_debug"] = "vendor/glslang/lib/Debug/OSDependentd.lib"
LibraryDir["glslang_GenericCodeGen_debug"] = "vendor/glslang/lib/Debug/GenericCodeGend.lib"

LibraryDir["SPIRV_Tools_debug"] = "vendor/SPIRV-Tools/lib/Debug/SPIRV-Tools.lib"
LibraryDir["SPIRV_Tools_opt_debug"] = "vendor/SPIRV-Tools/lib/Debug/SPIRV-Tools-opt.lib"

LibraryDir["PhysX_debug"] = "vendor/PhysX/lib/Debug/PhysX_64.lib"
LibraryDir["PhysXCharacterKinematic_debug"] = "vendor/PhysX/lib/Debug/PhysXCharacterKinematic_static_64.lib"
LibraryDir["PhysXCommon_debug"] = "vendor/PhysX/lib/Debug/PhysXCommon_64.lib"
LibraryDir["PhysXCooking_debug"] = "vendor/PhysX/lib/Debug/PhysXCooking_64.lib"
LibraryDir["PhysXExtensions_debug"] = "vendor/PhysX/lib/Debug/PhysXExtensions_static_64.lib"
LibraryDir["PhysXFoundation_debug"] = "vendor/PhysX/lib/Debug/PhysXFoundation_64.lib"
LibraryDir["PhysXPvd_debug"] = "vendor/PhysX/lib/Debug/PhysXPvdSDK_static_64.lib"

-- Release libs
LibraryDir["shaderc_release"] = "vendor/shaderc/lib/Release/shaderc.lib"
LibraryDir["shaderc_util_release"] = "vendor/shaderc/lib/Release/shaderc_util.lib"

LibraryDir["glslang_release"] = "vendor/glslang/lib/Release/glslang.lib"
LibraryDir["glslang_MachineIndependent_release"] = "vendor/glslang/lib/Release/MachineIndependent.lib"
LibraryDir["glslang_SPIRV_release"] = "vendor/glslang/lib/Release/SPIRV.lib"
LibraryDir["glslang_OGLCompiler_release"] = "vendor/glslang/lib/Release/OGLCompiler.lib"
LibraryDir["glslang_OSDependent_release"] = "vendor/glslang/lib/Release/OSDependent.lib"
LibraryDir["glslang_GenericCodeGen_release"] = "vendor/glslang/lib/Release/GenericCodeGen.lib"

LibraryDir["SPIRV_Tools_release"] = "vendor/SPIRV-Tools/lib/Release/SPIRV-Tools.lib"
LibraryDir["SPIRV_Tools_opt_release"] = "vendor/SPIRV-Tools/lib/Release/SPIRV-Tools-opt.lib"

LibraryDir["PhysX_release"] = "vendor/PhysX/lib/Release/PhysX_64.lib"
LibraryDir["PhysXCharacterKinematic_release"] = "vendor/PhysX/lib/Release/PhysXCharacterKinematic_static_64.lib"
LibraryDir["PhysXCommon_release"] = "vendor/PhysX/lib/Release/PhysXCommon_64.lib"
LibraryDir["PhysXCooking_release"] = "vendor/PhysX/lib/Release/PhysXCooking_64.lib"
LibraryDir["PhysXExtensions_release"] = "vendor/PhysX/lib/Release/PhysXExtensions_static_64.lib"
LibraryDir["PhysXFoundation_release"] = "vendor/PhysX/lib/Release/PhysXFoundation_64.lib"
LibraryDir["PhysXPvd_release"] = "vendor/PhysX/lib/Release/PhysXPvdSDK_static_64.lib"

group "Dependencies"
include "Neon/vendor/GLFW"
include "Neon/vendor/ImGui"
include "Neon/vendor/SPIRV-Cross"
group ""

group "Core"
project "Neon"
	location "Neon"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "neopch.h"
	pchsource "Neon/src/neopch.cpp"

	files 
	{ 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.c", 
		"%{prj.name}/src/**.hpp", 
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/src/Neon",
		"%{prj.name}/vendor",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Vulkan}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.SPIRV_Cross}",
		"%{prj.name}/vendor/assimp/include",
		"%{prj.name}/vendor/stb/include",
		"%{prj.name}/vendor/softfloat/include",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.PhysX_internal}"
	}
	
	links 
	{ 
		"GLFW",
		"ImGui",
		"SPIRV-Cross",

		"%{LibraryDir.Vulkan}"
	}
	
	defines
	{
		"PX_PHYSX_STATIC_LIB"
	}

	filter "system:windows"
		systemversion "latest"
		
		defines 
		{ 
			"NEO_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines "NEO_DEBUG"
		symbols "On"
		
		links
		{
			"%{LibraryDir.shaderc_debug}",
			"%{LibraryDir.shaderc_util_debug}",
			"%{LibraryDir.glslang_debug}",
			"%{LibraryDir.glslang_MachineIndependent_debug}",
			"%{LibraryDir.glslang_SPIRV_debug}",
			"%{LibraryDir.glslang_OGLCompiler_debug}",
			"%{LibraryDir.glslang_OSDependent_debug}",
			"%{LibraryDir.glslang_GenericCodeGen_debug}",
			"%{LibraryDir.SPIRV_Tools_debug}",
			"%{LibraryDir.SPIRV_Tools_opt_debug}",
			
			"%{LibraryDir.PhysX_debug}",
			"%{LibraryDir.PhysXCharacterKinematic_debug}",
			"%{LibraryDir.PhysXCommon_debug}",
			"%{LibraryDir.PhysXCooking_debug}",
			"%{LibraryDir.PhysXExtensions_debug}",
			"%{LibraryDir.PhysXFoundation_debug}",
			"%{LibraryDir.PhysXPvd_debug}"
		}
				
	filter "configurations:Release"
		defines 
		{
			"NEO_RELEASE",
			"NDEBUG" -- PhysX Requires This
		}
		
		links
		{
			"%{LibraryDir.shaderc_release}",
			"%{LibraryDir.shaderc_util_release}",
			"%{LibraryDir.glslang_release}",
			"%{LibraryDir.glslang_MachineIndependent_release}",
			"%{LibraryDir.glslang_SPIRV_release}",
			"%{LibraryDir.glslang_OGLCompiler_release}",
			"%{LibraryDir.glslang_OSDependent_release}",
			"%{LibraryDir.glslang_GenericCodeGen_release}",
			"%{LibraryDir.SPIRV_Tools_release}",
			"%{LibraryDir.SPIRV_Tools_opt_release}",
			
			"%{LibraryDir.PhysX_release}",
			"%{LibraryDir.PhysXCharacterKinematic_release}",
			"%{LibraryDir.PhysXCommon_release}",
			"%{LibraryDir.PhysXCooking_release}",
			"%{LibraryDir.PhysXExtensions_release}",
			"%{LibraryDir.PhysXFoundation_release}",
			"%{LibraryDir.PhysXPvd_release}"
		}
		
		optimize "On"

group "Tools"
project "NeonEditor"
	location "NeonEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	links 
	{ 
		"Neon"
	}
	
	files 
	{
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.c", 
		"%{prj.name}/src/**.hpp", 
		"%{prj.name}/src/**.cpp" 
	}
	
	includedirs 
	{
		"%{prj.name}/src",
		"Neon/src",
		"Neon/vendor",
		"%{IncludeDir.entt}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.shaderc}"
	}
	
	filter "system:windows"
		systemversion "latest"
				
		defines 
		{ 
			"NEO_PLATFORM_WINDOWS"
		}
	
	filter "configurations:Debug"
		defines "NEO_DEBUG"
		symbols "on"

		links
		{
			"Neon/vendor/assimp/bin/Debug/assimp-vc141-mtd.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../Neon/vendor/assimp/bin/Debug/assimp-vc141-mtd.dll" "%{cfg.targetdir}"',
			'{COPY} "../Neon/vendor/PhysX/lib/Debug/PhysX_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Neon/vendor/PhysX/lib/Debug/PhysXFoundation_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Neon/vendor/PhysX/lib/Debug/PhysXCooking_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Neon/vendor/PhysX/lib/Debug/PhysXCommon_64.dll" "%{cfg.targetdir}"'
		}
				
	filter "configurations:Release"
		defines "NEO_RELEASE"
		optimize "on"

		links
		{
			"Neon/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
		}

		postbuildcommands 
		{
			'{COPY} "../Neon/vendor/assimp/bin/Release/assimp-vc141-mt.dll" "%{cfg.targetdir}"',
			'{COPY} "../Neon/vendor/PhysX/lib/Release/PhysX_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Neon/vendor/PhysX/lib/Release/PhysXFoundation_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Neon/vendor/PhysX/lib/Release/PhysXCooking_64.dll" "%{cfg.targetdir}"',
			'{COPY} "../Neon/vendor/PhysX/lib/Release/PhysXCommon_64.dll" "%{cfg.targetdir}"'
		}
group ""