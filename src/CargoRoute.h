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
    unsigned int num_nodes;
    double **v, **v_;
    double **e, **e_;
    vector<Cargo*> cargos;
    vector<Path*> all_paths;
    EntireNetwork networks;
    vector<Path*>** path_categories;
    vector<pair<int, Path*>> selected_path;


    unordered_map<int, unordered_map<int, Arc*>> arcs;


    void read_cargo_file(string data);
    void get_available_path(vector<Path*>** path_categories, vector<Path*>& paths);
    void cal_path_profit(Path* path);
    void cal_path_cost(Path* path);

    void branch_and_price();
    void bp_init(GRBModel &model, vector<GRBVar> z, vector<GRBVar> u);
    void Var_init(GRBModel &model, vector<GRBVar> z, vector<GRBVar> u);
    void Obj_init(GRBModel &model, vector<GRBVar> z);
};


#endif //TLM_THESIS_CARGOROUTE_H
