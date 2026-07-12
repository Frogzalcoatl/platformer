cmake_minimum_required(VERSION 3.26)

# vcpkg implementation written by AI
# Once again I don't care much about learning CMake at the moment
# Check if the developer has a global vcpkg defined in their environment variables
if(DEFINED ENV{VCPKG_ROOT})
    set(VCPKG_DIR "$ENV{VCPKG_ROOT}")
    message(STATUS "[vcpkg] Using global installation found in environment: ${VCPKG_DIR}")
else()
    # Fallback to local bootstrapping if no global vcpkg is found
    set(VCPKG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.vcpkg")

    if(NOT EXISTS "${VCPKG_DIR}/vcpkg" AND NOT EXISTS "${VCPKG_DIR}/vcpkg.exe")
        message(STATUS "[vcpkg] Global VCPKG_ROOT not found. Bootstrapping local vcpkg...")
        
        # Clone the lightweight single-depth vcpkg repository
        execute_process(
            COMMAND git clone --depth 1 https://github.com/microsoft/vcpkg.git "${VCPKG_DIR}"
            RESULT_VARIABLE CLONE_RESULT
        )
        if(NOT CLONE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to clone vcpkg repository.")
        endif()

        # Bootstrap the executable based on the host platform
        if(WIN32)
            execute_process(
                COMMAND "${VCPKG_DIR}/bootstrap-vcpkg.bat" -disableMetrics
                WORKING_DIRECTORY "${VCPKG_DIR}"
                RESULT_VARIABLE BOOTSTRAP_RESULT
            )
        else()
            execute_process(
                COMMAND sh "${VCPKG_DIR}/bootstrap-vcpkg.sh" -disableMetrics
                WORKING_DIRECTORY "${VCPKG_DIR}"
                RESULT_VARIABLE BOOTSTRAP_RESULT
            )
        endif()

        if(NOT BOOTSTRAP_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to bootstrap vcpkg compiler.")
        endif()
    else()
        message(STATUS "[vcpkg] Using existing local installation: ${VCPKG_DIR}")
    endif()
endif()

# Instruct CMake to use the detected toolchain
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

project(MyAndroidProject CXX C)

if(NOT ANDROID)
    message(FATAL_ERROR "This branch is configured exclusively for Android. Please configure using an Android preset (e.g. android-arm64 or android-x86_64).")
endif()

set(ASSETS_FOLDER_NAME "assets")
set(DAT_FILE_NAME "${PROJECT_NAME}.dat")

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json" VCPKG_JSON_DATA)
string(JSON GAME_VERSION GET "${VCPKG_JSON_DATA}" "version")
message(STATUS "Configuring ${PROJECT_NAME} - Version: ${GAME_VERSION}")

# Static lib setup
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCES
    src/AssetManager.cpp
    src/AudioManager.cpp
    src/Camera.cpp
    src/DiscordRpcManager.cpp
    src/Drawing.cpp
    src/Entity.cpp
    src/EntityController.cpp
    src/Events.cpp
    src/FormatLogs.cpp
    src/InputManager.cpp
    src/Level.cpp
    src/Main.cpp
    src/Platformer.cpp
    src/UiManager.cpp
    src/WindowManager.cpp
)

# Android requires native binaries to be SHARED libraries (.so)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

add_subdirectory(libs/vfs)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    GameVersion="${GAME_VERSION}"
    AssetsFolderName="${ASSETS_FOLDER_NAME}"
    DatFileName="${DAT_FILE_NAME}"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
    SDL3_mixer::SDL3_mixer
    box2d::box2d
    imgui::imgui
    VirtualFileSystem
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)

if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES 
        WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif()

set(ANDROID_PROJ_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android-project")
set(ANDROID_PROJ_DIR_MAIN "${ANDROID_PROJ_DIR}/app/src/main")
set(ANDROID_JNILIBS_DIR "${ANDROID_PROJ_DIR_MAIN}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}")
set(ANDROID_ASSETS_DIR "${ANDROID_PROJ_DIR_MAIN}/assets")

if(CMAKE_HOST_WIN32)
    set(GRADLE_CMD "gradlew.bat")
else()
    set(GRADLE_CMD "./gradlew")
endif()

if(CMAKE_HOST_UNIX)
    set(CHMOD_COMMAND COMMAND chmod +x "${ANDROID_PROJ_DIR}/${GRADLE_CMD}")
else()
    set(CHMOD_COMMAND)
endif()

set(PREPARE_JNILIBS_COMMANDS
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ANDROID_PROJ_DIR_MAIN}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ANDROID_JNILIBS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
            "$<TARGET_FILE:${PROJECT_NAME}>" 
            "${ANDROID_JNILIBS_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
)

set(GRADLE_COMMANDS
    ${CHMOD_COMMAND}
    COMMAND "${ANDROID_PROJ_DIR}/${GRADLE_CMD}" "assemble$<IF:$<CONFIG:Debug>,Debug,Release>"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ANDROID_PROJ_DIR}/app/build/outputs/apk/$<IF:$<CONFIG:Debug>,debug/app-debug.apk,release/app-release-unsigned.apk>"
            "${CMAKE_BINARY_DIR}/bin/$<IF:$<CONFIG:Debug>,app-debug.apk,app-release-unsigned.apk>"
)

add_custom_target(copy_assets ALL
    # 1. Clean and recreate assets folder
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${ANDROID_ASSETS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ANDROID_ASSETS_DIR}"

    # 2. Setup libs and copy .so files
    ${PREPARE_JNILIBS_COMMANDS}

    # 3. Copy Raw Assets Folder
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/${ASSETS_FOLDER_NAME}"
            "${ANDROID_ASSETS_DIR}"

    # 4. Compile APK with Gradle
    ${GRADLE_COMMANDS}
    
    WORKING_DIRECTORY "${ANDROID_PROJ_DIR}"
    DEPENDS ${PROJECT_NAME}
    COMMENT "VFS: Packaging raw assets folder and compiling APK..."
    VERBATIM
)

include(ExternalProject)
set(HOST_BUILD_DIR "${CMAKE_BINARY_DIR}/host_packer_build")

# Setup the host compiler arguments
set(HOST_CMAKE_ARGS 
    "-DCMAKE_BUILD_TYPE=Release"
    "-DHOST_BUILD=ON"
    "-DCMAKE_TOOLCHAIN_FILE=" # Clear the parent's Android cross-compiling toolchain
    "-DVCPKG_MANIFEST_MODE=OFF" # Prevent host packer from processing your game's vcpkg.json
)

# On Windows, we override the generator using CMAKE_GENERATOR.
# This prevents it from inheriting "Ninja Multi-Config" from the parent preset.
if(CMAKE_HOST_WIN32)
    # 1. Set the default fallback (Visual Studio 2022)
    set(PACKER_TOOL "${HOST_BUILD_DIR}/$<CONFIG>/Packer.exe")
    set(HOST_GENERATOR CMAKE_GENERATOR "Visual Studio 17 2022")

    # 2. Try to locate Microsoft's official VS locator (vswhere.exe)
    find_program(VSWHERE_EXE 
        NAMES vswhere 
        PATHS 
            "$ENV{ProgramFiles\(x86\)}/Microsoft Visual Studio/Installer"
            "$ENV{ProgramFiles}/Microsoft Visual Studio/Installer"
    )

    if(VSWHERE_EXE)
        # 3. Query vswhere to see if VS 2026 (v18.x) is installed on this machine
        execute_process(
            COMMAND "${VSWHERE_EXE}" -version "[18.0,19.0)" -products * -property installationVersion
            OUTPUT_VARIABLE VS_2026_INSTALLED
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        # 4. VS 2026 requires both the VS 2026 IDE/Build Tools AND CMake 4.2.0+
        if(VS_2026_INSTALLED AND CMAKE_VERSION VERSION_GREATER_EQUAL "4.2.0")
            set(HOST_GENERATOR CMAKE_GENERATOR "Visual Studio 18 2026")
            message(STATUS "[vfs] Detected Visual Studio 2026 and compatible CMake. Using VS 2026 generator.")
        else()
            message(STATUS "[vfs] Defaulting to Visual Studio 2022 generator.")
        endif()
    endif()
else()
    set(PACKER_TOOL "${HOST_BUILD_DIR}/Packer")
    set(HOST_GENERATOR CMAKE_GENERATOR "Ninja") # Forces single-config Ninja to prevent /Release/ subdirectory placement
endif()

# Spawns a background compiler targeting your Desktop environment to build Packer
ExternalProject_Add(host_packer_project
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/vfs"
    BINARY_DIR "${HOST_BUILD_DIR}"
    ${HOST_GENERATOR} # Pass CMAKE_GENERATOR override here
    CMAKE_ARGS ${HOST_CMAKE_ARGS}
    INSTALL_COMMAND ""
)

add_custom_target(pack_assets
    # 1. Clean and recreate assets folder
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${ANDROID_ASSETS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ANDROID_ASSETS_DIR}"

    # 2. Setup libs and copy .so files
    ${PREPARE_JNILIBS_COMMANDS}

    # 3. Execute the compiled Host Packer to write the .dat package inside the assets folder
    COMMAND "${PACKER_TOOL}" 
            "${CMAKE_CURRENT_SOURCE_DIR}/${ASSETS_FOLDER_NAME}" 
            "${ANDROID_ASSETS_DIR}/${DAT_FILE_NAME}" 
            "${GAME_VERSION}"

    # 4. Compile APK with Gradle
    ${GRADLE_COMMANDS}
    
    WORKING_DIRECTORY "${ANDROID_PROJ_DIR}"
    DEPENDS ${PROJECT_NAME} host_packer_project
    COMMENT "VFS: Building host Packer, packing .dat bundle, and compiling APK..."
    VERBATIM
)