# Contributing (Android Crosscompiling Branch)

You'll probably want to check whether the src folder in this branch is up to date with main. I most
likely will not always keep it up to date. I will try to always write code on the main branch in a
way that it will be compatible with android. Shouldn't be too hard since SDL handles basically all
the hard work for me.

If you ever update the app version, make sure to update both vcpkg.json and
android-project/app/build.gradle (versionName).

## Prerequisites

### 1. Clang Format

* I use LLVM's clang-format to format C++ files on save. There are a few different download sources
for this. To install clang-format without any of LLVM's other tools you can use Node.js or python.

**Node.js:**

```
npm install -g clang-format
```

**Python:**

```
pip install clang-format
```

**Full LLVM Installation:**

* If you don't care about installing all of LLVM's tools, then
download [here](https://github.com/llvm/llvm-project/releases). Make
sure you select "Add LLVM to the system PATH" during installation.

### 2. Build Tools
**Windows:**

* Download [Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2026)

* Select and install the **"Desktop development with C++"** workload.

**MacOS:**

* Install Xcode Command Line Tools:

```bash
xcode-select --install
```

**Linux:**

* Install the build tools for your respective distribution
  listed [here](https://wiki.libsdl.org/SDL3/README-linux).
* If there are additional dependencies I'm unaware of, they will likely be logged as an error or
  warning when you try to build the project.

### 3. Android SDK

* Download [here](https://developer.android.com/studio#command-line-tools-only) and extract. I
  personally extracted to my C: drive for simplicity. Open a terminal in
  `android-sdk/cmdline-tools/bin` and run `sdkmanager.bat` (Simply sdkmanager on linux/macos) with
  the following arguments:

```
./sdkmanager.bat "platform-tools" "build-tools;34.0.0" "platforms;android-28"
```

It also wont work unless you accept the licenses:

```
./sdkmanager.bat --licenses
```

Define the `ANDROID_HOME` environment variable pointing to your SDK root.

- Example (Windows): `set ANDROID_HOME=C:\android-sdk`
- Example (Linux/MacOS): `export ANDROID_HOME=~/Android/Sdk`

### 4. Android NDK (r29 suggested)

* Download [here](https://developer.android.com/ndk/downloads) and extract. No extra setup needed
  for the ndk. Define the `ANDROID_NDK_HOME` environment variable to point to the extracted
  android-ndk-r29 folder.
    - Example (Windows): `set ANDROID_NDK_HOME=C:\android-ndk-r29`
    - Example (Linux/MacOS): `export ANDROID_NDK_HOME=~/Android/Sdk/ndk/29.x.xxxxxx`

### 5. Java Development Kit (JDK 17+)

* Install [here](https://adoptium.net/temurin/releases/?version=17). Required for gradle to build
  the apk file.
    - During the installation you must click the red X dropdown next to "Set JAVA_HOME variable" and
      change it to "Will be installed on local hard drive". If you forget, you can also manually set
      the environment variable to the jdk directory after installing.

## Building the Project

I tried my best to set this up as straightforwardly as possible. Just like in the main branch, I use
CMake and Vcpkg. If you've never installed vcpkg, don't worry. It will automatically clone into the
project directory on build. However, if you have vcpkg installed globally with the VCPKG_ROOT
environment variable set up, your global installation is used for efficiency.

As for what has changed for android. CMake builds the .so file and copies the assets, then copies
them both to the android-project directory. Then gradle runs to turn those two files into a runnable
.apk file. You'll find the outputted apk copied to build/configurePresetName/bin. I also added a
CMake target called pack_assets. It runs the packer and bundles the pack file into the apk instead
of bundling each file from the assets directory individually.

This project works fine in whatever ide you want, given you use a preset listed
in [CMakePresets.json](https://github.com/Frogzalcoatl/platformer/blob/android/CMakePresets.json). I
use vscode with
the [ms-vscode.cpptools-extension-pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack).
I select "Android (ARM64)", "Android ARM64 Debug", then click "build" in the status bar.

If you do not want to use vscode, the equivalent commands are:

```
cmake --preset android-arm64
cmake --build --preset android-arm64-debug
```

First command uses
a [CMakePresets.json](https://github.com/Frogzalcoatl/platformer/blob/android/CMakePresets.json)
option in "configurePresets". Second uses a matching option in "buildPresets". Each configure preset
has a "debug" and "release" build preset.

If you want to run the packer during the build:

```
cmake --build --preset android-arm64-debug --target pack_assets
```

## Debugging with Android Studio

If you want to debug the android build, I suggest downloading [Android Studio](https://developer.android.com/studio). I plug my
android phone into my computer and use USB debugging, which I had to enable in my phone's developer options. SDL_Logs are visible in
the logcat tab (Looks like a little cat icon in the bottom left as of July 2026 when I run the
debugger).

Make sure to filter by `package:mine` or `tag:SDL/APP`, otherwise you'll see a bunch of logs from processes
not related to this project. If you're on Windows, you need to install the
[Google USB Driver](https://developer.android.com/studio/run/win-usb) to use USB debugging.

I include the extensions rainbow brackets for proper cpp text coloring and clang format for
formatting on save while in Android Studio.
