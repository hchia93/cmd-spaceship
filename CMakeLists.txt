cmake_minimum_required(VERSION 3.20)
project(cmd-spaceship LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force custom configuration types (for multi-config generators like Visual Studio)
set(CMAKE_CONFIGURATION_TYPES "ClientDebug;ClientRelease;ServerDebug;ServerRelease" CACHE STRING "" FORCE)

# Output dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cmd-spaceship)

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

# Define one target
add_executable(cmd-spaceship ${SOURCES} ${HEADERS})

# Apply definitions depending on configuration
target_compile_definitions(cmd-spaceship PRIVATE
    $<$<CONFIG:ClientDebug>:CLIENT_BUILD;DEBUG>
    $<$<CONFIG:ClientRelease>:CLIENT_BUILD;NDEBUG>
    $<$<CONFIG:ServerDebug>:SERVER_BUILD;DEBUG>
    $<$<CONFIG:ServerRelease>:SERVER_BUILD;NDEBUG>
)

# Optional: optimization flags
target_compile_options(cmd-spaceship PRIVATE
    $<$<CONFIG:ClientDebug>:/Od>
    $<$<CONFIG:ServerDebug>:/Od>
    $<$<CONFIG:ClientRelease>:/O2>
    $<$<CONFIG:ServerRelease>:/O2>
)
