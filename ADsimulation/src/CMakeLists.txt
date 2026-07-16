cmake_minimum_required(VERSION 3.23)
set(CMAKE_BUILD_TYPE Debug)
project(J.A.D.E. VERSION 0.0.1
                 DESCRIPTION "Jade's (and Alex9127's) Antimatter Dimensions Estimator"
                 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include_directories(${nlohmann_json_SOURCE_DIR}/include)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
)

FetchContent_MakeAvailable(nlohmann_json)

add_executable(main)
target_sources(main 
    PRIVATE 
        main.cpp
)
target_link_libraries(main
    PRIVATE
        decimal
        constants
        dimensions
        tickspeed
        achievements
        gamestate
        permutations
        simulation
)

add_executable(testing)
target_sources(testing 
    PRIVATE 
        tests/test.cpp
)
target_link_libraries(testing
    PRIVATE
        decimal
        constants
        dimensions
        tickspeed
        achievements
        gamestate
        permutations
        simulation
)

add_library(decimal)
target_sources(decimal
    PRIVATE
        libraries/decimal/decimal.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/decimal
        FILES
            libraries/decimal/decimal.h
)
target_link_libraries(decimal
    PRIVATE
        interfaces
        nlohmann_json::nlohmann_json
)

add_library(constants)
target_sources(constants
    PRIVATE
        libraries/constants/constants.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/constants
        FILES
            libraries/constants/constants.h
)
target_link_libraries(constants
    PRIVATE
        decimal
        interfaces
        nlohmann_json::nlohmann_json
)

add_library(dimensions)
target_sources(dimensions
    PRIVATE
        libraries/dimensions/dimensions.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/dimensions
        FILES
            libraries/dimensions/dimensions.h
)
target_link_libraries(dimensions
    PRIVATE
        constants
        interfaces
        nlohmann_json::nlohmann_json
)

add_library(tickspeed)
target_sources(tickspeed
    PRIVATE
        libraries/tickspeed/tickspeed.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/tickspeed
        FILES
            libraries/tickspeed/tickspeed.h
)
target_link_libraries(tickspeed
    PRIVATE
        constants
        interfaces
        nlohmann_json::nlohmann_json
)

add_library(achievements)
target_sources(achievements
    PRIVATE
        libraries/achievements/achievements.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/achievements
        FILES
            libraries/achievements/achievements.h
)

add_library(gamestate)
target_sources(gamestate
    PRIVATE
        libraries/gamestate/gamestate.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/gamestate
        FILES
            libraries/gamestate/gamestate.h
)
target_link_libraries(gamestate
    PRIVATE
        decimal
        dimensions
        tickspeed
        achievements
        nlohmann_json::nlohmann_json
)

add_library(permutations)
target_sources(permutations
    PRIVATE
        libraries/permutations/permutations.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/permutations
        FILES
            libraries/permutations/permutations.h
)

add_library(simulation)
target_sources(simulation
    PRIVATE
        libraries/simulation/simulation.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/simulation
        FILES
            libraries/simulation/simulation.h
)
target_link_libraries(simulation
    PRIVATE
        gamestate
        permutations
)

add_library(interfaces)
target_sources(interfaces
    PRIVATE
        libraries/interfaces/interfaces.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/interfaces
        FILES
            libraries/interfaces/interfaces.h
)
target_link_libraries(interfaces
    PRIVATE
        nlohmann_json::nlohmann_json
)