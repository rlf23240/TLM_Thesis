//
// Created by Ashee on 2019/6/4.
//

#ifndef TLM_THESIS_AIRNETWORK_H
#define TLM_THESIS_AIRNETWORK_H

#include "Network.h"
#include <string>
#include <chrono>

struct Flight{
    Flight(char start_node, int gap, int freq, int cycle_time, int volume_ub, int weight_ub);

    friend ostream &operator<<(ostream &os, const Flight &flight);

    char start_node;
    int gap;
    int freq;
    int cycle_time;
    int weight_ub;
    int volume_ub;
    vector<Route> routes;
};

class AirNetwork : public Network{
private:
    vector<Flight> designed_flights;
    vector<Flight> cur_flights;
    vector<Flight> rival_flights;
    void read_flights_param(std::string flights_data);
    void read_data(std::string data_path) override;
    void print_flights(const vector<Flight>& flights, const string& prefix);
    void generate_flights(vector<Flight> &flight, int n, int seed);
    vector<Route*> find_routes_from_single_node(char start_node, int start_time, char end_node, int end_time);
    void forward_append(vector<Route*>** dp, int node, int time);

public:
    explicit AirNetwork(string data_path, int num_cur_flights, int num_rival_ships);
    AirNetwork();
    void run_algo() override;
    vector<Route*> find_all_routes();
    void generate_designed_flight();
    const vector<Flight> &getFlights() const;
    const vector<Flight> &getCur_flights() const;
    const vector<Flight> &getRival_flights() const;
    void set_designed_flight(Route route);
};

#endif //TLM_THESIS_AIRNETWORK_H
