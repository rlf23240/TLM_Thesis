cmake_minimum_required(VERSION 3.12)
project(TLM_Thesis)

set(CMAKE_CXX_STANDARD 17)

add_executable(TLM_Thesis main.cpp src/EntireNetwork.cpp src/EntireNetwork.h src/Network.cpp src/Network.h src/Node.cpp src/Node.h src/Arc.cpp src/Arc.h src/param.h)

#-------set gurobi environment-------------#
include(./Cmake/FindGurobi.cmake)
if (GUROBI_FOUND)
    target_compile_definitions(TLM_Thesis PUBLIC HAS_GUROBI)

    target_include_directories(TLM_Thesis PRIVATE ${GUROBI_INCLUDE_DIRS})
    target_link_libraries(TLM_Thesis ${GUROBI_LIBRARIES})
endif()