cmake_minimum_required(VERSION 3.12)
project(TLM_Thesis)

set(CMAKE_CXX_STANDARD 17)

add_executable(TLM_Thesis main.cpp Lable_selection_method/LSM.cpp Lable_selection_method/LSM.h Lable_selection_method/LSM_main.cpp)
add_executable(LSM_main Lable_selection_method/LSM_main.cpp)
