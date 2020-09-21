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
    double get_original_profit();
    
    vector<double> get_r_column();
    unordered_map<pair<int, int>, bool, pair_hash> get_arc_usage_in_design();
    
    Solution* run_bp();
    void rebuild_entire_network();
    const vector<pair<int, int>> &getSea_arc_pairs() const;
    const vector<pair<int, int>> &getAir_arc_pairs() const;
    EntireNetwork *getNetworks();
    const vector<Path *>& find_all_paths();
    void arcs_to_file(string data);
    
    void out_put_v_value(ostream& os);
    void out_put_v_value_with_path(ostream& os, vector<Path*> *target_path, vector<Path*> *rival_path);

    Solution* Run_full_model();
    
    // Number of sea arcs.
    int num_sea_arc() {
        return sea_arc_pairs.size();
    };
    
    // Number of air arcs.
    int num_air_arc() {
        return air_arc_pairs.size();
    };
    
    const vector<Cargo*> getCargos() const {
        return cargos;
    };

private:
    unsigned int num_nodes;
    double **e, **e_, **v, **v_;
    vector<Cargo*> cargos;
    vector<Path*> all_paths;
    EntireNetwork* networks;
    
    int bp_iter = 0;

    vector<Path*>** path_categories = NULL;
    vector<Path*>* target_path = NULL;
    vector<Path*>* rival_path = NULL;
    
    #pragma mark Column Deletion
    // I know this is bed idea to put this, don't complain!
    vector<unordered_map<Path, int>>* not_use_count = NULL;
    void column_deletion(GRBModel &model);
    
    unordered_set<int>* chosen_paths;
    unordered_map<int, unordered_map <Path, bool>> integer_set;

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
    //void cal_paths_profit();
    double cal_path_profit(int cargo_index, Path* path, Cargo *cargo);
    void cal_paths_cost();
    void cal_path_cost(Path* path);
    void cal_target_path_reduced_cost(Path* path, int cargo_index);
    void cal_rival_path_reduced_cost(Path* path, int cargo_index);
    Solution* branch_and_price();
    void bp_init(GRBModel &model);
    void select_init_path();
    bool is_path_feasible_for_cargo(Path* path, Cargo* cargo);
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
    pair<Path*, int> select_most_profit_target_path();
    pair<Path*, int> select_most_profit_rival_path();
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
    double get_air_complicate_constr_val_volume(int start_idx, int end_idx, int ub);
    double get_air_complicate_constr_val_weight(int start_idx, int end_idx, int ub);
    GRBLinExpr complicate_constr_lhs_volume(int start_idx, int end_idx);
    GRBLinExpr complicate_constr_lhs_weight(int start_idx, int end_idx);
    double complicate_sea_rhs(int start_idx, int end_idx, int ub);
    double complicate_air_rhs(int start_idx, int end_idx, int ub);
    void reset_bp();


    void show_model_result(GRBModel &model);
};


#endif //TLM_THESIS_CARGOROUTE_H
