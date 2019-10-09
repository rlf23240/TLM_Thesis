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

Node::Node(const std::string &name, int cost) : cost(cost), name(name) {
//    for node which has layer
    if(name.size() > 2) {
      layer = (int)name[0] - 48;
      node = name[1] - 65;
      time = stoi(name.substr(2));
    }else{ // otherwise
      node = (int)name[0] -48;
      time = stoi(name.substr(1));
    }
}

const std::string &Node::getName() const {
    return name;
}

int Node::getCost() const {
    return cost;
}

int Node::getLayer() const {
      return layer; //ascii 48 = 0
}

int Node::getNode() const {
    return node; //char to int (A to 0)
}

int Node::getTime() const {
    return time;
}

int Node::excel_alpha_to_num(string str){
  if(str.size() <= 1)
    return (int) str[0] - 65;
  else{
    return ((int) str[0] - 65) * 26 + ((int) str[1] - 65);
  }
}

