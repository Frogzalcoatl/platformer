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

This project is set up to have a pretty straightforward build process using CMake and Vcpkg. If you've never installed vcpkg, don't worry. It will automatically clone into the project directory on build. However, if you have vcpkg installed globally with the VCPKG_ROOT environment variable set up, this project uses that for efficiency.

I use vscode with the [ms-vscode.cpptools-extension-pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack) extension, however this project probably works fine using whatever environment you want, given you use a configure preset listed in [CMakePresets.json](https://github.com/Frogzalcoatl/platformer/blob/main/CMakePresets.json).

If you're not using vscode, I believe a preset can be selected by running a command like this:

```
cmake --build --preset windows-msvc
```

In vscode, I click **"windows-clang-vs"** then **"build"** in the status bar, which is added by the vscode extension listed above.