cmake_minimum_required(VERSION 3.12)
project(TLM_Thesis)

set(CMAKE_CXX_STANDARD 17)

add_executable(TLM_Thesis main.cpp Lable_selection_method/LSM.cpp Lable_selection_method/LSM.h )
add_executable(LSM_main Lable_selection_method/LSM_main.cpp)

#-------set gurobi environment-------------#
include(./Cmake/FindGurobi.cmake)
if (GUROBI_FOUND)
    target_compile_definitions(TLM_Thesis PUBLIC HAS_GUROBI)

    target_include_directories(TLM_Thesis PRIVATE ${GUROBI_INCLUDE_DIRS})
    target_link_libraries(TLM_Thesis ${GUROBI_LIBRARIES})
endif()