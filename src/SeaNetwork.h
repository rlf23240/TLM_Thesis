//
// Created by Ashee on 2019/6/4.
//

#ifndef TLM_THESIS_SEANETWORK_H
#define TLM_THESIS_SEANETWORK_H

#include "Network.h"
#include "param.h"
#include <string>
#include <chrono>

using namespace std;

struct Ship{
    Ship(char start_node, int start_time, int frequency, int cycle_time, int volume_ub);

    char start_node;
    int start_time;
    int frequency;
    int cycle_time;
    int volume_ub;
    Route route;
};

class SeaNetwork : public Network{
private:
    vector<Ship> designed_ships;
    vector<Ship> cur_ships;
    vector<Ship> rival_ships;
    
    void read_ship_param(string ships_data);
    void read_data(string data_path) override;
    void forward_update(Route **dp, int node, int time) override;
    void read_sea_routes(string data_path, vector<Ship>& ships);
    void print_ships(vector<Ship> ships, string prefix);
    vector<Route*> find_routes_from_single_node(char start_node, int start_time, char end_node, int end_time);
    void forward_append(vector<Route*>** dp, int node, int time, int finish_time);


public:
    explicit SeaNetwork(string data_path, int num_cur_ships,int num_rival_ships);
    SeaNetwork();
    
    void generate_designed_ship();
    void run_algo() override;
    const vector<Ship> &getShips() const;
    const vector<Ship> &getCur_ships() const;
    const vector<Ship> &getRival_ships() const;
    vector<Route*> find_all_routes();
    void set_designed_ship(Route route);
    const vector<Ship> &getDesignedShips() const;

};

#endif //TLM_THESIS_SEANETWORK_H
