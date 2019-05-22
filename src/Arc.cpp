//
// Created by Ashee on 2019/5/22.
//

#include "Arc.h"


Arc::Arc(const std::string &start_node, const std::string &end_node, int cost) : start_node(start_node),
                                                                                 end_node(end_node), cost(cost) {}

const std::string &Arc::getStart_node() const {
    return start_node;
}

void Arc::setStart_node(const std::string &start_node) {
    Arc::start_node = start_node;
}

const std::string &Arc::getEnd_node() const {
    return end_node;
}

void Arc::setEnd_node(const std::string &end_node) {
    Arc::end_node = end_node;
}

int Arc::getCost() const {
    return cost;
}

void Arc::setCost(int cost) {
    Arc::cost = cost;
}
