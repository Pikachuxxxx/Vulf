IncludeDir = {}
IncludeDir["glfw"]          = "vendor/glfw/include"
IncludeDir["glm"]           = "vendor/glm"
IncludeDir["ImGui"]         = "vendor/imgui"
IncludeDir["ImGuizmo"]      = "vendor/ImGuizmo"
IncludeDir["SPIRVReflect"] = "vendor/SPIRV-Reflect"
IncludeDir["stb"]           = "vendor/stb"
IncludeDir["tinyobj"]       = "vendor/tinyobj"
IncludeDir["tracy"]         = "vendor/tracy"
IncludeDir["vendor"]         = "vendor"
VulkanSDK = os.getenv("VULKAN_SDK")
print(VulkanSDK)
root_dir = os.getcwd()

workspace ( "Vulf" )
    location "build"
    architecture "x86_64"

    -- Output directory path based on build config
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    -- Binaries Output directory
    targetdir ("bin/%{outputdir}/")
    -- Intermediate files Output directory
    objdir ("bin-int/%{outputdir}/obj/")

    -- Various build configuration for the engine
    configurations
    {
        "Debug",
        "Release"
    }

    project "Vulf"
        kind "ConsoleApp"
        language "C++"

    files
	{
		"src/**.h",
        "src/**.hpp",
        "src/**.c",
		"src/**.cpp",
        "vendor/ImGui/*.h",
        "vendor/ImGui/*.cpp",
        "vendor/ImGui/backends/*.h",
        "vendor/ImGui/backends/*.cpp",
        "vendor/ImGuizmo/ImCurveEdit.h",
        "vendor/ImGuizmo/ImCurveEdit.cpp",
        "vendor/ImGuizmo/ImGradient.h",
        "vendor/ImGuizmo/ImGradient.cpp",
        "vendor/ImGuizmo/ImGuizmo.h",
        "vendor/ImGuizmo/ImGuizmo.cpp",
        "vendor/ImGuizmo/ImSequencer.h",
        "vendor/ImGuizmo/ImSequencer.cpp",
        "vendor/tracy/Tracy.hpp",
        "vendor/tracy/TracyClient.cpp",
        "vendor/SPIRV-Reflect/spirv_reflect.h",
        "vendor/SPIRV-Reflect/spirv_reflect.c",
        "vendor/SPIRV-Reflect/common/output_stream.h",
        "vendor/SPIRV-Reflect/common/output_stream.cpp",
        "src/shaders/glsl/*.vert",
        "src/shaders/glsl/*.frag",
	}

	sysincludedirs
	{
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.SPIRVReflect}",
		"%{IncludeDir.stb}",
        "%{IncludeDir.tinyobj}",
        "%{IncludeDir.tracy}",
        "%{IncludeDir.VULKAN_SDK}",
        "%{IncludeDir.vendor}"
	}

    links
    {
        "glfw3",
        "vulkan-1"
    }

    filter "files: src/shaders/glsl/*.vert"
        removeflags("ExcludeFromBuild")
        buildcommands ' %{VulkanSDK}Bin/glslc.exe" -o "%{file.directory}/../spir-v/%{file.name}.spv" "%{file.relpath}"'
        buildoutputs "%{file.directory}/../spir-v/%{file.name}.spv"


    includedirs { VulkanSDK .. "/include" }

    filter "system:windows"
        cppdialect "C++17"
        systemversion "latest"

        defines
        {
            "SRC_DIR=\"%{root_dir}\"",
            "SHADER_BINARY_DIR=\"%{root_dir}/src/shaders/spir-v\""
        }

        includedirs { VulkanSDK .. "/include" }

        libdirs { VulkanSDK .. "/Lib" }

    filter "configurations:Debug"
        libdirs { "vendor/glfw/libs/%{cfg.buildcfg}" }
        symbols "On"
        runtime "Debug"
        optimize "Off"

    filter "configurations:Release"
        libdirs { "vendor/glfw/libs/%{cfg.buildcfg}" }
        optimize "Full"
        symbols "Off"
