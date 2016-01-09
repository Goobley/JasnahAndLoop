cmake_minimum_required(VERSION 3.3)

file(GLOB libSrc "*.cpp")

add_library(Loop SHARED ${libSrc})
target_include_directories(Loop PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(Loop ${SDL2_LIBRARY})
