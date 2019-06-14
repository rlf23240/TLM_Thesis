#include <utility>

//
// Created by Ashee on 2019/6/4.
//

#include "SeaNetwork.h"

Ship::Ship(char start_node, int start_time, int frequency, int cycle_time) : start_node(start_node),
                                                                             start_time(start_time),
                                                                             frequency(frequency),
                                                                             cycle_time(cycle_time) {}

SeaNetwork::SeaNetwork(string data_path) {
    read_data(data_path);
    run_algo();
}

void SeaNetwork::read_data(std::string data_path) {
    Network::read_node(data_path + "_arccost.txt");
    Network::read_stop_cost(data_path + "_stopcost.txt");
    Network::read_time_cost(data_path + "_timecost.txt");
    Network::add_nodes();
    Network::add_edges();
    read_ship_param(data_path+"_ships_param.txt");
}

void SeaNetwork::read_ship_param(string ships_data) {
    fstream file;
    file.open(ships_data);


    if(file.is_open()){
        string line;
        getline(file, line);
        int num_ships = stoi(line);

        for(int i = 0; i < num_ships ; i++){
            getline(file, line);
            istringstream iss(line);
            string token;

            getline(iss, token, '\t');
            char stating_node = token[0];

            getline(iss, token, '\t');
            int starting_time = stoi(token);

            getline(iss, token, '\t');
            int ships_freq = stoi(token);

            getline(iss, token, '\t');
            int ships_cycle_time = stoi(token);

            Ship new_ship = Ship(stating_node, starting_time, ships_freq, ships_cycle_time);
            ships.push_back(new_ship);
        }
    }
    else {
        cout << "ships data cannot open !!!";
    }
}

void SeaNetwork::run_algo() {
    for(auto ship : ships) {
        Route route = DP_shortest_path(ship.start_node, ship.start_time, ship.start_node, ship.start_time+ship.cycle_time);
        cout << route;
    }
}

void SeaNetwork::forward_update(Route **dp, int node, int time) {
    char node_char = (char) ('A' + node) ;
    Node* cur_node = nodes[node_char][time];

    for (auto& arc : cur_node->out_arcs){
        Node* end_node = arc->end_node;
        char end_node_char = end_node->getName()[0];
        int end_node_idx = (int) end_node_char - 65;

        int additional_stay_days = (node_char==end_node_char)? 0 : SHIP_STOP_DAY;

        int end_time = stoi(end_node->getName().substr(1)) + additional_stay_days;

        //Calculate cost if append end node to current route
        Route cur_route = dp[node][time];
        Route end_route = dp[end_node_idx][end_time];
        int new_cost = cur_route.cost + arc->cost + end_node->getCost() * (1+additional_stay_days);

        // if yes, replace old route.
        if (new_cost < end_route.cost){
            vector<string> new_nodes;
            new_nodes.assign(cur_route.nodes.begin(), cur_route.nodes.end());

            if(additional_stay_days != 0)
                new_nodes.push_back(end_node_char + to_string(end_time - additional_stay_days));
            new_nodes.push_back(end_node_char + to_string(end_time));
            //cout << end_node_idx << " " << end_time <<endl;

            dp[end_node_idx][end_time] = Route(new_nodes, new_cost);
        }
    }
}



