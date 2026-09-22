cmake_minimum_required(VERSION 3.16)
project(jobanote C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(${PROJECT_NAME}
"src/main.c"
    "src/jobanote.c"
    src/jbgl.c
  "src/odt.c"  "src/odt_styles.c" "src/document_renderer.c")

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/vendor/stb
    ${CMAKE_SOURCE_DIR}/vendor/raygui/src
)

set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/resources)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${RESOURCE_DIR}
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)

find_package(glfw3 3.4 QUIET)

if (NOT glfw3_FOUND)
    include(FetchContent)

    FetchContent_Declare(
        glfw3
        URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
    )

    FetchContent_MakeAvailable(glfw3)
endif()

find_package(OpenGL REQUIRED)

add_library(glad STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/src/gl.c
 )

target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include
)

add_subdirectory(vendor/cglm EXCLUDE_FROM_ALL)

if(WIN32)
    set(NFD_PLATFORM_SRC vendor/nativefiledialog/src/nfd_win.cpp)
elseif(APPLE)
    set(NFD_PLATFORM_SRC vendor/nativefiledialog/src/nfd_cocoa.m)
else()
    set(NFD_PLATFORM_SRC vendor/nativefiledialog/src/nfd_gtk.c)
endif()

add_library(nfd STATIC
    vendor/nativefiledialog/src/nfd_common.c
    ${NFD_PLATFORM_SRC}
)

target_include_directories(nfd PUBLIC
    vendor/nativefiledialog/src/include
)

add_library(miniz vendor/miniz/miniz.c  )

target_include_directories(miniz PUBLIC
    vendor/miniz
)

set(LIBXML2_WITH_ICONV OFF CACHE BOOL "" FORCE)
set(LIBXML2_WITH_PYTHON OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    libxml2
    GIT_REPOSITORY https://gitlab.gnome.org/GNOME/libxml2.git
    GIT_TAG v2.14.5
)

FetchContent_MakeAvailable(libxml2)

FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
    GIT_TAG VER-2-13-3
)

FetchContent_MakeAvailable(freetype)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    freetype
    cglm_headers
    nfd
    glfw
    glad
    OpenGL::GL
    miniz
    LibXml2::LibXml2
)