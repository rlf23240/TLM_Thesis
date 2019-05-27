//
// Created by Ashee on 2019/5/23.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>
#include "../src/Network.h"

TEST(networkTest, name_collision){
    Network network = Network();
    network.add_node("A1",0);
    ASSERT_EQ(network.add_node("A1",0), false);
    ASSERT_EQ(network.add_node("B1",0), true);
}

TEST(networkTest, read_data_test){
    testing::internal::CaptureStdout();
    Network network = Network();
    network.read_data("../Data/Data1");
    string output = testing::internal::GetCapturedStdout();
}