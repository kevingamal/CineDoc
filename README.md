# CineDoc

A multiplatform app for managing cinema production projects

## How this works

This App simplifies the processes of managing film production, starting from the script, allowing you to generate production sheets for each scene to be filmed.

## Requirements

This works on Windows, Mac, and Linux. You'll need `cmake` and a C++ compiler (tested on `clang`, `gcc`, and MSVC).

Linux builds require the GTK3 library and headers installed in the system.

## Building

To build the project, use:

```bash
cmake -S. -Bbuild
cmake --build build
```

This will create a directory named `build` and create all build artifacts there. The main executable can be found in the `build/subprojects/Build/wx_cmake_template_core` folder.

## Using as a Template (Linux/Mac)

Use the provided `copy_to_project.sh` script to create another project from the template.

```bash
./copy_to_project.sh directory project_name
```

This will create a copy of the template's directory structure in `directory`, renaming `wx_cmake_template` to the provided `project_name`.

## Notes

For details, see the [blog post](https://www.lukesdevtutorials.com/post/wxwidgets-cmake/) and the [video](https://www.youtube.com/watch?v=MfuBS9n5_aY) tutorial showcasing the installation on Linux, Windows, and Mac OS X. 

For more details, see the [blog post](https://www.lukesdevtutorials.com/post/wxwidgets-cmake/) and the [video](https://www.youtube.com/watch?v=MfuBS9n5_aY) tutorial showcasing the installation on Linux, Windows, and Mac OS X.
Check out his blog for more! [www.lukesdevtutorials.com](https://www.lukesdevtutorials.com)



