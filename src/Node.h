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

    Node* start_node;
    Node* end_node;
    int cost;
};

class Node {
public:
    explicit Node(int cost);

    Node(const std::string &name, int cost);

    std::vector<Arc*> out_arcs;
    std::vector<Arc*> in_arcs;

private:
    int cost = 0;
public:
    int getCost() const;

private:
    std::string name;
public:
    const std::string &getName() const;

};
#endif //TLM_THESIS_NODE_H