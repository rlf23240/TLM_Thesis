//
// Created by Ashee on 2019/5/22.
//

#ifndef TLM_THESIS_NODE_H
#define TLM_THESIS_NODE_H

#include "string"
#include "unordered_map"
#include "Arc.h"

class Node {
private:
    std::string name;
    int cost = 0;

public:
    std::unordered_map<std::string, Arc*> out_arc;
    std::unordered_map<std::string, Arc*> in_arc;

    Node(const std::string &name);

    const std::string &getName() const;

    int getCost() const;

    void setCost(int cost);
};
#endif //TLM_THESIS_NODE_H
