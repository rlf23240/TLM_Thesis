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
    vector<Flight> flights;
    void read_flights_param(std::string flights_data);
public:
    explicit AirNetwork(string data_path);

    AirNetwork();

    void read_data(std::string data_path) override;
    void run_algo() override;

    const vector<Flight> &getFlights() const;
};

#endif //TLM_THESIS_AIRNETWORK_H
