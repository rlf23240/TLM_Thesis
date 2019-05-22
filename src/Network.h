//
// Created by Ashee on 2019/5/22.
//

#ifndef TLM_THESIS_NETWORK_H
#define TLM_THESIS_NETWORK_H

#include "unordered_map"
#include "Node.h"

class Network {
private:
    std::unordered_map<std::string, Node*> nodes;
public:
    Network();
    bool add_node(std::string name, int cost);
    bool add_edge(std::string start, std::string end, int cost);
};


#endif //TLM_THESIS_NETWORK_H
