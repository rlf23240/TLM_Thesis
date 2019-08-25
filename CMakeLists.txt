cmake_minimum_required(VERSION 3.12)
project(TLM_Thesis)

set(CMAKE_CXX_STANDARD 17)

#
#include_directories(googletest/googletest/include)
#include_directories(googletest/googlemock/include)

set(src_files src/Dantzig_wolfe.cpp
              src/Dantzig_wolfe.h
              src/GurobiModel.cpp
              src/GurobiModel.h
              src/Network.cpp
              src/Network.h
              src/Node.cpp
              src/Node.h
              src/param.h
              src/AirNetwork.cpp
              src/AirNetwork.h
              src/SeaNetwork.cpp
              src/SeaNetwork.h
              src/EntireNetwork.cpp
              src/EntireNetwork.h src/Path.cpp
              src/Point.cpp
              src/Cargo.cpp
              src/CargoRoute.cpp
              src/CargoRoute.h
              src/BB_node.cpp
              src/BB_node.h
              src/Solution.cpp)

add_executable(TLM_Thesis main.cpp ${src_files})
add_executable(Model model.cpp)
add_executable(Gurobi GurobiMain.cpp ${src_files})
#add_executable(Gurobi_king src/Gurobi_try.cpp)
#add_executable(Generate_paths_arcs paths_arcs_gen.cpp ${src_files})
#add_executable(Test Test/testMain.cpp Test/NodeTest.cpp src/Node.h src/Node.cpp src/Network.cpp src/Network.h Test/networkTest.cpp src/AirNetwork.cpp src/AirNetwork.h src/SeaNetwork.cpp src/SeaNetwork.h)

#target_link_libraries(Test gtest gtest_main)

#-------set gurobi environment-------------#
include(./Cmake/FindGurobi.cmake)
if (GUROBI_FOUND)
    target_compile_definitions(TLM_Thesis PUBLIC HAS_GUROBI)

    target_include_directories(TLM_Thesis PRIVATE ${GUROBI_INCLUDE_DIRS})
    target_link_libraries(TLM_Thesis ${GUROBI_LIBRARIES})

    target_compile_definitions(Model PUBLIC HAS_GUROBI)

    target_include_directories(Model PRIVATE ${GUROBI_INCLUDE_DIRS})
    target_link_libraries(Model ${GUROBI_LIBRARIES})

    target_compile_definitions(Gurobi PUBLIC HAS_GUROBI)

    target_include_directories(Gurobi PRIVATE ${GUROBI_INCLUDE_DIRS})
    target_link_libraries(Gurobi ${GUROBI_LIBRARIES})

#    target_compile_definitions(Gurobi_king PUBLIC HAS_GUROBI)
#
#    target_include_directories(Gurobi_king PRIVATE ${GUROBI_INCLUDE_DIRS})
#    target_link_libraries(Gurobi_king ${GUROBI_LIBRARIES})

#    target_compile_definitions(Generate_paths_arcs PUBLIC HAS_GUROBI)
#
#    target_include_directories(Generate_paths_arcs PRIVATE ${GUROBI_INCLUDE_DIRS})
#    target_link_libraries(Generate_paths_arcs ${GUROBI_LIBRARIES})
endif()