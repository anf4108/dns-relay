cmake_minimum_required(VERSION 3.17)
project(dnsr C)

set(CMAKE_C_STANDARD 11)

link_directories(/usr/local/lib /opt/homebrew/lib)
include_directories(/usr/local/include /opt/homebrew/include include)

add_executable(main
        src/main.c
        src/struct.c
        src/convert.c
        src/config.c
        src/query.c
        src/tools.c
        src/bufferpool.c
        src/bp-doublelist.c
        src/bp-hashtable.c
)
target_link_libraries(main)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdiagnostics-color=always")

