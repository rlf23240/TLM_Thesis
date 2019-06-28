//
// Created by Ashee on 2019/6/27.
//

#ifndef TLM_THESIS_CARGOROUTE_H
#define TLM_THESIS_CARGOROUTE_H

#include "Cargo.cpp"
#include "EntireNetwork.h"
#include "param.h"
#include "gurobi_c++.h"

class CargoRoute {
public:
    explicit CargoRoute(string data);

private:
    vector<Cargo*> cargos;
    vector<Path*> paths;
    EntireNetwork networks;
    void read_cargo_file(string data);
    void run_model();
    void get_available_path();
};


#endif //TLM_THESIS_CARGOROUTE_H
