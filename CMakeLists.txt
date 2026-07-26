cmake_minimum_required(VERSION 3.26)

# Vcpkg implementation influenced by AI
# Check if the developer has a global vcpkg defined in their environment variables
if(DEFINED ENV{VCPKG_ROOT})
    set(VCPKG_DIR "$ENV{VCPKG_ROOT}")
    message(STATUS "[vcpkg] Using global installation found in environment: ${VCPKG_DIR}")
else()
    # Fallback to local bootstrapping if no global vcpkg is found
    set(VCPKG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.vcpkg")
    if(NOT EXISTS "${VCPKG_DIR}/vcpkg" AND NOT EXISTS "${VCPKG_DIR}/vcpkg.exe")
        message(STATUS "[vcpkg] Global VCPKG_ROOT not found. Bootstrapping local vcpkg...")
        # Depth 1 means only cloning the latest commit without the entire repository history
        execute_process(
            COMMAND git clone --depth 1 https://github.com/microsoft/vcpkg.git "${VCPKG_DIR}"
            RESULT_VARIABLE CLONE_RESULT
        )
        if(NOT CLONE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to clone vcpkg repository.")
        endif()
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
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Static lib setup
cmake_policy(SET CMP0091 NEW)
# $<$<CONFIG:Debug>:Debug> Explanation
# In Debug: Becomes "MultiThreadedDebug" (/MTd).
# In Release: Becomes "MultiThreaded" (/MT).
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Project/Executable file name Here:
project(MyPlatformer CXX C)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Compile definition cmake var initialization
set(ASSETS_FOLDER_NAME "assets")
set(PACK_FILE_NAME "${PROJECT_NAME}.dat")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg.json" VCPKG_JSON_DATA)
string(JSON GAME_VERSION GET "${VCPKG_JSON_DATA}" "version")
message(STATUS "Configuring ${PROJECT_NAME} - Version: ${GAME_VERSION}")

# Libraries
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)
set(USE_DISCORD_RPC FALSE)
if(WIN32 OR (APPLE AND NOT IOS) OR LINUX)
    set(USE_DISCORD_RPC TRUE)
endif()
if(USE_DISCORD_RPC)
    find_path(DISCORD_INCLUDE_DIR NAMES discord_rpc.h)
    # Discord rpc static lib implementation influenced by AI
    if(DISCORD_INCLUDE_DIR)
        # Get the root installation directory of the triplet (the parent of /include)
        get_filename_component(DISCORD_ROOT "${DISCORD_INCLUDE_DIR}/.." ABSOLUTE)
        # Construct the platform-independent library filename (e.g. "discord-rpc.lib" or "libdiscord-rpc.a")
        set(DISCORD_LIB_NAME "${CMAKE_STATIC_LIBRARY_PREFIX}discord-rpc${CMAKE_STATIC_LIBRARY_SUFFIX}")
        # Directly construct the Release and Debug absolute paths
        set(DISCORD_LIBRARY_RELEASE "${DISCORD_ROOT}/lib/${DISCORD_LIB_NAME}")
        set(DISCORD_LIBRARY_DEBUG "${DISCORD_ROOT}/debug/lib/${DISCORD_LIB_NAME}")
        # Print paths to the output during configuration for easy verification
        message(STATUS "[Discord-RPC] Found header at: ${DISCORD_INCLUDE_DIR}")
        message(STATUS "[Discord-RPC] Release library: ${DISCORD_LIBRARY_RELEASE}")
        message(STATUS "[Discord-RPC] Debug library:   ${DISCORD_LIBRARY_DEBUG}")
        if(EXISTS "${DISCORD_LIBRARY_RELEASE}" AND EXISTS "${DISCORD_LIBRARY_DEBUG}")
            add_library(discord-rpc::discord-rpc STATIC IMPORTED)
            set_target_properties(discord-rpc::discord-rpc PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${DISCORD_INCLUDE_DIR}"
                IMPORTED_CONFIGURATIONS "RELEASE;DEBUG"
                IMPORTED_LOCATION_RELEASE "${DISCORD_LIBRARY_RELEASE}"
                IMPORTED_LOCATION_DEBUG "${DISCORD_LIBRARY_DEBUG}"
            )
        elseif(EXISTS "${DISCORD_LIBRARY_RELEASE}")
            # Fallback if only Release is available
            add_library(discord-rpc::discord-rpc STATIC IMPORTED)
            set_target_properties(discord-rpc::discord-rpc PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${DISCORD_INCLUDE_DIR}"
                IMPORTED_LOCATION "${DISCORD_LIBRARY_RELEASE}"
            )
        else()
            message(WARNING "[Discord-RPC] Could not locate the library file: ${DISCORD_LIBRARY_RELEASE}")
        endif()
    endif()
endif()

# Source C++ Files
set(SOURCES
    src/Main.cpp
    src/assets/AssetManager.cpp
    src/drawing/Drawing.cpp
    src/entities/Entity.cpp
    src/events/EventQueue.cpp
    src/gui/ImGuiWidgets.cpp
    src/gui/NotificationManager.cpp
    src/gui/TouchController.cpp
    src/gui/UiManager.cpp
    src/gui/ui-states/Debug.cpp
    src/gui/ui-states/MainMenu.cpp
    src/gui/ui-states/Paused.cpp
    src/gui/ui-states/PlayerSourceSetup.cpp
    src/gui/ui-states/Settings.cpp
    src/platformer/Camera.cpp
    src/platformer/EntityController.cpp
    src/platformer/Level.cpp
    src/platformer/Platformer.cpp
    src/system/Android.cpp
    src/system/Audiomanager.cpp
    src/system/DiscordRpcManager.cpp
    src/system/LogFormatting.cpp
    src/system/InputManager.cpp
    src/system/WindowManager.cpp
    src/user-data/JsonManager.cpp
    src/user-data/SettingsManager.cpp
)

if (ANDROID)
    list(APPEND SOURCES src/system/Android.cpp)
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
else()
    add_executable(${PROJECT_NAME} ${SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE src)
add_subdirectory(libs/vfs)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
    SDL3_mixer::SDL3_mixer
    box2d::box2d
    imgui::imgui
    rapidjson
    VirtualFileSystem
)
if(USE_DISCORD_RPC AND TARGET discord-rpc::discord-rpc)
    target_link_libraries(${PROJECT_NAME} PRIVATE discord-rpc::discord-rpc)
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_DISCORD_RPC)
endif()

# I like strict compilers
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        /W4
        $<$<CONFIG:Release>:/WX> # Warnings as errors only in release
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall 
        -Wextra 
        -Wconversion
        $<$<CONFIG:Release>:-Werror> # Warnings as errors only in release
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE _USE_MATH_DEFINES)
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    GameVersion="${GAME_VERSION}"
    AssetsFolderName="${ASSETS_FOLDER_NAME}"
    PackFileName="${PACK_FILE_NAME}"
)

if(NOT ANDROID)
    if(WIN32)
        set_target_properties(${PROJECT_NAME} PROPERTIES 
            WIN32_EXECUTABLE $<CONFIG:Release>
        )
    endif()
    add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E remove -f "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PACK_FILE_NAME}"
        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${ASSETS_FOLDER_NAME}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${ASSETS_FOLDER_NAME}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${ASSETS_FOLDER_NAME}"
        DEPENDS ${PROJECT_NAME}
        COMMENT "VFS (Desktop): Copying raw assets directory next to executable and removing old pack file..."
        VERBATIM
    )
    add_custom_target(pack_assets ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${ASSETS_FOLDER_NAME}"
        COMMAND $<TARGET_FILE:Packer>
                "${CMAKE_CURRENT_SOURCE_DIR}/${ASSETS_FOLDER_NAME}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${PACK_FILE_NAME}"
                "${GAME_VERSION}"
        DEPENDS ${PROJECT_NAME} Packer
        COMMENT "VFS (Desktop): Packing assets archive next to executable and removing raw assets..."
        VERBATIM
    )
else()
    # Android crosscompiling implementation influenced by AI
    set(ANDROID_PROJ_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android-project")
    set(ANDROID_PROJ_DIR_MAIN "${ANDROID_PROJ_DIR}/app/src/main")
    set(ANDROID_JNILIBS_DIR "${ANDROID_PROJ_DIR_MAIN}/jniLibs/${CMAKE_ANDROID_ARCH_ABI}")
    set(ANDROID_ASSETS_DIR "${ANDROID_PROJ_DIR_MAIN}/assets")
    # Map CMAKE_ANDROID_ARCH_ABI to the corresponding output directory name
    if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
        set(ANDROID_OUTPUT_DIR "android-arm64")
    elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "x86_64")
        set(ANDROID_OUTPUT_DIR "android-x86_64")
    else()
        set(ANDROID_OUTPUT_DIR "android-${CMAKE_ANDROID_ARCH_ABI}")
    endif()
    # Explicitly direct the output folder to be inside the build/<arch>/bin directory
    set(ANDROID_BIN_DEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/build/${ANDROID_OUTPUT_DIR}/bin")
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
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ANDROID_BIN_DEST_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${ANDROID_PROJ_DIR}/app/build/outputs/apk/$<IF:$<CONFIG:Debug>,debug/app-debug.apk,release/app-release-unsigned.apk>"
                "${ANDROID_BIN_DEST_DIR}/$<IF:$<CONFIG:Debug>,app-debug.apk,app-release-unsigned.apk>"
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
    # Use single-config Ninja for the host tool to avoid complex configuration subdirectories
    set(HOST_GENERATOR CMAKE_GENERATOR "Ninja")
    if(CMAKE_HOST_WIN32)
        set(PACKER_TOOL "${HOST_BUILD_DIR}/Packer.exe")
    else()
        set(PACKER_TOOL "${HOST_BUILD_DIR}/Packer")
    endif()
    # Spawns a background compiler targeting your Desktop environment to build Packer
    ExternalProject_Add(host_packer_project
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/vfs"
        BINARY_DIR "${HOST_BUILD_DIR}"
        ${HOST_GENERATOR}
        CMAKE_ARGS ${HOST_CMAKE_ARGS}
        INSTALL_COMMAND ""
    )
    add_custom_target(copy_assets
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${ANDROID_ASSETS_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ANDROID_ASSETS_DIR}"
        # Setup libs and copy .so files
        ${PREPARE_JNILIBS_COMMANDS}
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                "${CMAKE_CURRENT_SOURCE_DIR}/${ASSETS_FOLDER_NAME}"
                "${ANDROID_ASSETS_DIR}"
        # Compile APK with Gradle
        ${GRADLE_COMMANDS}
        WORKING_DIRECTORY "${ANDROID_PROJ_DIR}"
        DEPENDS ${PROJECT_NAME}
        COMMENT "VFS: Packaging raw assets folder and compiling APK..."
        VERBATIM
    )
    add_custom_target(pack_assets ALL
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${ANDROID_ASSETS_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ANDROID_ASSETS_DIR}"
        ${PREPARE_JNILIBS_COMMANDS}
        # Execute the compiled Host Packer to write the pack file inside the assets folder
        COMMAND "${PACKER_TOOL}" 
                "${CMAKE_CURRENT_SOURCE_DIR}/${ASSETS_FOLDER_NAME}" 
                "${ANDROID_ASSETS_DIR}/${PACK_FILE_NAME}" 
                "${GAME_VERSION}"
        ${GRADLE_COMMANDS}
        WORKING_DIRECTORY "${ANDROID_PROJ_DIR}"
        DEPENDS ${PROJECT_NAME} host_packer_project
        COMMENT "VFS: Building host Packer, packing assets bundle, and compiling APK..."
        VERBATIM
    )
    add_custom_target(install_and_run
        COMMAND adb install -r "${ANDROID_BIN_DEST_DIR}/$<IF:$<CONFIG:Debug>,app-debug.apk,app-release-unsigned.apk>"
        COMMAND adb shell am start -n "com.frogzalcoatl.platformer/org.libsdl.app.SDLActivity"
        DEPENDS pack_assets
        COMMENT "VFS: Deploying APK and launching..."
        USES_TERMINAL
        VERBATIM
    )
endif()