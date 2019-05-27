//
// Created by Ashee on 2019/5/22.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>
#include "../src/Node.h"

TEST(nodeTest, basic_check){
    Node node = Node("Jimmy");
    ASSERT_EQ(node.getName(),"Jimmy");
}

