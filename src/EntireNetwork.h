//
// Created by Ashee on 2019/6/15.
//

#ifndef TLM_THESIS_ENTIRENETWORK_H
#define TLM_THESIS_ENTIRENETWORK_H

#include "param.h"
#include "Node.h"
#include "Network.h"
#include "SeaNetwork.h"
#include "AirNetwork.h"

using namespace std;

class EntireNetwork {
public:
    explicit EntireNetwork(string data);
private :
    void read_data(string data);
    void add_designed_ships(string data);
    void add_designed_flights(string data);
    void add_virtual_network(string data);
    void add_current_ships(string data);
    void add_current_flights(string data);
    int* read_stop_cost(string data_path);
    void add_arc(Node* out, Node* in, int cost);
    void find_all_paths();
    void find_paths_from_single_node(Path path, Point point, int*** color);
    int*** create_3d_array(int x, int y, int z);

    void print_all_arcs();

    AirNetwork air_network;
    SeaNetwork sea_network;
    vector<Path> all_paths;
    unsigned int num_nodes;
    vector<Arc*> arcs;
    vector<vector<Node*>>* nodes = new vector<vector<Node*>>[5]; //total 5 time space network
};


#endif //TLM_THESIS_ENTIRENETWORK_H
