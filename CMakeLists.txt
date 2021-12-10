cmake_minimum_required (VERSION 3.8)
project ("SimpleFileOscillator")
add_executable (simplefileoscillator "main.cpp" "wav.h" "wav.cpp")

# Find and link VOLK
find_package(Volk CONFIG REQUIRED)
target_link_libraries(simplefileoscillator Volk::volk)