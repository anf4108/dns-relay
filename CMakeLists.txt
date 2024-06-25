cmake_minimum_required(VERSION 3.17)
project(dnsr C)

set(CMAKE_C_STANDARD 11)

link_directories(/usr/local/lib /opt/homebrew/lib)
include_directories(/usr/local/include /opt/homebrew/include include)

add_executable(main
        src/main.c
        src/convert.c
)
target_link_libraries(main)