cmake_minimum_required(VERSION 3.10)
project(DzwigSim)

set(CMAKE_CXX_STANDARD 17)

# Dodaj plik źródłowy
add_executable(outDebug projekt.cpp) # zmiana nazwy pliku wykonywalnego

# Ustaw folder wyjściowy na build/Debug
set_target_properties(outDebug PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/Debug
    WIN32_EXECUTABLE TRUE
)

# Linkowanie bibliotek WinAPI (dla MinGW)
if(MINGW)
    target_link_libraries(outDebug PRIVATE -static-libgcc -static-libstdc++ -mwindows)
endif()

# Linkowanie WinAPI dla MSVC (Visual Studio)
if(MSVC)
    target_link_libraries(outDebug PRIVATE user32 gdi32)
endif()
