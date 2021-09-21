cmake_minimum_required(VERSION 3.18)

# set the project name and version
project(Vulf)

# specify the C++ standard and making it mandatory
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_SUPPRESS_REGENERATION true)

#===============================================================================
# COMPILE SHADERS

if(APPLE)
    set(glslc_executable $ENV{VULKAN_SDK}/macOS/bin/glslc)
endif()

if(WIN32)
    set(glslc_executable $ENV{VULKAN_SDK}/Bin/glslc)
endif()

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/glsl)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/spir-v)

file(GLOB SHADERS
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

foreach(source IN LISTS SHADERS)
  get_filename_component(FILENAME ${source} NAME_WE)
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

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS} SOURCES ${SHADERS})
#===============================================================================


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
    # Vulkan MacOS headers
    $ENV{VULKAN_SDK}/macOS/include
    # Tracy headers
    "${CMAKE_SOURCE_DIR}/vendor/tracy"
    #SPIRV-Reflect
    "${CMAKE_SOURCE_DIR}/vendor/SPIRV-Reflect"
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
    "${CMAKE_SOURCE_DIR}/vendor"
    # GLFW headers
    "${CMAKE_SOURCE_DIR}/vendor/glfw/include"
    # GLM headers
    "${CMAKE_SOURCE_DIR}/vendor/glm"
    # Vulkan MacOS headers
    "$ENV{VULKAN_SDK}/include"
    # ImGui Headers
    "${CMAKE_SOURCE_DIR}/vendor/ImGui"
    # Tracy headers
    "${CMAKE_SOURCE_DIR}/vendor/tracy"
    #SPIRV-Reflect
    "${CMAKE_SOURCE_DIR}/vendor/SPIRV-Reflect"
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

file(GLOB_RECURSE VENDOR_SOURCE_FILES
	${CMAKE_SOURCE_DIR}/vendor/imgui/*.c
    ${CMAKE_SOURCE_DIR}/vendor/imgui/*.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/*.c
    ${CMAKE_SOURCE_DIR}/vendor/imgui/*.cpp
    ${CMAKE_SOURCE_DIR}/vendor/ImGuizmo/ImCurveEdit.cpp
    ${CMAKE_SOURCE_DIR}/vendor/ImGuizmo/ImGradient.cpp
    ${CMAKE_SOURCE_DIR}/vendor/ImGuizmo/ImGuizmo.cpp
    ${CMAKE_SOURCE_DIR}/vendor/ImGuizmo/ImSequencer.cpp
    ${CMAKE_SOURCE_DIR}/vendor/tracy/TracyClient.cpp
    ${CMAKE_SOURCE_DIR}/vendor/SPIRV-Reflect/spirv_reflect.c
    ${CMAKE_SOURCE_DIR}/vendor/SPIRV-Reflect/common/output_stream.cpp
)

# Add source header files
file(GLOB_RECURSE HEADER_FILES
	${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.hpp
    # ${CMAKE_SOURCE_DIR}/vendor/tracy/Tracy.hpp
)
#===============================================================================
# file(GLOB_RECURSE VENDOR_HEADER_FILES
# 	${CMAKE_SOURCE_DIR}/vendor/imgui/*.h
#     ${CMAKE_SOURCE_DIR}/vendor/imgui/*.hpp
# )
# source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
#      PREFIX "Source"
#      FILES ${SOURCE_FILES})
#===============================================================================
# Compile the source files for the executable
source_group("src" FILES ${SOURCE_FILES})
add_executable(Vulf ${HEADER_FILES} ${SOURCE_FILES} ${VENDOR_SOURCE_FILES})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "_CMakePredefinedTargets")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Vulf)
#===============================================================================

#===============================================================================
# LINK EXECUTABLE WITH LIBRARIES
if (APPLE)
target_link_libraries(Vulf glfw3 vulkan.1 "-framework CoreFoundation -framework CoreAudio -framework OpenAL -framework Cocoa -framework IOKit -framework CoreGraphics")
endif (APPLE)
if(WIN32)
target_link_libraries(Vulf glfw3 vulkan-1)
endif()
#===============================================================================
# Global pre-processor defines
add_compile_definitions(TRACY_ENABLE)
target_compile_definitions(Vulf PRIVATE "SHADER_BINARY_DIR=\"${SHADER_BINARY_DIR}\"")
target_compile_definitions(Vulf PRIVATE "SRC_DIR=\"${CMAKE_SOURCE_DIR}\"")
add_dependencies(Vulf shaders)

#===============================================================================
# Run the binary built using 'make run' command
add_custom_target(run
    COMMAND Vulf
    DEPENDS Vulf
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} # here we set the executable direcotry to the CMake home dir
)
#===============================================================================
