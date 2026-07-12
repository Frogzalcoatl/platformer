# Contributing (Android Crosscompiling Branch)
## Prerequisites
1. Install the [Android NDK](https://developer.android.com/ndk/downloads).
2. Set the "ANDROID_NDK_HOME" environment variable to point to your NDK directory.
   - *Example (Linux/macOS)*: `export ANDROID_NDK_HOME=~/Android/Sdk/ndk/25.x.xxxxxx`
   - *Example (Windows)*: `set ANDROID_NDK_HOME=C:\Users\YourUser\AppData\Local\Android\Sdk\ndk\25.x.xxxxxx`

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

1. Build a desktop version of this project first to properly pack the assets into a .dat file.

This project is set up to have a pretty straightforward build process using CMake and Vcpkg. If you've never installed vcpkg, don't worry. It will automatically clone into the project directory on build. However, if you have vcpkg installed globally with the VCPKG_ROOT environment variable set up, your global installation is used for efficiency.

This project works fine in whatever ide you want, given you use a preset listed in [CMakePresets.json](https://github.com/Frogzalcoatl/platformer/blob/main/CMakePresets.json). I use vscode with the [ms-vscode.cpptools-extension-pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack). I select **"windows-clang-vs"**, **"Clang VS Debug"**, then click **"build"** in the status bar.

If you do not want to use vscode, the equivalent commands are:

```
cmake --preset windows-clang-vs
cmake --build --preset clang-vs-debug
```

First command uses a [CMakePresets.json](https://github.com/Frogzalcoatl/platformer/blob/main/CMakePresets.json) option in **"configurePresets"**. Second uses a matching option in **"buildPresets".** Each configure preset has a "debug", "release", and "release with debug info" build preset.

2. Switch to the android preset and build.
```
cmake --preset android-arm64
cmake --build --preset android-arm64-debug
```
