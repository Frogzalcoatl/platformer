cmake_minimum_required(VERSION 3.22)
project(VirtualFileSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} STATIC src/VirtualFileSystem.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(TARGET SDL3::SDL3)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
    target_compile_definitions(${PROJECT_NAME} PUBLIC VFS_USE_SDL)
endif()

if(NOT CMAKE_CROSSCOMPILING OR HOST_BUILD)
    add_executable(Packer 
        src/Packer.cpp
    )
    target_link_libraries(Packer PRIVATE ${PROJECT_NAME})
    set_target_properties(Packer PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
endif()