//
// Created by Ashee on 2019/6/4.
//

#ifndef TLM_THESIS_AIRNETWORK_H
#define TLM_THESIS_AIRNETWORK_H

#include "Network.h"

struct Flight{
    Flight(char start_node, int gap, int freq, int cycle_time, int weight_ub, int volume_ub);

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
    bool is_target;
    vector<Flight> flights;
    vector<Flight> cur_flights;
    void read_flights_param(std::string flights_data);


public:
    explicit AirNetwork(string data_path, int num_cur_flights, bool is_target);
    AirNetwork();
    void read_data(std::string data_path) override;
    void run_algo() override;
    void print_flights(vector<Flight> flights, bool is_designed, bool is_target);
    void generate_cur_flights(int n);

    const vector<Flight> &getFlights() const;
    const vector<Flight> &getCur_flights() const;


};

#endif //TLM_THESIS_AIRNETWORK_H
