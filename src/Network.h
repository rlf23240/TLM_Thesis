//
// Created by Ashee on 2019/5/22.
//

#ifndef TLM_THESIS_NETWORK_H
#define TLM_THESIS_NETWORK_H

#include <ostream>
#include "param.h"
#include "Node.h"

using namespace std;


struct Route{
    Route(const vector<string> &nodes, int cost);

    Route();
    friend ostream &operator<<(ostream &os, const Route &route);

    vector<string> nodes;
    int cost = INT_MAX;
};

class Network {
protected:
    unordered_map<char, vector<Node*>> nodes;
    int num_nodes;
    int* stop_cost;
    int** arc_cost;
    int** time_cost;

    virtual void add_nodes();
    void add_edges();
    bool add_edge(Node* start, Node* end, int cost);

    virtual void read_node(std::string node_data_path);
    void read_stop_cost(std::string cost_data_path);
    void read_time_cost(std::string time_data_path);
public:
    virtual void run_algo() = 0;
    virtual void read_data(std::string data_path);

    Route DP_shortest_path(char start_node, int start_time, char end_node, int end_time);

    virtual void forward_update(Route** dp, int node, int time);


};


#endif //TLM_THESIS_NETWORK_H
