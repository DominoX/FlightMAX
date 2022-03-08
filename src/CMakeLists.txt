#
# build FlightMAX
#

cmake_minimum_required(VERSION 3.22.3)
project(FlightMAX VERSION 1.00.0 DESCRIPTION "FlightMAX plugin for XP11")
set(CMAKE_CXX_STANDARD 14)

# list of dirs to search for header files
message(STATUS "CMAKE_SOURCE_DIR is ${CMAKE_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/../../SDKs/XPSDK303/CHeaders/XPLM")
include_directories("${CMAKE_SOURCE_DIR}/../../SDKs/XPSDK303/CHeaders/Widgets")
include_directories("${CMAKE_SOURCE_DIR}/../../SDKs/XPSDK303/CHeaders/Wrappers")
include_directories("${CMAKE_SOURCE_DIR}/imgui")
include_directories("${CMAKE_SOURCE_DIR}/imgui/misc/cpp")
include_directories("${CMAKE_SOURCE_DIR}/ImgWindow")
include_directories("${CMAKE_SOURCE_DIR}/")

# include the necessary lib dirs (all libs in these dirs will be linked)
if (WIN32)
    list(APPEND CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/../../SDKs/XPSDK303/Libraries/Win")
    message(STATUS "Platform Win32 detected ... CMAKE_LIBRARY_PATH is ${CMAKE_LIBRARY_PATH}")
elseif (APPLE)
    list(APPEND CMAKE_FRAMEWORK_PATH "${CMAKE_SOURCE_DIR}/../../SDKs/XPSDK302/Libraries/Mac")
    message(STATUS "Platform APPLE detected ... CMAKE_FRAMEWORK_PATH is ${CMAKE_FRAMEWORK_PATH}")
elseif (UNIX)
    message(STATUS "Platform UNIX detected ... no action")
endif ()

# define which XP11 SDK APIs are valid
add_definitions(-DXPLM303=1)
add_definitions(-DXPLM301=1)
add_definitions(-DXPLM300=1)
add_definitions(-DXPLM210=1)
add_definitions(-DXPLM200=1)

# platform defines
add_definitions(-DIBM=$<BOOL:${WIN32}>)
add_definitions(-DAPL=$<BOOL:${APPLE}>)
add_definitions(-DLIN=$<AND:$<BOOL:${UNIX}>,$<NOT:$<BOOL:${APPLE}>>>)

# Visual Studio defines compiler switches via CMakeSettings.json
if (NOT MSVC)
    message(STATUS "IDE is NOT MSVC")
    # Force-enable exception support. This is most likely redundant, although for C
    # code the default is the opposite. Since we are mixing C++ and C libraries,
    # safer to set it on?
    add_compile_options(-fexceptions)
    # On UNIX systems this makes symbols non-exported by default. On Windows this
    # option is simply ignored, since symbol visibility works differently there.
    add_compile_options(-fvisibility=hidden)

    # Enable stricter warnings and then disable some we are not interested in.
    add_compile_options(-Wall -Wpedantic -Wshadow -Wextra)
    add_compile_options(-Wno-unused)

    # Always use position-independent code and highest optimization level (FPS!).
    add_compile_options(-O3)
    if (NOT WIN32)
        message(STATUS "NOT WIN32 is true")
        add_compile_options(-fPIC)
    endif()
else()
    message(STATUS "IDE is MSVC ... compiler switches in CMakeSettings.json")
endif ()

# X-Plane plugin
# FIXME: Split this into individual targets.
list(APPEND FLIGHTMAX_SRCS
    FlightMAX.cpp
    FlightMAX_starter_window.cpp
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/misc/cpp/imgui_stdlib.cpp
    ImgWindow/ImgFontAtlas.cpp
    ImgWindow/ImgWindow.cpp
)
add_library(FlightMAX SHARED ${FLIGHTMAX_SRCS})
target_compile_features(FlightMAX PUBLIC cxx_std_14)

if (APPLE)
    # X-Plane supports OS X 10.10+, so this should ensure FlyWithLua can run on
    # all supported versions.
    target_compile_options(FlightMAX PUBLIC -mmacosx-version-min=10.10)
    target_link_libraries(FlightMAX -mmacosx-version-min=10.10)
endif ()

# Link OpenGL and OpenAL related libraries.
# find_package(OpenGL REQUIRED)  # apt install freeglut3-dev
find_library(OPENGL_LIBRARY NAMES OpenGL)
find_library(OPENAL_LIBRARY NAMES openal OpenAL OpenAL32)  # apt install libopenal-dev
find_library(GLUT_LIBRARY NAMES glut GLUT glut64)  # apt install freeglut3-dev
message(STATUS "OPENGL_LIBRARY found at ${OPENGL_LIBRARY}")
message(STATUS "OPENAL_LIBRARY found at ${OPENAL_LIBRARY}")
message(STATUS "GLUT_LIBRARY found at ${GLUT_LIBRARY}")

# Link X-Plane plugin system libraries. They are only provided for OS X and Windows.
if (WIN32 OR APPLE)
    find_library(XPLM_LIBRARY NAMES XPLM XPLM_64.lib)
    message(STATUS "XPLM_LIBRARY found at ${XPLM_LIBRARY}")
    target_link_libraries(FlightMAX ${XPLM_LIBRARY})
endif ()

# Link library for dynamic loading of shared objects on UNIX systems.
if (UNIX)
    find_library(DL_LIBRARY dl)
    target_link_libraries(FlightMAX ${DL_LIBRARY})
endif ()

# Link OS X core system libraries.
if (APPLE)
    find_library(APPLICATION_SERVICES ApplicationServices)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(OpenGL_LIBRARY OpenGL)
    target_link_libraries(FlightMAX ${APPLICATION_SERVICES} ${CORE_FOUNDATION_LIBRARY} ${OpenGL_LIBRARY})
endif ()

if (WIN32)
    # Unlike OS X and Linux we build standard libraries statically since X-Plane does not provide them.
    if (MSVC)
        # Visual Studio includes many standard libraries already, just need to make sure OpenGL is linked
        message(STATUS "WIN32 & MSVC - Targeting link library opengl32")
        target_link_libraries(FlightMAX opengl32)
    else ()
        target_link_libraries(FlightMAX -static-libgcc -static-libstdc++ opengl32)
        # MSYS2 for some reason always links against winpthread, so we have to work around it by statically linking the
        # entire plugin. This allows building the plugin nativaly on Windows.
        target_link_libraries(FlightMAX -static)
    endif ()
elseif (APPLE)
    # Restrict set of symbols exported from the plugin. This reduces changes of
    # conflict with other plugins, in particular ones with Lua interpreter
    # embedded.
    target_link_libraries(FlightMAX "-exported_symbols_list ${CMAKE_SOURCE_DIR}/FlightMAX.sym_mac")
    target_link_libraries(FlightMAX "-ldl")
elseif (UNIX)
    # Specify additional runtime search laths for dynamically-linked libraries.
    # Restrict set of symbols exported from the plugin. This reduces changes of
    # conflict with other plugins, in particular ones with Lua interpreter
    # embedded.
    target_link_libraries(FlightMAX -Wl,--version-script -Wl,${CMAKE_SOURCE_DIR}/FlightMAX.sym)
endif ()

set_target_properties(FlightMAX PROPERTIES PREFIX "")
set_target_properties(FlightMAX PROPERTIES OUTPUT_NAME "FlightMAX")
set_target_properties(FlightMAX PROPERTIES SUFFIX ".xpl")
