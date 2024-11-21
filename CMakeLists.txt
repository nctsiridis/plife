
cmake_minimum_required(VERSION 3.15)
project(plife C)

# Find ZLIB package
find_package(ZLIB REQUIRED)

# Find SDL2 package (assuming SDL2)
find_package(SDL2 REQUIRED)

# Add the executable
add_executable(${PROJECT_NAME} src/main.c)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB SDL2::SDL2)
