# Hello Vulkan

This is a simple Hello Triangle application to learn the basics of the Vulkan API from the tutorial by
[Alexander Overvoorde's Vulkan tutorial](https://vulkan-tutorial.com/).

## Building

(Assuming you have the environment variable `VULKAN_SDK` pointing to the SDK)

Use the CMake to build the application (currently only works )

```shell
mkdir build
cd build
cmake ..
make run
```

Once built, the program can be run from within the build directory using the `make run` command (even for changes)

For compiling the shaders using `./../src/shaders/glsl/compile-glsl.sh` from within the build directory (if ou get permission erros use `chmod +x ../src/shaders/glsl/compile-glsl.sh` before running the shell script)

## Thanks to
*  Alexander Overvoorde for his awesome Vulkan tutorial.
