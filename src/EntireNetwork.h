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

    unsigned int num_nodes;
    vector<Arc*> arcs;
    vector<vector<Node*>>* nodes = new vector<vector<Node*>>[5]; //total 5 time space network
};


#endif //TLM_THESIS_ENTIRENETWORK_H
