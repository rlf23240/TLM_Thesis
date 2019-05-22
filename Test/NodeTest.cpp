//
// Created by Ashee on 2019/5/22.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>
#include "../src/Node.h"
#include "../src/Network.h"

TEST(nodeTest, basic_check){
    Node node = Node("Jimmy");
    ASSERT_EQ(node.getName(),"Jimmy");
}
TEST(networkTest, name_collision){
    Network network = Network();
    network.add_node("A1",0);
    ASSERT_EQ(network.add_node("A1",0), false);
    ASSERT_EQ(network.add_node("B1",0), true);

}
