//
// Created by Ashee on 2019/6/27.
//

#ifndef TLM_THESIS_CARGOROUTE_H
#define TLM_THESIS_CARGOROUTE_H

#include "Cargo.cpp"
#include "EntireNetwork.h"
#include "BB_node.h"
#include "Solution.cpp"
#include "param.h"
#include "gurobi_c++.h"

struct pair_hash;

class CargoRoute {
public:
    explicit CargoRoute(string data);
    double getObjVal() const;
    double get_P_value();
    vector<double> get_r_column();
    Solution* run_bp();
    void rebuild_entire_network();
    const vector<pair<int, int>> &getSea_arc_pairs() const;
    const vector<pair<int, int>> &getAir_arc_pairs() const;
    EntireNetwork &getNetworks();
    vector<Path *> find_all_paths();

    Solution* Run_full_model();

private:
    unsigned int num_nodes;
    double **e, **e_, **v, **v_;
    vector<Cargo*> cargos;
    vector<Path*> all_paths;
    EntireNetwork networks;

    vector<Path*>** path_categories;
    vector<Path*>* target_path;
    vector<Path*>* rival_path;
    unordered_set<int>* chosen_paths;
    unordered_map<int, unordered_map <int, bool>> integer_set;

    vector<GRBVar> *z, *z_, *u;
    vector<double> *z_value;
    GRBConstr* cons1;
    vector<GRBConstr> *cons2, *cons3, *cons4;
    unordered_map<int, unordered_map<int, GRBConstr>> cons5, cons6, cons7;
    double incumbent = 0;
    double objVal;

    vector<pair<int, int>> sea_arc_pairs;
    vector<pair<int, int>> air_arc_pairs;

    unordered_map<int, unordered_map<int, Arc*>> arcs;
    void read_cargo_file(string data);
    void get_available_path(vector<Path*>** path_categories, vector<Path*>& paths);
    void cal_paths_profit();
    void cal_path_profit(Path* path);
    void cal_paths_cost();
    void cal_path_cost(Path* path);
    void cal_path_reduced_cost(Path* path, int k);
    Solution* branch_and_price();
    void bp_init(GRBModel &model);
    void select_init_path();
    void LP_relaxation(GRBModel &model);
    void column_generation(GRBModel &model);
    void Var_init(GRBModel &model);
    void Obj_init(GRBModel &model);
    void Constr_init(GRBModel &model);
    void set_constrs(GRBModel &model);
    void set_constr1(GRBModel &model);
    void set_constr2(GRBModel &model);
    void cal_e();
    void cal_v();
    void set_constr3(GRBModel &model);
    void set_constr4(GRBModel &model);
    void set_constr5(GRBModel &model);
    void set_constr6(GRBModel &model);
    void set_constr7(GRBModel &model);
    void set_complicate_constr1(GRBModel &model);
    void set_complicate_constr2(GRBModel &model);
    void set_complicate_constr3(GRBModel &model);
    void update_arcs();
    pair<Path*, int> select_most_profit_path();
    void append_column(Path* best_path, int best_k);
    bool is_integral();
    void set_integer(GRBModel &model);
    void set_all_u_integer(GRBModel &model, vector<GRBVar> *u);
    pair<int,int> find_kp_pair();

    void find_sea_arcs();
    void find_air_arcs();
    double* cal_constr1_val();
    double* cal_constr2_val();
    double* cal_constr3_val();
    unordered_set<pair<int, int>, pair_hash> get_arc_set(Path* path);
    double get_sea_complicate_constr_val(int start_idx, int end_idx, int ub);
    double get_air_complicate_constr_val(int start_idx, int end_idx, int ub);
    GRBLinExpr complicate_constr_lhs(int start_idx, int end_idx);
    double complicate_sea_rhs(int start_idx, int end_idx, int ub);
    double complicate_air_rhs(int start_idx, int end_idx, int ub);
    void reset_bp();


    void show_model_result(GRBModel &model);
};


#endif //TLM_THESIS_CARGOROUTE_H
