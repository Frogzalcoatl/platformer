# Contributing

When updating the app version, change both `vcpkg.json` and `android-project/app/build.gradle` (versionName).

## Prerequisites

### 1. LLVM / Clang Tools (clangd & clang-format)
* **Windows:** `winget install LLVM.LLVM`
* **macOS:** `brew install llvm`
* **Debian / Ubuntu:** `sudo apt install clangd clang-format`
* **Fedora / RHEL:** `sudo dnf install clang-tools-extra`
* **Arch / Manjaro:** `sudo pacman -S clang`
* **openSUSE:** `sudo zypper install clang-tools`

### 2. Build Tools
**Windows:**

1. Download [Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2026)

2. Select and install the **"Desktop development with C++"** workload.

3. On the right side panel (**Installation Details**), only check:
    * **MSVC v14x - VS 20xx C++ x64/x86 build tools**
    * **C++ CMake tools for Windows**
    * **Windows 11 SDK** (Compatible with both Windows 10 and 11)

**MacOS:**

1. Install Xcode Command Line Tools:
```bash
xcode-select --install
```

2. Install Homebrew:
```
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

3. Install `cmake` and `ninja`:
```
brew install cmake ninja
```

**Linux:**

* Install the SDL3 build dependencies [here](https://wiki.libsdl.org/SDL3/README-linux).

## Building the Project
Use a preset listed in [CMakePresets.json](https://github.com/Frogzalcoatl/platformer/blob/main/CMakePresets.json).
```
cmake --preset windows
cmake --build build/windows --target run
```
Appending `--target run` is not required but opens the executable after building.

# Android Crosscompiling
## Prerequisites
### 1. Android SDK

1. Download [here](https://developer.android.com/studio#command-line-tools-only) and extract to a directory of your choice (for example, `C:\android-sdk` on Windows or `~/android-sdk` on Linux/MacOS).

2. Open a terminal in `android-sdk/cmdline-tools/bin` and run `sdkmanager.bat` (or `sdkmanager` on linux/macos) with the following arguments:
```
./sdkmanager.bat "platform-tools" "build-tools;34.0.0" "platforms;android-28"
```

3. Accept the licenses:
```
./sdkmanager.bat --licenses
```

4. Define the `ANDROID_HOME` environment variable pointing to the SDK root:
    * Example (Windows): `set ANDROID_HOME=C:\android-sdk`
    * Example (Linux/MacOS): `export ANDROID_HOME=~/Android/Sdk`

### 2. Android NDK (r29 suggested)

1. Download [here](https://developer.android.com/ndk/downloads) and extract.

2. Define the `ANDROID_NDK_HOME` environment variable to point to the extracted android-ndk-r29 folder:
    * Example (Windows): `set ANDROID_NDK_HOME=C:\android-ndk-r29`
    * Example (Linux/MacOS): `export ANDROID_NDK_HOME=~/Android/Sdk/ndk/29.x.xxxxxx`

### 3. Java Development Kit (JDK 17+)

1. Required for gradle to build the apk file. Install [here](https://adoptium.net/temurin/releases/?version=17). 

2. During installation, click the red X dropdown next to "Set JAVA_HOME variable" and select "Will be installed on local hard drive".
The variable can also be set manually after installing.

### 4. Enable USB Debugging (To debug using an android device)

* [Enable developer options](https://developer.android.com/studio/debug/dev-options) on the android device.
* Enable USB Debugging and plug the device in to your computer.
* If using windows, install the [Google USB Driver](https://developer.android.com/studio/run/win-usb).

## Building the Project
Use an android preset listed in [CMakePresets.json](https://github.com/Frogzalcoatl/platformer/blob/main/CMakePresets.json).
On vscode based IDEs, check tasks.json for android debugging options.

Commands:
```
cmake --preset android-arm64
cmake --build build/android-arm64 --target run
```
The run target for android acrosscompiling opens the built apk file using usb debugging and a debug console on your computer.
