#include <utility>

//
// Created by Ashee on 2019/6/4.
//

#include "SeaNetwork.h"
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

        Route route = Route(nodes, total_cost);
        Ship new_ship = Ship(start_node[0], stoi(start_node.substr(1)),1, (stoi(cur_node.substr(1)) - stoi(start_node.substr(1))), volume_ub);
        new_ship.route = route;
        ships.push_back(new_ship);
    }
}

void SeaNetwork::run_algo() {
    for(auto &ship : designed_ships) {
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
        catch(bad_alloc& e){
//            cout << e.what() << " " << end_node_idx << endl;
        }
    }
}

void SeaNetwork::generate_designed_ship() {
//    unsigned seed = static_cast<unsigned int>(chrono::system_clock::now().time_since_epoch().count());
    mt19937 gen =  mt19937(sea_seed++);
    uniform_int_distribution<int> dis(0, INT_MAX);
    Ship cur_ship = designed_ships[0];

    int start_node = (int) cur_ship.route.nodes[0][0] - 65;
    int start_time = cur_ship.start_time;
    int cur_node = start_node;
    int cur_time = start_time;
    int next_node, next_time;
    int total_cost = 0;
    vector<string> nodes;
    nodes.push_back((char) (65 + start_node) + to_string(start_time));
    total_cost = stop_cost[start_node];
    while (cur_time - start_time < cur_ship.cycle_time - 5) {
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
        if(next_time < TOTAL_TIME_SLOT) {
            total_cost += stop_cost[next_node];
            total_cost += arc_cost[cur_node][next_node];
            nodes.push_back((char) (65 + next_node) + to_string(next_time));
        }
    }


    Route route = Route(nodes, total_cost);
    Ship new_ship = Ship((char) (65 + start_node), start_time, 1, cur_ship.cycle_time, cur_ship.volume_ub);
    new_ship.route = route;
    designed_ships[0] = new_ship;

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

vector<Route *> SeaNetwork::find_all_routes() {
    vector<Route*> all_routes;

    for(auto &ship : designed_ships) {
        vector<Route*> routes;
        routes = find_routes_from_single_node(ship.start_node, ship.start_time, ship.start_node, ship.start_time+ship.cycle_time);
        all_routes.insert(all_routes.end(), routes.begin(), routes.end());
    }
    return all_routes;
}


vector<Route *> SeaNetwork::find_routes_from_single_node(char start_node, int start_time, char end_node, int end_time) {
    
    cout << "=======================SeaNetwork::find_routes_from_single_node=======================" << endl;
    
    // Internal data use to record travesal state.
    struct NodeTraversalData {
        string node;
        vector<Arc*> arcs;
        int cost;
        
        // Consider stay in same point. Different from AirNetwork.
        bool wait = false;
    
        NodeTraversalData(string node, int cost, vector<Arc*> arcs): node(node), cost(cost), arcs(arcs) {}
    };
    
    // TODO: Very Important! Check this algorithm is vaild!!
    vector<Route*> routes = vector<Route*>();
    
    int start_node_idx = (int) start_node - 'A';
    int end_node_idx = (int) end_node - 'A';
    
    // Use stack to record passed node.
    vector<NodeTraversalData*> stack {new NodeTraversalData(start_node + to_string(start_time), stop_cost[start_node_idx], nodes[start_node][start_time]->out_arcs)};
    
    while (stack.empty() == false) {
        string node_str = stack.back()->node;
        // If arcs are all visited and self-loop also considered, then pop back and find next node.
        if (stack.back()->arcs.empty()) {
            int next_time = stoi(node_str.substr(1)) + 1;
            if (stack.back()->wait == true || next_time > end_time) {
                delete stack.back();
                stack.pop_back();
            } else if (next_time <= end_time) {
                auto data = stack.back();
                
                int cost = data->cost;
                char node_char = node_str[0];
                
                Node *node = nodes[node_char][next_time];
                if (node->getNode() != node_char - 'A') {
                    cout<<node->getNode() << endl;
                    cout<<"miss match!"<<endl;
                }
                stack.push_back(new NodeTraversalData(
                    node->getName(),
                    cost,
                    node->out_arcs
                ));
            }
            stack.back()->wait = true;
        } else {
            Arc *arc = stack.back()->arcs.back();
            
            Node *next_node = arc->end_node;
            string next_node_str = next_node->getName();
                
            int additional_stay_days = (next_node->getNode() == end_node_idx) ? 0 : SHIP_STOP_DAY;
            
            int next_time = next_node->getTime() + additional_stay_days;
            
            // TODO: Very Important! Check the cost is vaild!
            double cost = stack.back()->cost + arc->cost + next_node->getCost() * (1 + additional_stay_days);
            
            // If node is feasible...
            if (next_time <= end_time) {
                // If we reach the goal...
                if (next_node->getNode() == end_node_idx) {
                    vector<string> new_nodes = vector<string>();
                    for (auto& data : stack) {
                        new_nodes.push_back(data->node);
                    }
                    new_nodes.push_back(next_node_str);
                    
                    // TODO: Very Important! Air didn't have this one?
                    if (additional_stay_days != 0)
                        new_nodes.push_back(to_string(next_node->getNode()) + to_string(end_time));
                    
                    Route *route = new Route(new_nodes, cost);
                    routes.push_back(route);
                    
                    cout << *route << endl;
                } else {
                    stack.push_back(new NodeTraversalData(next_node_str, cost, next_node->out_arcs));
                }
            }
        }
    }

    return routes;
}

void SeaNetwork::set_designed_ship(Route route) {
    designed_ships[0].route = route;
}

const vector<Ship> &SeaNetwork::getDesignedShips() const {
    return designed_ships;
}






