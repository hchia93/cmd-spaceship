cmake_minimum_required(VERSION 3.20)
project(cmd-spaceship LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Only keep Debug and Release
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Sources
set(SOURCES
    src/main.cpp
    src/Bullet.cpp
    src/BulletPoolService.cpp
    src/GameWorld.cpp
    src/Inputs.cpp
    src/Network.cpp
    src/Spaceship.cpp
    src/Utils.cpp
)

set(HEADERS
    src/Bullet.h
    src/BulletPoolService.h
    src/GameWorld.h
    src/Inputs.h
    src/Macro.h
    src/Network.h
    src/Spaceship.h
    src/Utils.h
)

# Client target
add_executable(cmd-spaceship-client ${SOURCES} ${HEADERS})
target_compile_definitions(cmd-spaceship-client PRIVATE CLIENT_BUILD)

# Server target
add_executable(cmd-spaceship-server ${SOURCES} ${HEADERS})
target_compile_definitions(cmd-spaceship-server PRIVATE SERVER_BUILD)

# Output path
set_target_properties(cmd-spaceship-client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmd-spaceship
)
set_target_properties(cmd-spaceship-server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmd-spaceship
)
