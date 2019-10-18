//
// Created by Ashee on 2019/5/22.
//

#ifndef TLM_THESIS_NODE_H
#define TLM_THESIS_NODE_H
#define MAX(a,b) (((a)>(b))?(a):(b))

#include "string"
#include "unordered_map"
#include "vector"
#include "random"
#include "iostream"

using namespace std;

class Node;

struct Arc{
    Arc(Node *start_node, Node *end_node, int cost);

    Arc(Node *start_node, Node *end_node, int cost, double unit_profit, double unitCost);

    Arc(Node *start_node, Node *end_node, int cost, int volume_ub, double unit_profit, double unitCost);

    Arc(Node *start_node, Node *end_node, int cost, int volume_ub, int weight_ub, double unit_profit, double unitCost);

    double get_reduced_cost();
    void minus_fixed_profit(double fixed_profit);
    void minus_fixed_cost(double fixed_profit);

    Node* start_node;
    Node* end_node;
    double cost;
    int weight_ub;
    int volume_ub;
    double unit_profit;
    double unit_cost;

    double getUnitCost() const;

    double fixed_profit = 0;
    double fixed_cost = 0;

};

class Node {
public:
    Node(const std::string &name, int cost);
    
    // NULL if not connected
    Arc* connected(Node* node);
    
    int getCost() const;
    std::string getName() const;
    int getLayer() const;
    int getNode() const;
    int getTime() const;
    std::vector<Arc*> out_arcs;
    std::vector<Arc*> in_arcs;

    
private:
    int cost = 0;
    std::string name;
};
#endif //TLM_THESIS_NODE_H
