cmake_minimum_required(VERSION 3.3)

file(GLOB loopSrc "*.h" "*.hpp" "*.cpp")

execute_process(COMMAND sdl2-config --cflags OUTPUT_VARIABLE SDLFlags)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SDLFLags}")
add_executable(loop ${loopSrc})
target_include_directories(loop PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(loop Loop ${SDL2_LIBRARY})
