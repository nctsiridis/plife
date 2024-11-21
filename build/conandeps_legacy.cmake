message(STATUS "Conan: Using CMakeDeps conandeps_legacy.cmake aggregator via include()")
message(STATUS "Conan: It is recommended to use explicit find_package() per dependency instead")

find_package(ZLIB)
find_package(SDL2)

set(CONANDEPS_LEGACY  ZLIB::ZLIB  SDL2::SDL2main )