IncludeDir = {}
IncludeDir["glfw"]          = "vendor/glfw/include"
IncludeDir["glm"]           = "vendor/glm"
IncludeDir["ktx"]           = "vendor/ktx/include"
IncludeDir["ImGui"]         = "vendor/imgui"
IncludeDir["ImGuizmo"]      = "vendor/ImGuizmo"
IncludeDir["SPIRVReflect"]  = "vendor/SPIRVReflect"
IncludeDir["stb"]           = "vendor/stb"
IncludeDir["tinyobj"]       = "vendor/tinyobj"
IncludeDir["tinygltf"]      = "vendor/tinygltf"
IncludeDir["tracy"]         = "vendor/TracyProfiler"
IncludeDir["optick"]        = "vendor/optick/src"
IncludeDir["vendor"]        = "vendor"
VulkanSDK = os.getenv("VULKAN_SDK")
print(VulkanSDK)
root_dir = os.getcwd()


workspace ( "Vulf" )
    location "build"
    platforms { "x64" }

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
        "vendor/TracyProfiler/Tracy.hpp",
        "vendor/TracyProfiler/TracyClient.cpp",
        "vendor/SPIRVReflect/spirv_reflect.h",
        "vendor/SPIRVReflect/spirv_reflect.c",
        "vendor/SPIRVReflect/common/output_stream.h",
        "vendor/SPIRVReflect/common/output_stream.cpp",
        "vendor/optick/**",
        "vendor/optick/src/optick_gpu.d3d12.cpp",
        "vendor/optick/src/optick_gpu.vulkan.cpp",
        "vendor/ktx/lib/*.cpp",
        "vendor/ktx/lib/*.hpp",
        "vendor/ktx/lib/*.c",
        "vendor/ktx/lib/*.h",
        "vendor/ktx/other_include/**",
        "src/shaders/glsl/*.vert",
        "src/shaders/glsl/*.frag",
        -- Examples
        "examples/cpp/**.cpp"
	}

    includedirs
	{
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.ktx}",
                "%{IncludeDir.ktx}/../other_include",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.SPIRVReflect}",
		"%{IncludeDir.stb}",
        "%{IncludeDir.tinyobj}",
        "%{IncludeDir.tinygltf}",
        "%{IncludeDir.tracy}",
        "%{IncludeDir.optick}",
        "%{IncludeDir.VULKAN_SDK}",
        "%{IncludeDir.vendor}",
        "./",
        "./src/",
        "./src/utils",
        "./src/Vulkan"
	}

	sysincludedirs
	{
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.ktx}",
                        "%{IncludeDir.ktx}/../other_include",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.SPIRVReflect}",
		"%{IncludeDir.stb}",
        "%{IncludeDir.tinyobj}",
        "%{IncludeDir.tinygltf}",
        "%{IncludeDir.tracy}",
        "%{IncludeDir.optick}",
        "%{IncludeDir.VULKAN_SDK}",
        "%{IncludeDir.vendor}",
        "./",
        "./src/",
        "./src/utils",
        "./src/Vulkan"
	}

    links
    {
        "glfw3",
        "vulkan-1"
    }

    filter {"files:**.vert or **.frag"}
        removeflags "ExcludeFromBuild"
        buildcommands ' \"%{VulkanSDK}/Bin/glslc.exe\" "%{file.directory}/%{file.name}" -o "%{file.directory}/../spir-v/%{file.basename }.spv" '
        buildoutputs "%{file.directory}/../spir-v/%{file.basename }.spv"

    includedirs { VulkanSDK .. "/include" }

	filter "platforms:x64"
		system "Windows"
		architecture "x64"
        cppdialect "C++17"
        systemversion "latest"

        defines
        {
            "SRC_DIR=\"%{root_dir}\"",
            "SHADER_BINARY_DIR=\"%{root_dir}/src/shaders/spir-v\"",
            "ASSETS_DIR=\"%{root_dir}/../Vulkan/data\""
        }

         links
        {
            "Dbghelp",
            "d3d11",
            "D3DCompiler",
            "dxgi"
        }

        includedirs { VulkanSDK .. "/include" }

        libdirs { VulkanSDK .. "/Lib" }
    filter{}

    filter "configurations:Debug"
        libdirs { "vendor/glfw/libs/%{cfg.buildcfg}" }
        symbols "On"
        runtime "Debug"
        optimize "Off"

    filter "configurations:Release"
        libdirs { "vendor/glfw/libs/%{cfg.buildcfg}" }
        optimize "Full"
        symbols "Off"
