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
    vector<Cargo*> cargos;
    vector<Path*> target_paths;
    vector<Path*> rival_paths;
    vector<Path*> all_paths;
    EntireNetwork target_networks;
    EntireNetwork rival_networks;
    vector<Path*>** rival_path_categories;
    vector<Path*>** target_path_categories;
    vector<Path*>** all_path_categories;
    vector<Path*>* cargo_available_paths;
    vector<Path*>* target_cargo_available_paths;
    vector<Path*>* rival_cargo_available_paths;

    void read_cargo_file(string data);
    void get_available_path(vector<Path*>** path_categories, vector<Path*>& paths);
    void combine_paths(vector<Path*> target_paths, vector<Path*> rival_paths);
    void combine_path_categories(vector<Path*>** target_path_categories, vector<Path*>** rival_path_categories);
    void find_cargo_available_paths();
    void run_model();

    void set_constr1(GRBModel& model, GRBVar** z, GRBVar** z_);
    void set_constr2(GRBModel& model, GRBVar** z, GRBVar** u);
    void set_constr3(GRBModel& model);
    void set_constr4(GRBModel& model);
    void set_constr5(GRBModel& model);
    void set_constr6(GRBModel& model);
    void set_constr7(GRBModel& model);
};


#endif //TLM_THESIS_CARGOROUTE_H
