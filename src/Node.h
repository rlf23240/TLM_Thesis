//
// Created by Ashee on 2019/5/22.
//

#ifndef TLM_THESIS_NODE_H
#define TLM_THESIS_NODE_H

#include "string"
#include "unordered_map"
#include "vector"

class Node;

struct Arc{
    Arc(Node *start_node, Node *end_node, int cost);

    Arc(Node *start_node, Node *end_node, int cost, int weight_ub);

    Arc(Node *start_node, Node *end_node, int cost, int weight_ub, int volume_ub);

    Node* start_node;
    Node* end_node;
    int cost;
    int weight_ub;
    int volume_ub;
};

class Node {
public:
    Node(const std::string &name, int cost);

    int getCost() const;
    const std::string &getName() const;
    std::vector<Arc*> out_arcs;
    std::vector<Arc*> in_arcs;


private:
    int cost = 0;
    std::string name;

};
#endif //TLM_THESIS_NODE_H
