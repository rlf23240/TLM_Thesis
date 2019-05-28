//
// Created by Ashee on 2019/5/22.
//

#ifndef TLM_THESIS_NETWORK_H
#define TLM_THESIS_NETWORK_H

#include <ostream>
#include "param.h"
#include "Node.h"

using namespace std;

struct Flight{
    Flight(const string &start_node, int gap, int freq, int cycle_time);

    friend ostream &operator<<(ostream &os, const Flight &flight);

    string start_node;
    int gap;
    int freq;
    int cycle_time;
};

class Network {
private:
    int num_nodes;
    int* stop_cost;
    vector<Flight> flights;
    unordered_map<char, vector<Node*>> nodes;
    int** arc_cost;
    int** time_cost;
public:

    Network();
    void add_nodes();
    void add_edges();
    bool add_edge(Node* start, Node* end, int cost);

    void read_data(std::string data_path);
    void read_node(std::string node_data_path);
    void read_stop_cost(std::string cost_data_path);
    void read_flights_param(std::string flights_data);
    void read_time_cost(std::string time_data_path);
};


#endif //TLM_THESIS_NETWORK_H
