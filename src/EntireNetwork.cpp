//
// Created by Ashee on 2019/6/15.
//

#include "EntireNetwork.h"

EntireNetwork::EntireNetwork(string data) {
    read_data(data);

}

void EntireNetwork::read_data(string data) {
    cout << "===========SEA===========" << endl;
    add_designed_ships(data);
    cout << "===========AIR===========" << endl;
    add_designed_flights(data);
}

void EntireNetwork::add_designed_ships(string data) {
    SeaNetwork sea_network = SeaNetwork("../Data/sea" + data);

    num_nodes = sea_network.getNum_nodes();
    nodes[0] = vector<vector<Node*>>(num_nodes); //first time space network layer

    //add nodes
    int* node_cost = sea_network.getStop_cost();
    for(int i = 0; i < num_nodes; i++){
        nodes[0][i] = vector<Node*>(TOTAL_TIME_SLOT);
        for(int t = 0 ; t < TOTAL_TIME_SLOT; t++){
            nodes[0][i][t] = new Node(to_string(0) + (char) (65 + i) + to_string(t), node_cost[i]);
        }
    }

//    //add routes arc
    int** arc_cost = sea_network.getArc_cost();
    for(const auto &ship : sea_network.getShips()) {
        Route route = ship.route;
        cout << route;
        for(int i = 0 ; i < route.nodes.size() -1; i++){
            char start_node_char = route.nodes[i][0];
            int start_node_time = stoi(route.nodes[i].substr(1));
            char end_node_char = route.nodes[i+1][0];
            int end_node_time = stoi(route.nodes[i+1].substr(1));

            Node* start_node = nodes[0][(int) start_node_char -65][start_node_time];
            Node* end_node = nodes[0][(int) end_node_char -65][end_node_time];

            Arc* arc = new Arc(start_node, end_node, arc_cost[(int) end_node_char -65][(int) end_node_char -65], ship.weight_ub);

            arcs.push_back(arc);
            start_node->out_arcs.push_back(arc);
            end_node->in_arcs.push_back(arc);
        }
    }


}

void EntireNetwork::add_designed_flights(string data) {
    AirNetwork air_network = AirNetwork("../Data/air" + data);

    for(const auto &flight : air_network.getFlights()){
        vector<Route> routes = flight.routes;
    }

}
