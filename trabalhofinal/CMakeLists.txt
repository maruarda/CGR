cmake_minimum_required(VERSION 3.10)
project(trabalhofinal C)

set(CMAKE_C_STANDARD 11)

add_executable(trabalhofinal
    src/main.c
    src/shader.c
    src/camera.c
    src/profile.c
    src/mesh.c
    src/sweep.c
    src/texture.c
    external/glad/src/glad.c   

target_include_directories(trabalhofinal PRIVATE
    include
    external/glad/include      
)

find_package(glfw3 REQUIRED)
target_link_libraries(trabalhofinal glfw ${CMAKE_DL_LIBS})

