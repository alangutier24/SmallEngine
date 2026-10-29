cmake_minimum_required(VERSION 3.23)

project(Small_Engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(Small_Engine)

target_compile_definitions(Small_Engine PRIVATE
    RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)

find_package(glfw3 3.4 REQUIRED)

target_link_libraries(Small_Engine glfw)

find_package(OpenGL REQUIRED)

target_link_libraries(Small_Engine OpenGL::GL)

add_subdirectory(dependencies/glad)

target_link_libraries(Small_Engine glad)

add_subdirectory(dependencies/glm)

target_link_libraries(Small_Engine glm)

target_sources(Small_Engine
    PRIVATE
        main.cpp
        src/shader.cpp
        src/mesh.cpp
        src/window.cpp
        src/renderer.cpp
        src/application.cpp
        src/camera.cpp
        src/material.cpp
        src/input.cpp
        src/objloader.cpp

    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            include
            dependencies
            resources
        FILES   
            include/shader.h
            include/mesh.h
            include/window.h
            include/renderer.h
            include/application.h
            include/camera.h
            include/material.h
            include/input.h
            include/objloader.h

        )