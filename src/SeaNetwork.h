//
// Created by Ashee on 2019/6/4.
//

#ifndef TLM_THESIS_SEANETWORK_H
#define TLM_THESIS_SEANETWORK_H

#include "Network.h"
#include "param.h"

using namespace std;

struct Ship{
    Ship(char start_node, int start_time, int frequency, int cycle_time);

    char start_node;
    int start_time;
    int frequency;
    int cycle_time;
};

class SeaNetwork : public Network{
private:
    vector<Ship> ships;
public:
    explicit SeaNetwork(string data_path);

    void read_data(string data_path) override;
    void read_ship_param(string ships_data);
    void run_algo() override;
    void forward_update(Route **dp, int node, int time) override;
};

#endif //TLM_THESIS_SEANETWORK_H
