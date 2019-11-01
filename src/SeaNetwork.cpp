#include <utility>

//
// Created by Ashee on 2019/6/4.
//

#include "SeaNetwork.h"
#include "Dijkstra.h"

unsigned sea_seed = 0;

Ship::Ship(char start_node, int start_time, int frequency, int cycle_time, int volume_ub) : start_node(start_node),
                                                                                            start_time(start_time),
                                                                                            frequency(frequency),
                                                                                            cycle_time(cycle_time),
                                                                                            volume_ub(volume_ub) {}
SeaNetwork::SeaNetwork() {}

SeaNetwork::SeaNetwork(string data_path, int num_cur_ships, int num_rival_ships) {
    read_data(data_path);
    run_algo();

    print_ships(designed_ships,"Designed");
    print_ships(cur_ships,"Existing");
    print_ships(rival_ships,"Rival");
}

void SeaNetwork::read_data(std::string data_path) {
    Network::read_node(data_path + "_arccost.txt");
    Network::read_stop_cost(data_path + "_stopcost.txt");
    Network::read_time_cost(data_path + "_timecost.txt");
    Network::add_nodes();
    Network::add_edges();
    read_ship_param(data_path+"_ships_param.txt");
    read_sea_routes(data_path + "_target_routes.csv", cur_ships);
    read_sea_routes(data_path + "_rival_routes.csv", rival_ships);
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

            getline(iss, token, '\t');
            int weight_ub = stoi(token);

            Ship new_ship = Ship(stating_node, starting_time, ships_freq, ships_cycle_time, weight_ub);
            designed_ships.push_back(new_ship);
        }
    }
    else {
        cout << "ships data cannot open !!!";
    }
}

void SeaNetwork::read_sea_routes(string data_path, vector<Ship> &ships) {
    fstream file;
    file.open(data_path);

    string line;
    getline(file, line);
    int n = stoi(line);

    for(int i = 0 ; i < n; i++){
        getline(file, line);
        istringstream iss(line);
        string token;
        getline(iss, token, ',');
        int volume_ub = stoi(token);

        vector<string> nodes;
        string start_node,cur_node, next_node;
        int total_cost = 0;
        getline(iss, token, ',');
        start_node = token;
        cur_node = start_node;
        total_cost += stop_cost[(int) cur_node[0] -65];
        nodes.push_back(cur_node);
        while(getline(iss, token, ',')){
            if (token != "") {
                nodes.push_back(token);
                next_node = token;
                if(cur_node[0] == next_node[0]){
                    total_cost += stop_cost[(int) cur_node[0] -65] * (stoi(next_node.substr(1)) - stoi(cur_node.substr(1)));
                }else{
                    total_cost += arc_cost[(int) cur_node[0] -65][(int) next_node[0] -65];
                    total_cost += stop_cost[(int) next_node[0] -65];
                }
                cur_node = next_node;
            }
        }

        Route route = Route(nodes, total_cost);
        Ship new_ship = Ship(start_node[0], stoi(start_node.substr(1)),1, (stoi(cur_node.substr(1)) - stoi(start_node.substr(1))), volume_ub);
        new_ship.route = route;
        ships.push_back(new_ship);
    }
}

void SeaNetwork::run_algo() {
    for(auto &ship : designed_ships) {
        Route route = shortest_route(ship.start_node, ship.start_time, ship.start_node, ship.start_time+ship.cycle_time);
        ship.route = route;
    }
}

Route SeaNetwork::shortest_route(char start_node_ch, int start_time, char end_node_ch, int end_time) {
    Route **dp = new Route *[num_nodes];
    for (int i = 0; i < num_nodes; i++)
        dp[i] = new Route[TOTAL_TIME_SLOT];


    int start_node_idx = (int) start_node_ch - 'A';
    int end_node_idx = (int) end_node_ch - 'A';
    
    Node* start_node = nodes[start_node_ch][start_time];
    // Ship should stay on harbor at least 1 turn before the departure.
    Node* additional_stay_node = nodes[start_node_ch][start_time+1];
    
    Arc* additional_stay_arc = start_node->arc_to(additional_stay_node);

    vector<string> init_node = vector<string>();
    init_node.push_back(start_node_ch + to_string(start_time));
    init_node.push_back(start_node_ch + to_string(start_time+1));
    
    int init_cost = 2*stop_cost[start_node_idx] + additional_stay_arc->cost + additional_stay_arc->fixed_cost;

    dp[start_node_idx][start_time] = Route(init_node, init_cost);
    forward_update(dp, start_node_idx, start_time);

    for (int t = start_time + 1; t < end_time; t++) {
        for (int node = 0; node < num_nodes; node++) {
            if (dp[node][t].nodes.empty() == 0)
                forward_update(dp, node, t);
        }
    }
        
    // Release useless memories.
    Route result = dp[end_node_idx][end_time];
    for (int i = 0; i < num_nodes; ++i) {
        delete[] dp[i];
    }
    delete[] dp;

    return result;
}

void SeaNetwork::forward_update(Route **dp, int node, int time) {
    char node_char = (char) ('A' + node) ;
    Node* cur_node = nodes[node_char][time];

    for (auto& arc: cur_node->out_arcs){
        Node* end_node = arc->end_node;
        char end_node_char = end_node->getName()[0];
        int end_node_idx = (int) end_node_char - 65;

        int additional_stay_days = (node_char==end_node_char)? 0 : SHIP_STOP_DAY;

        int end_time = stoi(end_node->getName().substr(1)) + additional_stay_days;

        try {
            if (end_time < TOTAL_TIME_SLOT) {
                //Calculate cost if append end node to current route
                Route cur_route = dp[node][time];
                Route end_route = dp[end_node_idx][end_time];
                double new_cost = cur_route.cost + arc->cost + arc->fixed_cost + end_node->getCost() * (1 + additional_stay_days);
                new_cost = MAX(0, new_cost);

                // if yes, replace old route.
                if (new_cost < end_route.cost) {
                    vector<string> new_nodes;
                    new_nodes.assign(cur_route.nodes.begin(), cur_route.nodes.end());

                    if (additional_stay_days != 0)
                        new_nodes.push_back(end_node_char + to_string(end_time - additional_stay_days));
                    new_nodes.push_back(end_node_char + to_string(end_time));
    //            //cout << end_node_idx << " " << end_time <<endl;
    //
                    dp[end_node_idx][end_time] = Route(new_nodes, new_cost);

                }
            }
        }
        catch(bad_alloc& e){
//            cout << e.what() << " " << end_node_idx << endl;
        }
    }
}

void SeaNetwork::print_ships(vector<Ship> designed_ships, string prefix) {
    cout << "---------------" + prefix ;
    cout << " ships routes-----------" <<endl;

    for(const auto& ship : designed_ships){
        cout << ship.route;
    }

}

const vector<Ship> &SeaNetwork::getShips() const {
    return designed_ships;
}

const vector<Ship> &SeaNetwork::getCur_ships() const {
    return cur_ships;
}

const vector<Ship> &SeaNetwork::getRival_ships() const {
    return rival_ships;
}

void SeaNetwork::set_designed_ship(Route route) {
    designed_ships[0].route = route;
}

const vector<Ship> &SeaNetwork::getDesignedShips() const {
    return designed_ships;
}






