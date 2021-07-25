cmake_minimum_required(VERSION 3.18)

# set the project name and version
project(HelloVulkan)

# specify the C++ standard and making it mandatory
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (APPLE)
# set the include directories for the vendor dependencies
# TODO: Make this cross-platform for now it'll be MacOS specific for the Vulkan headers
include_directories(
    # GLFW headers
    "${CMAKE_SOURCE_DIR}/vendor/glfw/include"
    # GLM headers
    "${CMAKE_SOURCE_DIR}/vendor/glm"
    # Vulkan MacOS headers
    $ENV{VULKAN_SDK}/macOS/include
)

link_directories(
    # glfw library path
    "${CMAKE_SOURCE_DIR}/vendor/glfw/build/src"
    # MoltenVk libraries
    $ENV{VULKAN_SDK}/Lib
)
endif()

include_directories(
    # GLFW headers
    "${CMAKE_SOURCE_DIR}/vendor/glfw/include"
    # GLM headers
    "${CMAKE_SOURCE_DIR}/vendor/glm"
    # Vulkan MacOS headers
    $ENV{VULKAN_SDK}/include
)

link_directories(
    # glfw library path
    "${CMAKE_SOURCE_DIR}/vendor/glfw/build/src"
    # MoltenVk libraries
    $ENV{VULKAN_SDK}Lib
)


# Checking the include directories
get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()

if(DEFINED ENV{VK_ICD_FILENAMES})
    message(STATUS "Found VK ICD filenames at")
    message(STATUS $ENV{VK_ICD_FILENAMES})
endif()

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

# Compile the source files for the executable
add_executable(HelloVulkan ${HEADER_FILES} ${SOURCE_FILES})

if (APPLE)
target_link_libraries(HelloVulkan glfw3 vulkan.1 "-framework CoreFoundation -framework CoreAudio -framework OpenAL -framework Cocoa -framework IOKit -framework CoreGraphics")
endif (APPLE)

target_link_libraries(HelloVulkan glfw3 vulkan-1)


# Run the binary built using 'make run' command
add_custom_target(run
    COMMAND HelloVulkan
    DEPENDS HelloVulkan
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} # here we set the executable direcotry to the CMake home dir
)
