# Contributing

This project is set up to have a pretty straightforward build process using CMake and Vcpkg.

I use vscode with the [ms-vscode.cpptools-extension-pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack) extension, however this project probably works fine using whatever environment you want, given you use a preset listed in [CMakePresets.json](https://github.com/Frogzalcoatl/platformer/blob/main/CMakePresets.json). I use "windows-clang-vs" then click "build" in the status bar, which is added by the vscode extension listed above.

### Windows
* Download [Build Tools for Visual Studio](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2026)
    * Select and install the **"Desktop development with C++"** workload.

### MacOS
* Install Xcode Command Line Tools:
```bash
xcode-select --install
```

### Linux
* Install the build tools for your respective distribution listed [here](https://wiki.libsdl.org/SDL3/README-linux).