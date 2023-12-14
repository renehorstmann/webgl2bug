cmake_minimum_required(VERSION 3.0)
project(webgltest C)
set(CMAKE_C_STANDARD 11)

add_executable(webgltest main.c)

# check platform
if (MINGW)
    message("Platform: Windows MinGW")
    set(PLATFORM_MINGW true)
elseif (WIN32)
    message("Platform: Windows MSVC")
    set(PLATFORM_MSVC true)
elseif (EMSCRIPTEN)
    set(PLATFORM_EMSCRIPTEN true)
else ()
    find_library(GL_LIB GL)
    find_library(GLES_LIB GLESv3)
    if (GL_LIB)
        message("Platform: Unix")
        set(PLATFORM_UNIX true)
    elseif (GLES_LIB)
        message("Platform: CxxDroid")
        set(PLATFORM_CXXDROID true)
    else ()
        message(FATAL_ERROR "Platform: unknown, neither GL nor GLES found!")
    endif ()
endif ()



if (PLATFORM_MINGW)
    message("Setup Windows MinGW")

    add_definitions(-Dmain=SDL_main)

    # mingw may not find a few libraries, due to missing CMAKE_LIBRARY_PATH lib paths
    find_library(OPENGL_LIB opengl32 REQUIRED)
    find_library(GLU_LIB glu32 REQUIRED)

    target_link_libraries(webgltest
            mingw32
            SDL2main
            SDL2
            ${OPENGL_LIB}
            ${GLU_LIB}
    )

endif()

if (PLATFORM_MSVC)
    message("Setup Windows MSVC")

    add_definitions(-Dmain=SDL_main)

    find_package(SDL2 REQUIRED)

    target_link_libraries(webgltest
            SDL2::SDL2
            SDL2::SDL2main
    )
endif ()

if (PLATFORM_UNIX)
    message("Setup Unix")

    find_package(SDL2 REQUIRED)
    find_library(GL_LIB GL REQUIRED)

    target_link_libraries(webgltest ${SDL2_LIBRARIES} ${GL_LIB})
endif ()

# emscripten
if (PLATFORM_EMSCRIPTEN)
    message("Setup Emscripten")
    # run cmake with:
    # cmake -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake ..
    # cmake --build .

    add_definitions(-DMIA_OPTION_GLES)

    # libs
    set(EMS_LIBS "-s USE_SDL=2 -s")

    # compile step needs the libs flags, so emscripten ports are automatically installed
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EMS_LIBS}")

    # link flags
    set(EMS_LINK_FLAGS "${EMS_LIBS} -s FULL_ES3=1 -s EXPORTED_FUNCTIONS='[\"_main\"]'")

    set_target_properties(webgltest PROPERTIES LINK_FLAGS ${EMS_LINK_FLAGS})


    set_target_properties(webgltest PROPERTIES OUTPUT_NAME "index" SUFFIX ".html")

endif ()


# CxxDroid
if (PLATFORM_CXXDROID)
    message("Setup CxxDroid")

    find_package(SDL2 REQUIRED)
    find_library(GLES_LIB GLESv3 REQUIRED)

    target_link_libraries(webgltest ${SDL2_LIBRARIES} ${GLES_LIB})
endif ()