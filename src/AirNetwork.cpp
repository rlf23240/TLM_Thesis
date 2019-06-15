//
// Created by Ashee on 2019/6/4.
//

#include "AirNetwork.h"
#include <string>

/*-----------------Flight struct------------------------------*/

ostream &operator<<(ostream &os, const Flight &flight) {
    os << "start_node: " << flight.start_node << " gap: " << flight.gap << " freq: " << flight.freq << " cycle_time: "
       << flight.cycle_time;
    return os;
}

Flight::Flight(char start_node, int gap, int freq, int cycle_time, int weight_ub, int volume_ub) : start_node(
        start_node), gap(gap), freq(freq), cycle_time(cycle_time), weight_ub(weight_ub), volume_ub(volume_ub) {}

AirNetwork::AirNetwork() {}
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

            getline(iss, token, '\t');
            int weight_ub = stoi(token);

            getline(iss, token, '\t');
            int volume_ub = stoi(token);

            Flight new_flight = Flight(flight_name, flight_gap, flight_freq, flight_cycle_time, weight_ub, volume_ub);
            flights.push_back(new_flight);
        }
    }
    else {
        cout << "flights data cannot open !!!";
    }
}

void AirNetwork::run_algo() {

    for(auto &flight : flights) {
        vector<Route> best_routes;
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
                best_routes = routes;
            }
        }
        flight.routes = best_routes;
        for(const auto &route : flight.routes){
            cout << route ;
        }
    }

}

const vector<Flight> &AirNetwork::getFlights() const {
    return flights;
}


