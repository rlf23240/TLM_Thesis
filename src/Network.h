//
// Created by Ashee on 2019/5/22.
//

#ifndef TLM_THESIS_NETWORK_H
#define TLM_THESIS_NETWORK_H

#include "param.h"
#include "Node.h"

using namespace std;


class Network {
private:
    int N_nodes;
public:
    int getN_nodes() const;

    void setN_nodes(int N_nodes);

public:
    unordered_map<std::string, Node*> nodes;
    int** arc_cost;

    Network();
    bool add_node(std::string name, int cost);
    bool add_edge(std::string start, std::string end, int cost);
    void read_data(std::string data_path);
    void read_node(std::string node_data_path);
    void read_stop_cost(std::string cost_data_path);
    void read_flights_param(std::string flights_data);
};


#endif //TLM_THESIS_NETWORK_H
