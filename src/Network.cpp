//
// Created by Ashee on 2019/5/22.
//

#include "Network.h"
#include "vector"
#include "Arc.h"
#include "Node.h"

Network::Network() {}

bool Network::add_node(std::string name, int cost) {
    Node* new_node = new Node(name);
    new_node->setCost(cost);
    if(nodes[name] != nullptr){
        return false; //arc exist
    }
    nodes[name] = new_node;
    return true;
}

bool Network::add_edge(std::string start, std::string end, int cost) {
    Node* start_node = nodes[start];
    Node* end_node = nodes[end];
    Arc* new_arc = new Arc(start, end, cost);
    if(start_node->out_arc[end] != nullptr || end_node->out_arc[start] != nullptr)
        return false; //arc exist
    start_node->out_arc[end] = new_arc;
    end_node->in_arc[start] = new_arc;
    return true;
}



