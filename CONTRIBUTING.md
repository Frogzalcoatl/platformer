# Contributing

If you ever update the app version, make sure to update both `vcpkg.json` and `android-project/app/build.gradle` (versionName).

## Prerequisites

### 1. Clang Format (Suggested if not using VS Code)

* I use LLVM's clang-format to format C++ files on save. There are a few different download sources for this. To install clang-format without any of LLVM's other tools you can use Node.js or python.

**Node.js:**

```
npm install -g clang-format
```

**Python:**

```
pip install clang-format
```

**Full LLVM Installation:**

* If you don't care about installing all of LLVM's tools, then download [here](https://github.com/llvm/llvm-project/releases). Make sure you select "Add LLVM to the system PATH" during installation.

### 2. Build Tools
**Windows:**

* Download [Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2026)

* Select and install the **"Desktop development with C++"** workload.

**MacOS:**

1. Install Xcode Command Line Tools:
```bash
xcode-select --install
```

2. Install Homebrew (if you don't have it already):
```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

3. Install `cmake` and `ninja`:
```
brew install cmake ninja
```

**Linux:**

* Install the build tools for your respective distribution listed [here](https://wiki.libsdl.org/SDL3/README-linux).
* If there are additional dependencies I'm unaware of, they will likely be logged as an error or warning when you try to build the project.

## Building the Project
I tried my best to set this up as straightforwardly as possible. I use CMake and Vcpkg. If you've never installed vcpkg, don't worry. It will automatically clone into the project directory on build. However, if you have vcpkg installed globally with the VCPKG_ROOT environment variable set up, your global installation is used for efficiency.

This project works fine in whatever IDE you want, given you use a preset listed in [CMakePresets.json](https://github.com/Frogzalcoatl/platformer/blob/main/CMakePresets.json). I use vscode with the [ms-vscode.cpptools-extension-pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack). I select `Windows (MSVC 2026)`, `MSVC VS 2026 Debug`, then click `Build` in the status bar.

If you do not want to use vscode, the equivalent commands are:
```
cmake --preset windows-msvc-2026
cmake --build --preset vs2026-msvc-debug
```

First command uses a [CMakePresets.json](https://github.com/Frogzalcoatl/platformer/blob/main/CMakePresets.json) option in `configurePresets`. Second uses a matching option in `buildPresets`. Each configure preset has a `debug`, `release`, and `relwithdebinfo` build preset.

# Android Crosscompiling
## Prerequisites
### 1. Android SDK

* Download [here](https://developer.android.com/studio#command-line-tools-only) and extract. I personally extracted to my C: drive for simplicity. Open a terminal in `android-sdk/cmdline-tools/bin` and run `sdkmanager.bat` (or `sdkmanager` on linux/macos) with the following arguments:

```
./sdkmanager.bat "platform-tools" "build-tools;34.0.0" "platforms;android-28"
```

It also won't work unless you accept the licenses after:

```
./sdkmanager.bat --licenses
```

Define the `ANDROID_HOME` environment variable pointing to your SDK root.

- Example (Windows): `set ANDROID_HOME=C:\android-sdk`
- Example (Linux/MacOS): `export ANDROID_HOME=~/Android/Sdk`

### 2. Android NDK (r29 suggested)

* Download [here](https://developer.android.com/ndk/downloads) and extract. No extra setup needed for the ndk. Define the `ANDROID_NDK_HOME` environment variable to point to the extracted android-ndk-r29 folder.
  - Example (Windows): `set ANDROID_NDK_HOME=C:\android-ndk-r29`
  - Example (Linux/MacOS): `export ANDROID_NDK_HOME=~/Android/Sdk/ndk/29.x.xxxxxx`

### 3. Java Development Kit (JDK 17+)

* Install [here](https://adoptium.net/temurin/releases/?version=17). Required for gradle to build the apk file.
  - During the installation you must click the red X dropdown next to "Set JAVA_HOME variable" and change it to "Will be installed on local hard drive". If you forget you can also manually set the environment variable to the jdk directory after installing.

## 4. Enable USB Debugging (To debug using your android phone)

* [Enable developer options](https://developer.android.com/studio/debug/dev-options) if you haven't already.
* Enable USB Debugging and plug your phone in to your computer.

## Building the Project
You can build for android the same way as described above using vscode or in the terminal with an android preset listed in [CMakePresets.json](https://github.com/Frogzalcoatl/platformer/blob/main/CMakePresets.json). On vscode, check tasks.json for android debugging options.

I also added a install_and_run target which will open the built apk file using usb debugging and a debug console on your computer.

Commands:
```
cmake --preset android-arm64
cmake --build --preset android-arm64-debug --target install_and_run
```
