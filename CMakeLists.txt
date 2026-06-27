cmake_minimum_required(VERSION 3.22)
project(MyPlatformer CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")

if(ANDROID)
    add_library(MyPlatformer SHARED ${SOURCES})
else()
    add_executable(MyPlatformer ${SOURCES})
endif()

target_include_directories(MyPlatformer PRIVATE src)

target_link_libraries(MyPlatformer PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
    SDL3_mixer::SDL3_mixer
    box2d::box2d
    imgui::imgui
)

if(ANDROID)
    target_link_options(MyPlatformer PRIVATE "-u" "ANativeActivity_onCreate")
endif()

if(MSVC)
    target_compile_options(MyPlatformer PRIVATE /W4 /WX)
else()
    target_compile_options(MyPlatformer PRIVATE -Wall -Wextra -Werror)
endif()