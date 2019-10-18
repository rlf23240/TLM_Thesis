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
        while(getline(iss, token, ',')){
            if(token == ""){
                Route route = Route(nodes, total_cost);
                routes.push_back(route);
                nodes.clear();
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
        int cycle_time = stoi(nodes.back().substr(1)) - stoi(nodes[0].substr(1));
        Route route = Route(nodes, total_cost);
        routes.push_back(route);

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
                Route route = DP_shortest_path(flight.start_node, i+freq*flight.gap , flight.start_node, i + freq*flight.gap + flight.cycle_time);
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

void AirNetwork::generate_designed_flight() {
//    unsigned seed = static_cast<unsigned int>(chrono::system_clock::now().time_since_epoch().count());
    mt19937 gen = mt19937(air_seed++);
    uniform_int_distribution<int> dis(0, INT_MAX);
    Flight cur_flight = designed_flights[0];

    vector<Route> routes;

    int start_node = (int) cur_flight.routes[0].nodes[0][0]-65;
    int start_time = dis(gen) % 5;
    int cur_node = start_node;
    int cur_time = start_time;
    int next_node, next_time;
    int total_cost = 0;

    int freq =  2 ;

    vector<string> nodes;
    nodes.push_back((char) (65 + cur_node) + to_string(cur_time));
    total_cost = stop_cost[cur_node];

    while (cur_time - start_time < cur_flight.cycle_time -1) {
        do {
            next_node = dis(gen) % num_nodes;
        } while (cur_node == next_node);

        next_time = cur_time + time_cost[cur_node][next_node];
        total_cost += stop_cost[next_node];
        total_cost += arc_cost[cur_node][next_node];

        nodes.push_back((char) (65 + next_node) + to_string(next_time));

        cur_node = next_node;
        cur_time = next_time;
    }
    //back to start node
    if(cur_node != start_node) {
        next_node = start_node;
        next_time = cur_time + time_cost[cur_node][next_node];
        if(next_time + 1 + (next_time - start_time) < 20) {
            cur_time = next_time;
            total_cost += stop_cost[next_node];
            total_cost += arc_cost[cur_node][next_node];
            nodes.push_back((char) (65 + next_node) + to_string(next_time));
        }
    }


    Route route = Route(nodes, total_cost);
    int gap = (cur_time - start_time+1);

    Flight new_flight = Flight((char) (65 + start_node), gap, freq, cur_flight.cycle_time, cur_flight.volume_ub , cur_flight.weight_ub);
    routes.push_back(route);
    for(int f = 1; f < freq; f++){
        Route next_route = Route(route,  gap * f);
        routes.push_back(next_route);
    }
    new_flight.routes = routes;
    designed_flights[0] = new_flight;
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
        string node_str = stack.back()->node;
        
        // If arcs are all visited pop back and find next node. 
        if (stack.back()->arcs.empty()) {
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
                if (next_node->getNode() == end_node_idx) {
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








