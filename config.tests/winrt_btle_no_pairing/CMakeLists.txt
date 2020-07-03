# Generated from winrt.pro.

cmake_minimum_required(VERSION 3.14.0)
project(config_test_winrt LANGUAGES CXX)

add_executable(${PROJECT_NAME}
    main.cpp
)

if(NOT WINRT)
    target_link_libraries(${PROJECT_NAME} PUBLIC
        runtimeobject.lib
    )
endif()
