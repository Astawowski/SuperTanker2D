cmake_minimum_required(VERSION 3.27)
project(C___Projects)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++ -static")

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY  https://github.com/fmtlib/fmt
        GIT_TAG         10.2.1
)

FetchContent_Declare(
        SFML
        GIT_REPOSITORY  https://github.com/SFML/SFML
        GIT_TAG         2.6.1
)



FetchContent_MakeAvailable(fmt)
set(SFML_BUILD_STATIC_LIBS TRUE CACHE BOOL "Build static SFML libraries" FORCE)
FetchContent_MakeAvailable(SFML)

# Add your source files
set(SOURCE_FILES SuperTanker2D.cpp
        MapGeneratorEngine.cpp
        Field.h
        MapGeneratorEngine.h
        MyMath.cpp
        MyMath.h
        EnemyTank.h
        EnemyTankType.h
        EnemyTank.cpp)

add_executable(SuperTanker2D ${SOURCE_FILES})

target_link_libraries(SuperTanker2D fmt sfml-graphics sfml-system sfml-window)




