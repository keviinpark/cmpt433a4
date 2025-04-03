# CMake Build Configuration for root of project
cmake_minimum_required(VERSION 3.18)
project(beatbox
    VERSION 1.0 
    DESCRIPTION "Starter project" 
    LANGUAGES C
)

# Compiler options (inherited by sub-folders)
set(CMAKE_C_STANDARD 11)
add_compile_options(-Wall -Wpedantic -Wextra)
add_compile_options(-fdiagnostics-color)

# errors as warnings
add_compile_options(-Werror)

# debug symbols
add_compile_options(-g)

# ignore unused variables and parameters for now
#add_compile_options(-Wno-unused-variable -Wno-unused-function -Wno-unused-parameter)

# Enable address sanitizer
# (Comment this out to make your code faster)
#add_compile_options(-fsanitize=address)
#add_link_options(-fsanitize=address)

# Enable PThread library for linking
add_compile_options(-pthread)
add_link_options(-pthread)

# sound library
add_compile_options(-lasound)
add_link_options(-lasound)

# What folders to build
add_subdirectory(lgpio)
add_subdirectory(lcd)
add_subdirectory(common)
add_subdirectory(hal)  
add_subdirectory(app)
