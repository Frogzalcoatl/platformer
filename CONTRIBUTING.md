# Contributing
## Prerequisites
**Windows:**
* Download [Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2026)
* Select and install the **"Desktop development with C++"** workload.

**MacOS:**
* Install Xcode Command Line Tools:
```bash
xcode-select --install
```

**Linux:**
* Install the build tools for your respective distribution listed [here](https://wiki.libsdl.org/SDL3/README-linux).
* If there are additional dependencies I'm unaware of, they will likely be logged as an error or warning when you try to build the project.

## Building the Project

This project is set up to have a pretty straightforward build process using CMake and Vcpkg. If you've never installed vcpkg, don't worry. It will automatically clone into the project directory on build. However, if you have vcpkg installed globally with the VCPKG_ROOT environment variable set up, the global installation is used for efficiency.

This project works fine in whatever ide you want, given you use a preset listed in [CMakePresets.json](https://github.com/Frogzalcoatl/platformer/blob/main/CMakePresets.json). I use vscode with the [ms-vscode.cpptools-extension-pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack). I select **"windows-clang-vs"**, **"Clang VS Debug"**, then click **"build"** in the status bar.

If you do not want to use vscode, the equivalent commands are:

```
cmake --preset windows-clang-vs
cmake --build --preset clang-vs-debug
```

First command uses a [CMakePresets.json](https://github.com/Frogzalcoatl/platformer/blob/main/CMakePresets.json) option in **"configurePresets"**. Second uses a matching option in **"buildPresets".** Each configure preset has a "debug", "release", and "release with debug info" build preset.
