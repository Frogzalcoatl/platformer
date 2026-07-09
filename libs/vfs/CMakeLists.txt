cmake_minimum_required(VERSION 3.22)
project(VirtualFileSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} STATIC src/VirtualFileSystem.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(Packer 
    src/Packer.cpp
)

target_link_libraries(Packer PRIVATE VirtualFileSystem)

set_target_properties(Packer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}"
)

# Func that runs packer if assets directory is updated
# Written by AI. Don't really care about learning Cmake.
macro(vfs_pack_assets TARGET_NAME ASSETS_DIR OUTPUT_NAME VERSION)
    # Ensure we have an absolute path to the assets directory
    get_filename_component(_vfs_abs_assets_dir "${ASSETS_DIR}" ABSOLUTE)

    # 1. Recursively find and track asset files to detect updates
    file(GLOB_RECURSE _vfs_asset_files CONFIGURE_DEPENDS
        "${_vfs_abs_assets_dir}/*"
    )

    # 2. Define a local build stamp file in the VFS subproject directory
    set(_vfs_stamp_file "${VirtualFileSystem_BINARY_DIR}/${OUTPUT_NAME}.stamp")
    
    # 3. Define the actual destination of the packed file using generator expressions
    set(_vfs_dat_file_path "$<TARGET_FILE_DIR:${TARGET_NAME}>/${OUTPUT_NAME}")

    # 4. Create file-level custom command
    # NOTE: We do not use BYPRODUCTS here because target-dependent generator expressions
    # are forbidden in both OUTPUT and BYPRODUCTS fields.
    add_custom_command(
        OUTPUT "${_vfs_stamp_file}"
        COMMAND $<TARGET_FILE:Packer> 
                "${_vfs_abs_assets_dir}" 
                "${_vfs_dat_file_path}" 
                "${VERSION}"
        COMMAND ${CMAKE_COMMAND} -E touch "${_vfs_stamp_file}"
        DEPENDS Packer ${_vfs_asset_files}
        COMMENT "VFS: Packing assets into ${OUTPUT_NAME} next to the ${TARGET_NAME} executable"
        VERBATIM
    )

    # 5. Create a unique custom target linked to our stamp file
    set(_vfs_pack_target_name "pack_${TARGET_NAME}_assets")
    add_custom_target(${_vfs_pack_target_name}
        DEPENDS "${_vfs_stamp_file}"
    )

    # 6. Hook the custom target into the main target's dependency chain
    add_dependencies(${TARGET_NAME} ${_vfs_pack_target_name})
endmacro()