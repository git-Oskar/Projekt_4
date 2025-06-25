cmake_minimum_required(VERSION 3.10)
project(DzwigSim)

set(CMAKE_CXX_STANDARD 17)

# Dodaj plik źródłowy
add_executable(outDebug projekt.cpp)

# Ustaw folder wyjściowy na build/Debug
set_target_properties(outDebug PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
)

# Linkowanie bibliotek WinAPI (jeśli potrzebne)
if(MINGW)
    target_link_libraries(outDebug PRIVATE -static-libgcc -static-libstdc++)
endif()