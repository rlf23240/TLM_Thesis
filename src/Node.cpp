//
// Created by Ashee on 2019/5/22.
//
#include "Node.h"

Arc::Arc(Node *start_node, Node *end_node, int cost) : start_node(start_node), end_node(end_node), cost(cost) {
    weight_ub = INT_MAX;
    volume_ub = INT_MAX;
    set_unit_profit(start_node, end_node);
    set_unit_cost(start_node, end_node);
}

Arc::Arc(Node *start_node, Node *end_node, int cost, int volume_ub) : start_node(start_node), end_node(end_node),
                                                                      cost(cost), volume_ub(volume_ub) {
    volume_ub = INT_MAX;
    set_unit_profit(start_node, end_node);
    set_unit_cost(start_node, end_node);
}

Arc::Arc(Node *start_node, Node *end_node, int cost, int volume_ub, int weight_ub) : start_node(start_node),
                                                                                     end_node(end_node), cost(cost),
                                                                                     volume_ub(volume_ub), weight_ub(weight_ub) {
    set_unit_profit(start_node, end_node);
    set_unit_cost(start_node, end_node);
}

void Arc::set_unit_profit(Node *start_node, Node *end_node) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<float> ship_prof_dis(0.0, 3.0);
    std::uniform_real_distribution<float> flight_prof_dis(0.0, 1.0);

    if((start_node->getLayer() == 0 && end_node->getLayer() == 0) || (start_node->getLayer() == 3 && end_node->getLayer() == 3)){
        unit_profit = ship_prof_dis(generator);
    }else if((start_node->getLayer() == 1 && end_node->getLayer() == 1) || (start_node->getLayer() == 4 && end_node->getLayer() == 4)){
        unit_profit = flight_prof_dis(generator);
    }else{
        unit_profit = 0;
    }
}

void Arc::set_unit_cost(Node *start_node, Node *end_node) {
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<float> ship_cost_dis(0.0, 0.1);
    std::uniform_real_distribution<float> flight_cost_dis(1.0, 2.0);
    if((start_node->getLayer() == 0 && end_node->getLayer() == 0) || (start_node->getLayer() == 3 && end_node->getLayer() == 3 ) || (start_node->getLayer() == 5 && end_node->getLayer() == 5 )){
        unit_cost = ship_cost_dis(generator) * (end_node->getTime() - start_node->getTime());
    }else if((start_node->getLayer() == 1 && end_node->getLayer() == 1) || (start_node->getLayer() == 4 && end_node->getLayer() == 4) || (start_node->getLayer() == 6 && end_node->getLayer() == 6)) {
        unit_cost = flight_cost_dis(generator) * (end_node->getTime() - start_node->getTime());
    }
}

void Arc::minus_fixed_profit(double fixed_profit) {
    Arc::fixed_profit -= fixed_profit;
}


Node::Node(const std::string &name, int cost) : cost(cost), name(name) {}

const std::string &Node::getName() const {
    return name;
}

int Node::getCost() const {
    return cost;
}

int Node::getLayer() const {
    return (int) name[0] - 48; //ascii 48 = 0
}

int Node::getNode() const {
    return name[1] - 65; //char to int (A to 0)
}

int Node::getTime() const {
    return stoi(name.substr(2));
}


