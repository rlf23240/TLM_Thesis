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
    EntireNetwork();
    void rebuild_networks();
    vector<Path *> **getPaths_categories() const;
    unsigned int getNumNodes() const;
    Node* getNode(int layer, int node, int time);
    int get_node_idx(int layer, int node, int time);
    int get_node_idx(Point point);
    Point idx_to_point(int idx);
    const unordered_map<int, unordered_map<int, Arc *>> &getArcs() const;
    vector<Flight> get_cur_flights();
    vector<Ship> get_cur_ships();
    AirNetwork &getAir_network();
    SeaNetwork &getSea_network();
    void setAir_network(const AirNetwork &air_network);
    void setSea_network(const SeaNetwork &sea_network);
    unordered_map<int, unordered_map<int, Arc*>> arcs;

private :
    void create_networks(string data);
    void read_param_data(string data);
    void add_designed_ships();
    void add_designed_flights();
    void add_virtual_network(string data);
    void add_current_ships();
    void add_current_flights();
    void add_rival_ships();
    void add_rival_flights();
    int* read_stop_cost(string data_path);
    void add_arc(Node* out, Node* in, Arc* arc);
    void find_all_paths();
    void find_paths_from_single_node(Path path, Point point, int*** color);
    void add_path(Path* path);
    int*** create_3d_array(int x, int y, int z);

    string data_str;
    AirNetwork air_network;
    SeaNetwork sea_network;
    vector<Path*> all_paths;
    vector<Path*>** paths_categories;
    int num_cur_flights;
    int num_cur_ships;
    unsigned int num_nodes;
    int num_layers = 7;
    vector<vector<Node*>>* nodes = new vector<vector<Node*>>[num_layers]; //total 7 time space network
};


#endif //TLM_THESIS_ENTIRENETWORK_H
