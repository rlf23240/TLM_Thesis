//
// Created by Ashee on 2019/5/22.
//
#include "Node.h"

Arc::Arc(Node *start_node, Node *end_node, int cost)
        : start_node(start_node), end_node(end_node), cost(cost){
    unit_profit = 0;
    unit_cost = 0;
    weight_ub = INT_MAX;
    volume_ub = INT_MAX;
}

Arc::Arc(Node *start_node, Node *end_node, int cost, double unitProfit, double unitCost)
        : start_node(start_node), end_node(end_node), cost(cost), unit_profit(unitProfit), unit_cost(unitCost) {
    weight_ub = INT_MAX;
    volume_ub = INT_MAX;
}

Arc::Arc(Node *start_node, Node *end_node, int cost, int volume_ub, double unitProfit, double unitCost)
        : start_node(start_node), end_node(end_node),
          cost(cost), volume_ub(volume_ub), unit_profit(unitProfit), unit_cost(unitCost)  {
    volume_ub = INT_MAX;
}

Arc::Arc(Node *start_node, Node *end_node, int cost, int volume_ub, int weight_ub, double unitProfit, double unitCost)
        : start_node(start_node),
          end_node(end_node), cost(cost),
          volume_ub(volume_ub), weight_ub(weight_ub), unit_profit(unitProfit), unit_cost(unitCost)  {
}


void Arc::minus_fixed_profit(double pi) {
    Arc::fixed_profit -= pi;
}

void Arc::minus_fixed_cost(double pi) {
    Arc::fixed_cost -= pi;
}

double Arc::get_reduced_cost() {
    return MAX(0, unit_cost + fixed_cost);
}

double Arc::getUnitCost() const {
    return unit_cost;
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


