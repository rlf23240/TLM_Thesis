//
// Created by Ashee on 2019/6/4.
//

#ifndef TLM_THESIS_SEANETWORK_H
#define TLM_THESIS_SEANETWORK_H

#include "Network.h"
#include "param.h"

using namespace std;

struct Ship{
    Ship(char start_node, int start_time, int frequency, int cycle_time, int weight_ub);

    char start_node;
    int start_time;
    int frequency;
    int cycle_time;
    int weight_ub;
    Route route;
};

class SeaNetwork : public Network{
private:
    vector<Ship> designed_ships;
    vector<Ship> cur_ships;
    vector<Ship> rival_ships;
    void read_ship_param(string ships_data);
    void read_data(string data_path) override;
    void run_algo() override;
    void forward_update(Route **dp, int node, int time) override;
    void generate_ships(vector<Ship> &ships, int n, int seed);
    void print_ships(vector<Ship> ships, string prefix);

public:
    explicit SeaNetwork(string data_path, int num_cur_ships,int num_rival_ships);
    SeaNetwork();
    const vector<Ship> &getShips() const;
    const vector<Ship> &getCur_ships() const;
    const vector<Ship> &getRival_ships() const;
};

#endif //TLM_THESIS_SEANETWORK_H
