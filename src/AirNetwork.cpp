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
    generate_flights(cur_flights, num_cur_flights, 0);
    generate_flights(rival_flights, num_rival_flights, 1);

    print_flights(designed_flights, "Designed");
    print_flights(cur_flights, "Existing");
    print_flights(rival_flights, "Rival");
}

void AirNetwork::read_data(std::string data_path) {
    Network::read_data(data_path);
    read_flights_param(data_path+"_flights_param.txt");
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
        cur_time = next_time;
        total_cost += stop_cost[next_node];
        total_cost += arc_cost[cur_node][next_node];
        nodes.push_back((char) (65 + next_node) + to_string(next_time));
    }
    if(cur_time == 12 && start_time == 4) return;

    Route route = Route(nodes, total_cost);
    int gap = (cur_time -start_time+1);


    Flight new_flight = Flight((char) (65 + start_node), gap, freq, cur_flight.cycle_time, cur_flight.volume_ub , cur_flight.weight_ub);
    routes.push_back(route);
    for(int f = 1; f< freq; f++){
        Route next_route = Route(route,  gap * f);
        routes.push_back(next_route);
    }
    new_flight.routes = routes;
    designed_flights[0] = new_flight;
}

void AirNetwork::generate_flights(vector<Flight> &flights, int n, int seed) {
    random_device rd;
    mt19937 gen = mt19937(seed);
    uniform_int_distribution<int> dis(0, INT_MAX);


    for (int i = 0; i < n; i++) {
        vector<Route> routes;
        int cycle_time = 4 + dis(gen) % 2;


        int start_node = dis(gen) % num_nodes;
        int start_time = dis(gen) % 5;
        int cur_node = start_node;
        int cur_time = start_time;
        int next_node, next_time;
        int total_cost = 0;

        int freq =  2 ;


        vector<string> nodes;
        nodes.push_back((char) (65 + cur_node) + to_string(cur_time));
        total_cost = stop_cost[cur_node];

        while (cur_time - start_time < cycle_time) {
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
            cur_time = next_time;
            total_cost += stop_cost[next_node];
            total_cost += arc_cost[cur_node][next_node];
            nodes.push_back((char) (65 + next_node) + to_string(next_time));
        }

        Route route = Route(nodes, total_cost);
        int weight_ub = (5 + dis(gen) % 11) * 100 ;
        int volume_ub = (5 + dis(gen) % 11) * 100 ;
        int gap = (cur_time -start_time+1);

        Flight new_flight = Flight((char) (65 + start_node), gap, freq, cycle_time, volume_ub, weight_ub);
        routes.push_back(route);
        for(int f = 1; f< freq; f++){
            Route next_route = Route(route,  gap * f);
            routes.push_back(next_route);
        }
        new_flight.routes = routes;
        flights.push_back(new_flight);
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








