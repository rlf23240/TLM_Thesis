//
// Created by Ashee on 2019/6/4.
//

#include "AirNetwork.h"


/*-----------------Flight struct------------------------------*/
unsigned int air_seed = 0;

ostream &operator<<(ostream &os, const Flight &flight) {
    os << "start_node: " << flight.start_node << " gap: " << flight.gap << " freq: " << flight.freq << " cycle_time: "
       << flight.cycle_time;
    return os;
}

Flight::Flight(char start_node, int gap, int freq, int cycle_time, int volume_ub, int weight_ub) : start_node(
        start_node), gap(gap), freq(freq), cycle_time(cycle_time), volume_ub(volume_ub), weight_ub(weight_ub) {}

AirNetwork::AirNetwork() {}

AirNetwork::AirNetwork(const string data_path, int num_cur_flights, int num_rival_flights) {
    read_data(data_path);
    run_algo();

    print_flights(designed_flights, "Designed");
    print_flights(cur_flights, "Existing");
    print_flights(rival_flights, "Rival");
}

void AirNetwork::read_data(std::string data_path) {
    Network::read_data(data_path);
    read_flights_param(data_path+"_flights_param.txt");
    read_air_routes(data_path + "_target_routes.csv", cur_flights);
    read_air_routes(data_path + "_rival_routes.csv", rival_flights);
}

void AirNetwork::read_flights_param(std::string flights_data) {

    fstream file;
    file.open(flights_data);


    if(file.is_open()){
        string line;
        getline(file, line);
        int num_flights = stoi(line);

        for(int i = 0; i < num_flights ; i++){
            getline(file, line);
            istringstream iss(line);
            string token;

            getline(iss, token, '\t');
            char flight_name = token[0];

            getline(iss, token, '\t');
            int flight_gap = stoi(token);

            getline(iss, token, '\t');
            int flight_freq = stoi(token);

            getline(iss, token, '\t');
            int flight_cycle_time = stoi(token);

            getline(iss, token, '\t');
            int weight_ub = stoi(token);

            getline(iss, token, '\t');
            int volume_ub = stoi(token);

            Flight new_flight = Flight(flight_name, flight_gap, flight_freq, flight_cycle_time, volume_ub, weight_ub);
            designed_flights.push_back(new_flight);
        }
    }
    else {
        cout << "flights data cannot open !!!";
    }
}

void AirNetwork::read_air_routes(string data_path, vector<Flight> &flights){
    fstream file;
    file.open(data_path);

    string line;
    getline(file, line);
    int n = stoi(line);

    for(int i = 0; i < n; i++){
        getline(file,line);
        istringstream iss(line);
        string token;
        getline(iss, token, ',');
        int volume_ub = stoi(token);
        getline(iss, token, ',');
        int weight_ub = stoi(token);

        vector<string> nodes;
        vector<Route> routes;
        int total_cost = 0;
        string start_node, cur_node, next_node;
        getline(iss, token, ',');
        start_node = token;
        cur_node = start_node;
        nodes.push_back(cur_node);
        total_cost += stop_cost[(int) cur_node[0] - 65];
    
        int cycle_time = 0;
        while(getline(iss, token, ',')){
            if(token == ""){
                if (nodes.size() != 0) {
                    Route route = Route(nodes, total_cost);
                    routes.push_back(route);
                    
                    cycle_time = stoi(nodes.back().substr(1)) - stoi(nodes[0].substr(1));
                    nodes.clear();
                }
    
                total_cost = stop_cost[(int) cur_node[0] - 65];
            }else{
                nodes.push_back(token);
                next_node = token;
                if(cur_node[0] != next_node[0]) {
                    total_cost += arc_cost[(int) cur_node[0] - 65][(int) next_node[0] - 65];
                    total_cost += stop_cost[(int) next_node[0] - 65];
                }

            }
            cur_node = next_node;
        }

        if (nodes.size() != 0) {
            Route route = Route(nodes, total_cost);
            routes.push_back(route);
            cycle_time = stoi(nodes.back().substr(1)) - stoi(nodes[0].substr(1));
        }

        Flight new_flight = Flight(start_node[0], cycle_time+1, routes.size(), cycle_time, volume_ub, weight_ub);
        new_flight.routes = routes;
        flights.push_back(new_flight);
    }
}

void AirNetwork::run_algo() {
    for(auto &flight : designed_flights) {
        vector<Route> best_routes;
        double best_cost = INT_MAX;
        for(int i = 0; i <= 7 * TIME_SLOT_A_DAY - flight.gap * flight.freq; i++) {
            double acc_cost = 0;
            vector<Route> routes;
            for (int freq = 0 ; freq < flight.freq; freq ++) {
                Route route = shortest_route(flight.start_node, i+freq*flight.gap , flight.start_node, i + freq*flight.gap + flight.cycle_time);
                routes.push_back(route);
                acc_cost += route.cost;
            }
            if(acc_cost < best_cost){
                best_cost = acc_cost;
                best_routes = routes;
            }
        }
        flight.routes = best_routes;
//        for(const auto &route : flight.routes){
//            cout << route ;
//        }
    }
}
    
// TODO: Change the responsibility of freeing memory to caller.
Route AirNetwork::shortest_route(char start_node, int start_time, char end_node, int end_time) {
    Route **dp = new Route *[num_nodes];
    for (int i = 0; i < num_nodes; i++)
        dp[i] = new Route[TOTAL_TIME_SLOT];


    int start_node_idx = (int) start_node - 'A';
    int end_node_idx = (int) end_node - 'A';

    vector<string> init_node = vector<string>();
    init_node.push_back(start_node + to_string(start_time));

    dp[start_node_idx][start_time] = Route(init_node, stop_cost[start_node_idx]);
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
    
void AirNetwork::forward_update(Route** dp, int node, int time) {
    char node_char = (char) ('A' + node) ;
    Node* cur_node = nodes[node_char][time];

    for (auto& arc : cur_node->out_arcs){
        Node* end_node = arc->end_node;
        char end_node_char = end_node->getName()[0];
        int end_node_idx = (int) end_node_char - 65;
        int end_time = stoi(end_node->getName().substr(1));

        //Calculate cost if append end node to current route
        Route cur_route = dp[node][time];
        Route end_route = dp[end_node_idx][end_time];
        double new_cost = cur_route.cost + arc->cost + arc->fixed_cost + end_node->getCost();
        new_cost = MAX(0, new_cost);

        // if yes, replace old route.
        if (new_cost < end_route.cost) {
            vector<string> new_nodes;
            new_nodes.assign(cur_route.nodes.begin(), cur_route.nodes.end());
            new_nodes.push_back(end_node_char + to_string(end_time));
            //cout << end_node_idx << " " << end_time <<endl;

            dp[end_node_idx][end_time] = Route(new_nodes, new_cost);
        }
    }
}

void AirNetwork::print_flights(const vector<Flight>& flights, const string& prefix) {
    cout << "---------------" + prefix ;
    cout << " flights routes-----------" <<endl;
    for(const auto &flight : flights) {
        for(const auto &route : flight.routes){
            cout << route;
        }
    }

}

const vector<Flight> &AirNetwork::getFlights() const {
    return designed_flights;
}

const vector<Flight> &AirNetwork::getCur_flights() const {
    return cur_flights;
}

const vector<Flight> &AirNetwork::getRival_flights() const {
    return rival_flights;
}

void AirNetwork::set_designed_flight(Route route) {
    vector<Route> routes;
    Flight designed_flight = designed_flights[0];
    routes.push_back(route);
    for(int i = 1; i < designed_flight.freq; i++){
        Route next_route = Route(route,  designed_flight.gap * i);
        routes.push_back(next_route);
    }
    designed_flights[0].routes.assign(routes.begin(), routes.end());
}

const vector<Flight> &AirNetwork::getDesignedFlights() const {
    return designed_flights;
}

vector<Route*> AirNetwork::find_all_routes() {
    vector<Route*> all_routes;

    for(auto &flight : designed_flights) {
        int time_range = 7 * TIME_SLOT_A_DAY - flight.gap * flight.freq;
        for(int i = 0; i <= time_range; i++) {
            vector<Route*> routes;
            routes = find_routes_from_single_node(flight.start_node, i , flight.start_node, i + flight.cycle_time);
            all_routes.insert(all_routes.end(), routes.begin(), routes.end());
        }
    }
    return all_routes;

}

vector<Route*> AirNetwork::find_routes_from_single_node(char start_node, int start_time, char end_node, int end_time) {
    cout << "=======================AirNetwork::find_routes_from_single_node=======================" << endl;
    
    // TODO: Very Important! Check this algorithm is vaild!!
    
    // Internal data use to record travesal state.
    struct NodeTraversalData {
        string node;
        vector<Arc*> arcs;
        int cost;
    
        NodeTraversalData(string node, int cost, vector<Arc*> arcs): node(node), cost(cost), arcs(arcs) {}
    };
    
    vector<Route*> routes = vector<Route*>();
    
    int start_node_idx = (int) start_node - 'A';
    int end_node_idx = (int) end_node - 'A';
    
    // Use stack to record passed node.
    vector<NodeTraversalData*> stack {new NodeTraversalData(start_node + to_string(start_time), stop_cost[start_node_idx], nodes[start_node][start_time]->out_arcs)};
    while (stack.empty() == false) {
        auto data = stack.back();
        string node_str = data->node;
        
        // If arcs are all visited pop back and find next node.
        if (stack.back()->arcs.empty() || data->cost > 10000) {
            delete stack.back();
            stack.pop_back();
        } else {
            Arc *arc = stack.back()->arcs.back();
            stack.back()->arcs.pop_back();
            
            Node *next_node = arc->end_node;
            int next_time = next_node->getTime();
            string next_node_str = next_node->getName();
    
            double cost = stack.back()->cost + arc->cost + next_node->getCost();
    
            // If node is feasible...
            if (next_time <= end_time) {
                // If we reach the goal...
                if (next_node->getNode() == end_node_idx && next_time == end_time) {
                    vector<string> new_nodes = vector<string>();
                    for (auto& data : stack) {
                        new_nodes.push_back(data->node);
                    }
                    // TODO: Very Important! Check wheather if we need add additional node to stay at the end!
                    new_nodes.push_back(next_node_str);
                    
                    Route *route = new Route(new_nodes, cost);
                    routes.push_back(route);
                    
                    cout << *route;
    
                } else {
                    stack.push_back(new NodeTraversalData(next_node_str, cost, next_node->out_arcs));
                }
            }
        }
    }

    return routes;
}
