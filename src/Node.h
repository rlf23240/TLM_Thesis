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
        Node(int cost);

    private:
        int cost = 0;

public:
    std::vector<Arc*> out_arc;
    std::vector<Arc*> in_arc;

};
#endif //TLM_THESIS_NODE_H
