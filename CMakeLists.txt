cmake_minimum_required(VERSION 3.12)
project(TLM_Thesis)

set(CMAKE_CXX_STANDARD 17)

#add_subdirectory(googletest)
#
#include_directories(googletest/googletest/include)
#include_directories(googletest/googlemock/include)

add_executable(TLM_Thesis main.cpp src/Network.cpp src/Network.h src/Node.cpp src/Node.h src/param.h src/AirNetwork.cpp src/AirNetwork.h src/SeaNetwork.cpp src/SeaNetwork.h)
#add_executable(Test Test/testMain.cpp Test/NodeTest.cpp src/Node.h src/Node.cpp src/Network.cpp src/Network.h Test/networkTest.cpp src/AirNetwork.cpp src/AirNetwork.h src/SeaNetwork.cpp src/SeaNetwork.h)

#target_link_libraries(Test gtest gtest_main)

#-------set gurobi environment-------------#
#include(./Cmake/FindGurobi.cmake)
#if (GUROBI_FOUND)
#    target_compile_definitions(TLM_Thesis PUBLIC HAS_GUROBI)
#
#    target_include_directories(TLM_Thesis PRIVATE ${GUROBI_INCLUDE_DIRS})
#    target_link_libraries(TLM_Thesis ${GUROBI_LIBRARIES})
#endif()