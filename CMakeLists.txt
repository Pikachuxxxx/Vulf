cmake_minimum_required(VERSION 3.20)

# set the project name and version
project(Vulf)

# specify the C++ standard and making it mandatory
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "_CMakePredefinedTargets")
# Set the startup project - HelloTriangle
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT HelloTriangle)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_CXX_FLAGS_DEBUG "/ZI")
  set(CMAKE_SHARED_LINKER_FLAGS "/SAFESEH:NO")
  set(CMAKE_EXE_LINKER_FLAGS "/SAFESEH:NO")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL:YES" )
endif()

#===============================================================================
# TODO: Download the necesary assets here
#===============================================================================
# COMPILE SHADERS
# TODO: Design decision on whether shaders are distributed with Vulf library or with examples, this will also influence how theya re grouped into folders and compiled
# Set the path to glslc compiler based on the OS
if(APPLE)
    set(glslc_executable $ENV{VULKAN_SDK}/macOS/bin/glslc)
endif()

if(WIN32)
    set(glslc_executable $ENV{VULKAN_SDK}/Bin/glslc)
endif()

# Set the directory for loading the compiled spir-v shaders and source direcotry for glsl shaders
set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/glsl)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/spir-v)

# Add the glsl shader files
file(GLOB_RECURSE SHADERS
  ${SHADER_SOURCE_DIR}/*.vert
  ${SHADER_SOURCE_DIR}/*.frag
  ${SHADER_SOURCE_DIR}/*.comp
  ${SHADER_SOURCE_DIR}/*.geom
  ${SHADER_SOURCE_DIR}/*.tesc
  ${SHADER_SOURCE_DIR}/*.tese
  ${SHADER_SOURCE_DIR}/*.mesh
  ${SHADER_SOURCE_DIR}/*.task
  ${SHADER_SOURCE_DIR}/*.rgen
  ${SHADER_SOURCE_DIR}/*.rchit
  ${SHADER_SOURCE_DIR}/*.rmiss)

# Creating the spir-v source directory
  add_custom_command(
  COMMAND
    ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
  OUTPUT ${SHADER_BINARY_DIR}
  COMMENT "Creating ${SHADER_BINARY_DIR}"
)

source_group(shaders "Shaders" FILES ${SHADERS})

# Custom rule for compiling the shader files
foreach(source IN LISTS SHADERS)
  get_filename_component(FILENAME ${source} NAME)
  add_custom_command(
    COMMAND
      ${glslc_executable}
      #      -MD -MF ${SHADER_BINARY_DIR}/${FILENAME}.d
      -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
      ${source}
    OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
    DEPENDS ${source} ${SHADER_BINARY_DIR}
    COMMENT "Compiling ${FILENAME}"
  )
  list(APPEND SPV_SHADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()
# add dependency for shader files
add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS} SOURCES ${SHADERS})
#===============================================================================
# INCLUDE AND LINK DIRECTORIES

#   MACOS
if (APPLE)
# set the include directories for the vendor dependencies
# TODO: Make this cross-platform for now it'll be MacOS specific for the Vulkan headers
include_directories(
    # GLFW headers
    "${CMAKE_SOURCE_DIR}/vendor/glfw/include"
    # GLM headers
    "${CMAKE_SOURCE_DIR}/vendor/glm"
    # ImGui Headers
    "${CMAKE_SOURCE_DIR}/vendor/ImGui"
    # The entire vendor folder
    "${CMAKE_SOURCE_DIR}/vendor"
    "${CMAKE_SOURCE_DIR}/vendor/stb"
    # Tracy headers
    "${CMAKE_SOURCE_DIR}/vendor/TracyProfiler"
    #SPIRV-Reflect
    "${CMAKE_SOURCE_DIR}/vendor/SPIRVReflect"
    # Optick
    #"${CMAKE_SOURCE_DIR}/vendor/optick/src"
    # Vulf
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/Vulkan"
    # KTX
    "${CMAKE_SOURCE_DIR}/vendor/ktx/include"
    "${CMAKE_SOURCE_DIR}/vendor/tinygltf"
    "${CMAKE_SOURCE_DIR}/vendor/ktx/other_include/"

    # Vulkan MacOS headers
    $ENV{VULKAN_SDK}/macOS/include
)

link_directories(
    # glfw library path
    "${CMAKE_SOURCE_DIR}/vendor/glfw/build/src"
    # MoltenVk libraries
    $ENV{VULKAN_SDK}/macOS/lib
)
endif()

#   WINDOWS
if(WIN32)
include_directories(
    # GLFW headers
    "${CMAKE_SOURCE_DIR}/vendor/glfw/include"
    # GLM headers
    "${CMAKE_SOURCE_DIR}/vendor/glm"
    # ImGui Headers
    "${CMAKE_SOURCE_DIR}/vendor/ImGui"
    # The entire vendor folder
    "${CMAKE_SOURCE_DIR}/vendor"
    "${CMAKE_SOURCE_DIR}/vendor/stb"
    # Tracy headers
    "${CMAKE_SOURCE_DIR}/vendor/TracyProfiler"
    #SPIRV-Reflect
    "${CMAKE_SOURCE_DIR}/vendor/SPIRVReflect"
    # Optick
    #"${CMAKE_SOURCE_DIR}/vendor/optick/src"
    # Vulf
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/Vulkan"
    # KTX
    "${CMAKE_SOURCE_DIR}/vendor/ktx/include"
    "${CMAKE_SOURCE_DIR}/vendor/tinygltf"
    "${CMAKE_SOURCE_DIR}/vendor/ktx/other_include/"

    # Platform specific vulkan headers
    $ENV{VULKAN_SDK}/include
)

link_directories(
    # GLFW library path
    "${CMAKE_SOURCE_DIR}/vendor/glfw/libs"
    # Vulkan libraries
    $ENV{VULKAN_SDK}/Lib
)
endif()
#===============================================================================
# Add source files
file(GLOB_RECURSE SOURCE_FILES
	${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)


# Add source header files
file(GLOB_RECURSE HEADER_FILES
	${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.hpp
)

# vendor source files that need to compiled to build into Vulf
file(GLOB_RECURSE VENDOR_SOURCE_FILES
	${CMAKE_SOURCE_DIR}/vendor/imgui/*.c
    ${CMAKE_SOURCE_DIR}/vendor/imgui/*.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/*.c
    ${CMAKE_SOURCE_DIR}/vendor/imgui/*.cpp
    ${CMAKE_SOURCE_DIR}/vendor/ImGuizmo/ImCurveEdit.cpp
    ${CMAKE_SOURCE_DIR}/vendor/ImGuizmo/ImGradient.cpp
    ${CMAKE_SOURCE_DIR}/vendor/ImGuizmo/ImGuizmo.cpp
    ${CMAKE_SOURCE_DIR}/vendor/ImGuizmo/ImSequencer.cpp
    ${CMAKE_SOURCE_DIR}/vendor/TracyProfiler/TracyClient.cpp
    #${CMAKE_SOURCE_DIR}/vendor/optick/*.h
    #${CMAKE_SOURCE_DIR}/vendor/optick/*.cpp
    ${CMAKE_SOURCE_DIR}/vendor/SPIRVReflect/spirv_reflect.c
    ${CMAKE_SOURCE_DIR}/vendor/SPIRVReflect/common/output_stream.cpp
    ${CMAKE_SOURCE_DIR}/vendor/ktx/*.c
    ${CMAKE_SOURCE_DIR}/vendor/ktx/other_include/KHR/*.h
    ${CMAKE_SOURCE_DIR}/vendor/ktx/*.h
)

# remove unnecessary files
list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/application.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/application.h")
#list(REMOVE_ITEM SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/application.h")
#===============================================================================
#source_group("src" FILES ${SOURCE_FILES})
# file(GLOB_RECURSE VENDOR_HEADER_FILES
# 	${CMAKE_SOURCE_DIR}/vendor/imgui/*.h
#     ${CMAKE_SOURCE_DIR}/vendor/imgui/*.hpp
# )
# source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
#      PREFIX "Source"
#      FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${VENDOR_SOURCE_FILES})
#===============================================================================
# Compile the source files into a static library
add_library(Vulf STATIC ${HEADER_FILES} ${SOURCE_FILES} ${VENDOR_SOURCE_FILES})
add_dependencies(Vulf shaders)
#===============================================================================
# Global pre-processor defines
add_compile_definitions(TRACY_ENABLE)
add_compile_definitions(_DEBUG)
add_compile_definitions("SHADER_BINARY_DIR=\"${SHADER_BINARY_DIR}\"")
add_compile_definitions("SRC_DIR=\"${CMAKE_SOURCE_DIR}\"")
add_compile_definitions("ASSETS_DIR=\"${CMAKE_SOURCE_DIR}/data\"")
#===============================================================================
# Link with necessary external libraries
if (APPLE)
target_link_libraries(Vulf glfw3 vulkan.1 "-framework CoreFoundation -framework CoreAudio -framework OpenAL -framework Cocoa -framework IOKit -framework CoreGraphics")
endif (APPLE)
if(WIN32)
target_link_libraries(Vulf glfw3 vulkan-1)
endif()
#===============================================================================
# now build the examples
# HACK: Just try building hello triangle then make it dynamics to build all exaples with source dependency
file(GLOB_RECURSE EXAMPLE_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/examples/cpp/*.cpp
)
foreach(source IN LISTS EXAMPLE_SOURCE_FILES)
    get_filename_component(FILENAME ${source} NAME_WE)
    message("Examples being built : ${FILENAME}")
    # TODO: Improve source group folder hierarchy for examples
    #    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source})
    # Build the examples as executables (run them using make examplefilename.run command)
    ## add the source files in each directory of the example instead of by *.cpp + add a text file or something to load shaders from common pool or the folder itseld
    add_executable(${FILENAME} ${source})
    target_link_libraries(${FILENAME} Vulf)
    add_dependencies(${FILENAME} Vulf)
    if(APPLE) # Used only with makefiles cause VS can handle this
        # Run the binary built using 'make run' command
        add_custom_target("${FILENAME}.run"
            COMMAND ${FILENAME}
            DEPENDS ${FILENAME} Vulf
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} # here we set the executable direcotry to the CMake home dir
        )
    endif(APPLE)
    # Set the output directory for the examples build separately
    set_target_properties(${FILENAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/examples)
    set_target_properties(${FILENAME} PROPERTIES FOLDER "examples")

endforeach()

#===============================================================================
