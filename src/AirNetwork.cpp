//
// Created by Ashee on 2019/6/4.
//

#include "AirNetwork.h"
#include <string>

/*-----------------Flight struct------------------------------*/
Flight::Flight(char start_node, int gap, int freq, int cycle_time) : start_node(start_node), gap(gap),
                                                                     freq(freq), cycle_time(cycle_time) {}
ostream &operator<<(ostream &os, const Flight &flight) {
    os << "start_node: " << flight.start_node << " gap: " << flight.gap << " freq: " << flight.freq << " cycle_time: "
       << flight.cycle_time;
    return os;
}


AirNetwork::AirNetwork(const string data_path) {
    read_data(data_path);
    run_algo();
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

            Flight new_flight = Flight(flight_name, flight_gap, flight_freq, flight_cycle_time);
            flights.push_back(new_flight);
        }
    }
    else {
        cout << "flights data cannot open !!!";
    }
}

void AirNetwork::run_algo() {
    vector<Route> best_routes;
    for(auto flight : flights) {
        int best_cost = INT_MAX;
        for(int i = 0; i <= 7 * TIME_SLOT_A_DAY - flight.gap * flight.freq; i++) {
            int acc_cost = 0;
            vector<Route> routes;
            for (int freq = 0 ; freq < flight.freq; freq ++) {
                Route route = DP_shortest_path(flight.start_node, i+freq*flight.gap , flight.start_node, i + freq*flight.gap + flight.cycle_time);
                routes.push_back(route);
                acc_cost += route.cost;
            }
            if(acc_cost < best_cost){
                best_cost = acc_cost;
                best_routes.insert(best_routes.end(),routes.begin(), routes.end());
            }
        }
    }
    for(const auto &route : best_routes){
        cout << route;
    }
}
