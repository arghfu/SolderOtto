cmake_minimum_required(VERSION 4.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(solderotto)

target_sources(app PRIVATE
        src/main.c
        src/ext_log_system.c
        src/ext_log_system_adapter.c
        src/sample_instance.c
        src/sample_module.c)

target_include_directories(app PRIVATE include)