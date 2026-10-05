cmake_minimum_required(VERSION 3.20)
project(SkyrimSnowboard CXX)
set(CMAKE_CXX_STANDARD 17)

# Build on Windows (Visual Studio or MinGW-w64), 64-bit. Run `python tools/gen.py` first.
# UNTESTED: this has not been built yet.
add_library(SkyrimSnowboard SHARED plugin.cpp)
target_include_directories(SkyrimSnowboard PRIVATE ${CMAKE_SOURCE_DIR}/../include)
# Ultimate ASI Loader loads files ending in .asi
set_target_properties(SkyrimSnowboard PROPERTIES PREFIX "" SUFFIX ".asi")
