//
// Created by Ashee on 2019/5/22.
//

//#pragma once
#ifndef TLM_THESIS_ARC_H
#define TLM_THESIS_ARC_H

#include "string"

class Arc {
private:
    std::string start_node;
    std::string end_node;
    int cost;
public:
    Arc(const std::string &start_node, const std::string &end_node, int cost);

    const std::string &getStart_node() const;

    void setStart_node(const std::string &start_node);

    const std::string &getEnd_node() const;

    void setEnd_node(const std::string &end_node);

    int getCost() const;

    void setCost(int cost);

};

#endif //TLM_THESIS_ARC_H


