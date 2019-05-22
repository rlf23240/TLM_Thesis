//
// Created by Ashee on 2019/5/22.
//
#include "Node.h"

const std::string &Node::getName() const {
    return name;
}

Node::Node(const std::string &name) : name(name) {}

int Node::getCost() const {
    return cost;
}

void Node::setCost(int cost) {
    Node::cost = cost;
}
