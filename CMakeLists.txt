cmake_minimum_required(VERSION 3.26)

project(MyPlatformer CXX C)

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

find_path(DISCORD_INCLUDE_DIR NAMES discord_rpc.h)

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

add_executable(${PROJECT_NAME} ${SOURCES})

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
    discord-rpc::discord-rpc
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

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying modified assets to the build directory next to the executable"
)

vfs_pack_assets(
    ${PROJECT_NAME}
    "${CMAKE_CURRENT_SOURCE_DIR}/${ASSETS_FOLDER_NAME}"
    "${DAT_FILE_NAME}"
    "${GAME_VERSION}"
)