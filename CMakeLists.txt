cmake_minimum_required(VERSION 3.10.0)
project(translucent VERSION 0.1.0 LANGUAGES C)

add_subdirectory(external)

add_executable(translucent main.c)

target_include_directories(translucent PUBLIC include/)

target_link_libraries(translucent SDL3_image::SDL3_image SDL3::SDL3)