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
    double **e, **e_, **v, **v_;
    vector<Cargo*> cargos;
    vector<Path*> all_paths;
    EntireNetwork networks;
    vector<Path*>** path_categories;
    vector<Path*>* target_path;
    vector<Path*>* rival_path;
    GRBConstr* cons1;
    vector<GRBConstr> *cons2, *cons3, *cons4;
    unordered_map<int, unordered_map<int, GRBConstr>> cons5, cons6, cons7;


    unordered_map<int, unordered_map<int, Arc*>> arcs;
    void read_cargo_file(string data);
    void get_available_path(vector<Path*>** path_categories, vector<Path*>& paths);
    void cal_paths_profit();
    void cal_paths_cost();
    void cal_path_profit(Path* path);
    void cal_path_cost(Path* path);

    void branch_and_price();
    void bp_init(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *z_, vector<GRBVar> *u);
    void select_init_path();
    void Var_init(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *z_, vector<GRBVar> *u);
    void Obj_init(GRBModel &model, vector<GRBVar> *z);
    void Constr_init(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *z_, vector<GRBVar> *u);
    void set_constr1(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *z_);
    void set_constr2(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *u);
    void cal_e();
    void cal_v();
    void set_constr3(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *z_, vector<GRBVar> *u);
    void set_constr4(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *z_, vector<GRBVar> *u);
    void set_constr5(GRBModel &model, vector<GRBVar> *z);
    void set_constr6(GRBModel &model, vector<GRBVar> *z);
    void set_constr7(GRBModel &model, vector<GRBVar> *z);
    void update_arcs();
    Path* append_most_profit_path();
    void show_model_result(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *z_, vector<GRBVar> *u);

};


#endif //TLM_THESIS_CARGOROUTE_H
