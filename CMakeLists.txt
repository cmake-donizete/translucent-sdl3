cmake_minimum_required(VERSION 3.10.0)
project(translucent VERSION 0.1.0 LANGUAGES C)

add_subdirectory(external)

add_executable(translucent-sdl3 main.c)

target_include_directories(translucent-sdl3 PUBLIC include/)

target_link_libraries(translucent-sdl3 SDL3_image::SDL3_image SDL3::SDL3)