//
// Created by Ashee on 2019/5/22.
//
#include "Node.h"


Node::Node(const std::string &name, int cost) : cost(cost), name(name) {}

Arc::Arc(Node *start_node, Node *end_node, int cost) : start_node(start_node), end_node(end_node), cost(cost) {}
