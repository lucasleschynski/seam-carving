cmake_minimum_required(VERSION 3.10)
project(seamcarving)

find_package(OpenCV REQUIRED)
add_executable(seamcarving seam-carving.cpp)
target_link_libraries(seamcarving ${OpenCV_LIBS})
