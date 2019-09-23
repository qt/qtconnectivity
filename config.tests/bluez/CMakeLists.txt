# Generated from bluez.pro.

cmake_minimum_required(VERSION 3.14.0)
project(config_test_bluez LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    main.cpp
)
target_compile_options(${PROJECT_NAME}
    -fpermissive
)
