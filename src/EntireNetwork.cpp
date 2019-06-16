//
// Created by Ashee on 2019/6/15.
//

#include "EntireNetwork.h"

EntireNetwork::EntireNetwork(string data) {
    cout << "===========AIR===========" << endl;
    air_network = AirNetwork("../Data/" + data + "_air");
    cout << "===========SEA===========" << endl;
    sea_network = SeaNetwork("../Data/" + data + "_sea");
//    read_data(data);
//    print_all_arcs();
}

void EntireNetwork::read_data(string data) {
    // 5 layers of time space network
    add_designed_ships(data);
    add_designed_flights(data);
    add_current_ships(data);
    add_current_flights(data);
    add_virtual_network(data);

}

void EntireNetwork::add_designed_ships(string data) {
    num_nodes = sea_network.getNum_nodes();
    int layer = 0;
    nodes[layer] = vector<vector<Node*>>(num_nodes); //first time space network layer

    //add nodes
    int* node_cost = sea_network.getStop_cost();
    for(int i = 0; i < num_nodes; i++){
        nodes[layer][i] = vector<Node*>(TOTAL_TIME_SLOT);
        for(int t = 0 ; t < TOTAL_TIME_SLOT; t++){
            nodes[layer][i][t] = new Node(to_string(layer) + (char) (65 + i) + to_string(t), node_cost[i]);
        }
    }

    //add routes arc
    int** arc_cost = sea_network.getArc_cost();
    for(const auto &ship : sea_network.getShips()) {
        Route route = ship.route;
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
    int layer = 1;
    nodes[layer] = vector<vector<Node*>>(num_nodes);

    //add nodes
    int* node_cost = air_network.getStop_cost();
    for(int i = 0; i < num_nodes; i++){
        nodes[layer][i] = vector<Node*>(TOTAL_TIME_SLOT);
        for(int t = 0 ; t < TOTAL_TIME_SLOT; t++){
            nodes[layer][i][t] = new Node(to_string(layer) + (char) (65 + i) + to_string(t), node_cost[i]);
        }
    }

    int** arc_cost = air_network.getArc_cost();

    for(const auto &flight : air_network.getFlights()){
        for(int week = 0 ; week < TIME_PERIOD / 7; week++) {
            for(const auto &route : flight.routes){
                for(int i = 0; i < route.nodes.size()-1; i++) {
                    char start_node_char = route.nodes[i][0];
                    int start_node_time = week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i].substr(1));
                    char end_node_char = route.nodes[i + 1][0];
                    int end_node_time = week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i + 1].substr(1));

                    Node *start_node = nodes[layer][(int) start_node_char - 65][start_node_time];
                    Node *end_node = nodes[layer][(int) end_node_char - 65][end_node_time];

                    Arc *arc = new Arc(start_node, end_node,
                                       arc_cost[(int) end_node_char - 65][(int) end_node_char - 65], flight.weight_ub,
                                       flight.volume_ub);

                    arcs.push_back(arc);
                    start_node->out_arcs.push_back(arc);
                    end_node->in_arcs.push_back(arc);
                }
            }
        }

    }

}

void EntireNetwork::add_virtual_network(string data) {
    int layer = 2;
    nodes[layer] = vector<vector<Node*>>(num_nodes);

    //add nodes
    int* virtual_node_cost = read_stop_cost("../Data/" + data +"_virtual.txt");
    for(int i = 0; i < num_nodes; i++){
        nodes[layer][i] = vector<Node*>(TOTAL_TIME_SLOT);
        for(int t = 0 ; t < TOTAL_TIME_SLOT; t++){
            nodes[layer][i][t] = new Node(to_string(layer) + (char) (65 + i) + to_string(t), virtual_node_cost[i]);
        }
    }

    //add horizontal arcs
    for(int i = 0; i < num_nodes; i++){
        for(int t = 0; t < TOTAL_TIME_SLOT - 1; t++){
            Arc *arc = new Arc(nodes[layer][i][t], nodes[layer][i][t+1], FIX_COST_OF_VIRTUAL_ARC);

            arcs.push_back(arc);
            nodes[layer][i][t]->out_arcs.push_back(arc);
            nodes[layer][i][t+1]->in_arcs.push_back(arc);
        }
    }

    //add vertical arcs(only from ship to flight)
    for(int i = 0; i < num_nodes; i++){
        for(int t = 0; t < TOTAL_TIME_SLOT-1; t++){
            //from design ship to virtual
            Arc *arc = new Arc(nodes[0][i][t], nodes[layer][i][t+1], 0);
            arcs.push_back(arc);
            nodes[0][i][t]->out_arcs.push_back(arc);
            nodes[layer][i][t+1]->in_arcs.push_back(arc);
            //from virtual to design flight
            arc = new Arc(nodes[layer][i][t], nodes[1][i][t], 0);
            arcs.push_back(arc);
            nodes[layer][i][t]->out_arcs.push_back(arc);
            nodes[1][i][t]->in_arcs.push_back(arc);
            //from current ship to virtual
            arc = new Arc(nodes[3][i][t], nodes[layer][i][t+1], 0);
            arcs.push_back(arc);
            nodes[3][i][t]->out_arcs.push_back(arc);
            nodes[layer][i][t+1]->in_arcs.push_back(arc);
            //from current flight to virtual
            arc = new Arc(nodes[layer][i][t], nodes[4][i][t], 0);
            arcs.push_back(arc);
            nodes[layer][i][t]->out_arcs.push_back(arc);
            nodes[4][i][t]->in_arcs.push_back(arc);
        }
    }


}

int* EntireNetwork::read_stop_cost(const string cost_data_path) {
    fstream file;
    file.open(cost_data_path);
    int *stop_cost = new int[num_nodes];

    if(file.is_open()){
        string line;
        getline(file, line);
        istringstream iss(line);
        string token;
        for(int i = 0; getline(iss, token, '\t'); i++) {
            stop_cost[i] = stoi(token);
        }
        return stop_cost;
    }
    else {
        cout << "stop_cost file cannot open !!!" << endl;
        return nullptr;
    }
}

void EntireNetwork::add_current_ships(string data) {
    int layer = 3;
    nodes[layer] = vector<vector<Node*>>(num_nodes); //first time space network layer

    //add nodes
    int* node_cost = sea_network.getStop_cost();
    for(int i = 0; i < num_nodes; i++){
        nodes[layer][i] = vector<Node*>(TOTAL_TIME_SLOT);
        for(int t = 0 ; t < TOTAL_TIME_SLOT; t++){
            nodes[layer][i][t] = new Node(to_string(layer) + (char) (65 + i) + to_string(t), node_cost[i]);
        }
    }

    int** arc_cost = sea_network.getArc_cost();
    for(const auto &ship : sea_network.getCur_ships()) {
        Route route = ship.route;
        for(int i = 0 ; i < route.nodes.size() -1; i++){
            char start_node_char = route.nodes[i][0];
            int start_node_time = stoi(route.nodes[i].substr(1));
            char end_node_char = route.nodes[i+1][0];
            int end_node_time = stoi(route.nodes[i+1].substr(1));

            Node* start_node = nodes[layer][(int) start_node_char -65][start_node_time];
            Node* end_node = nodes[layer][(int) end_node_char -65][end_node_time];

            Arc* arc = new Arc(start_node, end_node, arc_cost[(int) end_node_char -65][(int) end_node_char -65], ship.weight_ub);

            arcs.push_back(arc);
            start_node->out_arcs.push_back(arc);
            end_node->in_arcs.push_back(arc);
        }
    }
}

void EntireNetwork::add_current_flights(string data) {
    int layer = 4;
    nodes[layer] = vector<vector<Node*>>(num_nodes);

    //add nodes
    int* node_cost = air_network.getStop_cost();
    for(int i = 0; i < num_nodes; i++){
        nodes[layer][i] = vector<Node*>(TOTAL_TIME_SLOT);
        for(int t = 0 ; t < TOTAL_TIME_SLOT; t++){
            nodes[layer][i][t] = new Node(to_string(layer) + (char) (65 + i) + to_string(t), node_cost[i]);
        }
    }

    //add arcs
    int** arc_cost = air_network.getArc_cost();

    for(const auto &flight : air_network.getCur_flights()){
        for(int week = 0 ; week < TIME_PERIOD / 7; week++) {
            for(const auto &route : flight.routes){
                for(int i = 0; i < route.nodes.size()-1; i++) {
                    char start_node_char = route.nodes[i][0];
                    int start_node_time = week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i].substr(1));
                    char end_node_char = route.nodes[i + 1][0];
                    int end_node_time = week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i + 1].substr(1));

                    Node *start_node = nodes[layer][(int) start_node_char - 65][start_node_time];
                    Node *end_node = nodes[layer][(int) end_node_char - 65][end_node_time];

                    Arc *arc = new Arc(start_node, end_node,
                                       arc_cost[(int) end_node_char - 65][(int) end_node_char - 65], flight.weight_ub,
                                       flight.volume_ub);

                    arcs.push_back(arc);
                    start_node->out_arcs.push_back(arc);
                    end_node->in_arcs.push_back(arc);
                }
            }
        }

    }
}

void EntireNetwork::print_all_arcs() {
    for(auto* arc : arcs){
        cout << arc->start_node->getName() << "->" << arc->end_node->getName() << endl;
    }
}