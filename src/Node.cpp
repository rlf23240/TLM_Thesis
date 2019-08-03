//
// Created by Ashee on 2019/5/22.
//
#include "Node.h"

Arc::Arc(Node *start_node, Node *end_node, int cost, double unitProfit)
        : start_node(start_node), end_node(end_node), cost(cost), unit_profit(unitProfit) {
    weight_ub = INT_MAX;
    volume_ub = INT_MAX;
}

Arc::Arc(Node *start_node, Node *end_node, int cost, int volume_ub, double unitProfit)
        : start_node(start_node), end_node(end_node),
          cost(cost), volume_ub(volume_ub), unit_profit(unitProfit) {
    volume_ub = INT_MAX;
}

Arc::Arc(Node *start_node, Node *end_node, int cost, int volume_ub, int weight_ub, double unitProfit)
        : start_node(start_node),
          end_node(end_node), cost(cost),
          volume_ub(volume_ub), weight_ub(weight_ub), unit_profit(unitProfit) {
}

void Arc::set_unit_profit(Node *start_node, Node *end_node) {
    int seed = start_node->getLayer() ^ end_node->getLayer() ^ start_node->getNode() ^ end_node->getNode();
    std::mt19937 generator(seed);
    std::uniform_real_distribution<float> ship_prof_dis(0.0, 1);
    std::uniform_real_distribution<float> flight_prof_dis(0.0, 1);

    if((start_node->getLayer() == 0 && end_node->getLayer() == 0) || (start_node->getLayer() == 3 && end_node->getLayer() == 3)){
        unit_profit = ship_prof_dis(generator);
    }else if((start_node->getLayer() == 1 && end_node->getLayer() == 1) || (start_node->getLayer() == 4 && end_node->getLayer() == 4)) {
        unit_profit = flight_prof_dis(generator);
    }else if(start_node->getLayer() == 5 && end_node->getLayer() == 5){
        unit_profit = ship_prof_dis(generator) / 2;
    }else if(start_node->getLayer() == 6 && end_node->getLayer() == 6){
        unit_profit = flight_prof_dis(generator) / 2;
    }else{
        unit_profit = 0;
    }
}


void Arc::minus_fixed_profit(double pi) {
    Arc::fixed_profit -= pi;
}

void Arc::minus_fixed_cost(double pi) {
    Arc::fixed_cost -= pi;
}

double Arc::get_reduced_cost() {
    return MAX(0,cost + fixed_cost);
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


