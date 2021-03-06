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
    Route(const vector<string> &nodes, double cost);
    Route(Route route, int gap);
    Route();
    friend ostream &operator<<(ostream &os, const Route &route);
    vector<string> nodes;
    double cost = INT_MAX;
};

class Network {
protected:
    unsigned int num_nodes;
    int* stop_cost;
    int** arc_cost;
    int** time_cost;

public:
    virtual void add_nodes();
    virtual void run_algo() = 0;
    virtual void read_data(std::string data_path);
    virtual void read_node(std::string node_data_path);
    virtual void forward_update(Route** dp, int node, int time);
    void add_edges();
    bool add_edge(Node* start, Node* end, int cost);
    void read_stop_cost(std::string cost_data_path);
    void read_time_cost(std::string time_data_path);
    Route DP_shortest_path(char start_node, int start_time, char end_node, int end_time);
    unordered_map<char, vector<Node*>> nodes;

    unsigned  getNum_nodes() const;
    int *getStop_cost() const;
    int **getArc_cost() const;

    static string num_to_excel_like_alpha(int num);
    static int excel_alpha_to_num(string str);
    static char excel_alpha_to_char(string str);
    static string parse_node(string token);
    static string parse_node_backward(string token);

};


#endif //TLM_THESIS_NETWORK_H
