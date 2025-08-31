cmake_minimum_required(VERSION 4.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app LANGUAGES C)

target_sources(app PRIVATE
        src/main.c
        src/wave_control.c
        src/display.c
        src/flash.c
        src/pid.c
        src/debug.c
        src/moving_average.c
        src/channel.c
        src/detect.c
)

target_include_directories(app PRIVATE include)