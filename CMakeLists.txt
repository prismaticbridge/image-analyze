set(OpenGL_GL_PREFERENCE GLVND)
cmake_minimum_required(VERSION 3.10)
project(image-analyze VERSION 2.0 LANGUAGES C)

# if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#   add_compile_options(-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
#   add_link_options(-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
# endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm")
  add_compile_definitions(STBI_NEON)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")

add_library(glad lib/gl.c)
target_include_directories(glad PUBLIC ${PROJECT_SOURCE_DIR}/include)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
# pain since its meson instead of cmake on arch linux
find_package(PkgConfig REQUIRED)
pkg_check_modules(SPNG REQUIRED IMPORTED_TARGET spng)
# search for either libturbojpeg or turbojpeg since some distros package it as one or the other
pkg_search_module(TURBOJPEG REQUIRED IMPORTED_TARGET libturbojpeg turbojpeg)
pkg_check_modules(HEIF IMPORTED_TARGET libheif)

file(GLOB SOURCES src/*.c)
add_executable(image-analyze ${SOURCES})
target_include_directories(image-analyze PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(image-analyze PRIVATE OpenGL::GL glad glfw PkgConfig::SPNG PkgConfig::TURBOJPEG dl m)

if (HEIF_FOUND)
  target_compile_definitions(image-analyze PRIVATE HAVE_LIBHEIF)
  target_link_libraries(image-analyze PRIVATE PkgConfig::HEIF)
endif()

install(TARGETS image-analyze DESTINATION bin)
