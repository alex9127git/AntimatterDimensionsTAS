cmake_minimum_required(VERSION 3.23)
project(J.A.D.E. VERSION 0.0.1
                 DESCRIPTION "Jade's (and Alex9127's) Antimatter Dimensions Estimator"
                 LANGUAGES CXX)

add_executable(main)
target_sources(main 
    PRIVATE 
        main.cpp
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

target_link_libraries(main
    PRIVATE
        decimal
        constants
        dimensions
        tickspeed
        gamestate
)

target_link_libraries(dimensions
    PRIVATE
        constants
)

target_link_libraries(tickspeed
    PRIVATE
        constants
)

target_link_libraries(gamestate
    PRIVATE
        dimensions
        tickspeed
        decimal
)