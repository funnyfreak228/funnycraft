cmake_minimum_required(VERSION 3.10)
project(funnycraft)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find wxWidgets
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

# Add nlohmann/json for JSON handling
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# Source files
set(SOURCES
    src/main.cpp
    src/MainFrame.cpp
    src/AccountManager.cpp
    src/InstanceManager.cpp
    src/MinecraftLauncher.cpp
    src/AccountDialog.cpp
    src/InstanceDialog.cpp
)

set(HEADERS
    src/MainFrame.h
    src/AccountManager.h
    src/InstanceManager.h
    src/MinecraftLauncher.h
    src/AccountDialog.h
    src/InstanceDialog.h
)

add_executable(funnycraft ${SOURCES} ${HEADERS})

target_link_libraries(funnycraft 
    ${wxWidgets_LIBRARIES}
    nlohmann_json::nlohmann_json
    stdc++fs
)

# Set data directory
set(FUNNYCRAFT_DATA_DIR "$ENV{HOME}/.funnycraft")
target_compile_definitions(funnycraft PRIVATE FUNNYCRAFT_DATA_DIR="${FUNNYCRAFT_DATA_DIR}")
