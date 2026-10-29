SET(FETCHCONTENT_QUIET FALSE)

cmake_minimum_required(VERSION 3.23)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

project(J.A.D.E. VERSION 0.0.1
                 DESCRIPTION "Jade's (and Alex9127's) Antimatter Dimensions Estimator"
                 LANGUAGES CXX)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipoSupported)
if(ipoSupported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

find_package(OpenMP REQUIRED)

add_library(interfaces INTERFACE)
target_sources(interfaces
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS
            libraries/interfaces
        FILES
            libraries/interfaces/interfaces.h
)
target_link_libraries(interfaces
    INTERFACE
        nlohmann_json::nlohmann_json
)

add_library(ioutils)
target_sources(ioutils
    PRIVATE
        libraries/ioutils/ioutils.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/ioutils
        FILES
            libraries/ioutils/ioutils.h
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
    PUBLIC
        interfaces
)

add_library(instructions)
target_sources(instructions 
    PRIVATE 
        libraries/instructions/instructions.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/instructions
        FILES
            libraries/instructions/instructions.h
)
target_link_libraries(instructions
    PRIVATE
        decimal
)

add_library(constants INTERFACE)
target_sources(constants
    INTERFACE
        FILE_SET HEADERS
        BASE_DIRS
            libraries/constants
        FILES
            libraries/constants/constants.h
)
target_link_libraries(constants
    INTERFACE
        decimal
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
    PUBLIC
        interfaces
        decimal
    PRIVATE
        constants
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
    PUBLIC
        interfaces
        decimal
    PRIVATE
        tickspeed
        constants
)

add_library(sacrifice)
target_sources(sacrifice
    PRIVATE
        libraries/sacrifice/sacrifice.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/sacrifice
        FILES
            libraries/sacrifice/sacrifice.h
)
target_link_libraries(sacrifice
    PUBLIC
        interfaces
        constants
        decimal
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
target_link_libraries(achievements
    PUBLIC
        interfaces
        decimal
    PRIVATE
        constants
)

add_library(gamestate)
target_sources(gamestate
    PRIVATE
        libraries/gamestate/gamestate.cpp
        libraries/gamestate/formulae.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/gamestate
        FILES
            libraries/gamestate/gamestate.h
            libraries/gamestate/formulae.h
)
target_link_libraries(gamestate
    PUBLIC
        achievements
        dimensions
        interfaces
        constants
        tickspeed
        sacrifice
        decimal
    PRIVATE
        ioutils
)

add_library(gameinstance)
target_sources(gameinstance
    PRIVATE
        libraries/gameinstance/gameinstance.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/gameinstance
        FILES
            libraries/gameinstance/gameinstance.h
)
target_link_libraries(gameinstance
    PUBLIC
        instructions
        gamestate
        constants
)

add_library(simulation)
target_sources(simulation
    PRIVATE
        libraries/simulation/simulation.cpp
        libraries/simulation/searchnode.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS
            libraries/simulation
        FILES
            libraries/simulation/simulation.h
            libraries/simulation/searchnode.h
)
target_link_libraries(simulation
    PUBLIC
        gameinstance
        constants
    PRIVATE
        OpenMP::OpenMP_CXX
        instructions
        ioutils
)

add_executable(main)
target_sources(main 
    PRIVATE 
        main.cpp
)
target_link_libraries(main
    PRIVATE
        OpenMP::OpenMP_CXX
        gameinstance
        simulation
        ioutils
)

add_executable(custom)
target_sources(custom 
    PRIVATE 
        customRun.cpp
)
target_link_libraries(custom
    PRIVATE
        OpenMP::OpenMP_CXX
        gameinstance
        simulation
        ioutils
)

add_executable(testing)
target_sources(testing 
    PRIVATE 
        tests/test.cpp
)
target_link_libraries(testing
    PRIVATE
        OpenMP::OpenMP_CXX
        gameinstance
        simulation
        constants
        ioutils
        decimal
)