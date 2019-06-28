#include <utility>

//
// Created by Ashee on 2019/6/4.
//

#include "SeaNetwork.h"


Ship::Ship(char start_node, int start_time, int frequency, int cycle_time, int weight_ub) : start_node(start_node),
                                                                                            start_time(start_time),
                                                                                            frequency(frequency),
                                                                                            cycle_time(cycle_time),
                                                                                            weight_ub(weight_ub) {}
SeaNetwork::SeaNetwork() {}
SeaNetwork::SeaNetwork(string data_path, int num_cur_ships, bool is_target) {
    this->is_target = is_target;
    read_data(data_path);
    if(is_target)
        run_algo();
    if(!is_target)
        ships.clear();
    generate_cur_ships(num_cur_ships);

    print_ships(ships,true,is_target);
    print_ships(cur_ships,false,is_target);


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

            getline(iss, token, '\t');
            int weight_ub = stoi(token);

            Ship new_ship = Ship(stating_node, starting_time, ships_freq, ships_cycle_time, weight_ub);
            ships.push_back(new_ship);
        }
    }
    else {
        cout << "ships data cannot open !!!";
    }
}

void SeaNetwork::run_algo() {
    for(auto &ship : ships) {
        Route route = DP_shortest_path(ship.start_node, ship.start_time, ship.start_node, ship.start_time+ship.cycle_time);
        ship.route = route;
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

        try {
            //Calculate cost if append end node to current route
            Route cur_route = dp[node][time];
            Route end_route = dp[end_node_idx][end_time];
            int new_cost = cur_route.cost + arc->cost + end_node->getCost() * (1 + additional_stay_days);

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
        catch(bad_alloc& e){
//            cout << e.what() << " " << end_node_idx << endl;
        }
    }
}

const vector<Ship> &SeaNetwork::getShips() const {
    return ships;
}

void SeaNetwork::generate_cur_ships(int n) {
    random_device rd;
    mt19937 gen = is_target? mt19937(rd()) : mt19937(0);
    uniform_int_distribution<int> dis(0, INT_MAX);

    for (int i = 0; i < n; i++) {
        int start_node = dis(gen) % num_nodes;
        int start_time = dis(gen) % 10;
        int cur_node = start_node;
        int cur_time = start_time;
        int next_node, next_time;
        int total_cost = 0;
        vector<string> nodes;
        nodes.push_back((char) (65 + start_node) + to_string(start_time));
        total_cost = stop_cost[start_node];
        while (cur_time - start_time < 35) {
            do {
                next_node = dis(gen) % num_nodes;
            } while (cur_node == next_node);


            next_time = cur_time + time_cost[cur_node][next_node];
            total_cost += stop_cost[next_node] * (1+SHIP_STOP_DAY);
            total_cost += arc_cost[cur_node][next_node];

            nodes.push_back((char) (65 + next_node) + to_string(next_time));
            nodes.push_back((char) (65 + next_node) + to_string(next_time + SHIP_STOP_DAY));

            cur_node = next_node;
            cur_time = next_time;
        }
        if(cur_node != start_node){
            next_node = start_node;
            next_time = cur_time + time_cost[cur_node][next_node];
            total_cost += stop_cost[next_node];
            total_cost += arc_cost[cur_node][next_node];
            nodes.push_back((char) (65 + next_node) + to_string(next_time));
        }

        Route route = Route(nodes, total_cost);
        Ship new_ship = Ship((char) (65 + start_node), start_time, 1, cur_time - start_time, (30 + dis(gen) % 20) * 100);
        new_ship.route = route;
        cur_ships.push_back(new_ship);
    }
}

void SeaNetwork::print_ships(vector<Ship> ships, bool is_designed, bool is_target) {

    cout << "---------------";
    if(is_target){
        cout << "Target ";
    }
    else {
        cout << "Rival ";
    }

    if (is_designed){
        cout << "Designed ships routes-----------" <<endl;
    }
    else{
        cout << "Exist ships routes-----------" << endl;
    }
    for(const auto& ship : ships){
        cout << ship.route;
    }

}

const vector<Ship> &SeaNetwork::getCur_ships() const {
    return cur_ships;
}




