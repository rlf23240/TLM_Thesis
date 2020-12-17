//
//  BPGurobiModelSolution.cpp
//  TLM_Thesis
//
//  Created by Ian Wang on 2020/12/14.
//  Copyright © 2020 ian wang. All rights reserved.
//

#include "BPGurobiModelSolution.hpp"

BPGurobiModelSolution::BPGurobiModelSolution() {
    // Empty solution.
}

BPGurobiModelSolution::BPGurobiModelSolution(const GRBModel& model,
                                             const vector<unordered_map<Path, GRBVar>>& gvar_u,
                                             const vector<unordered_map<Path, GRBVar>>& gvar_z_target,
                                             const vector<unordered_map<Path, GRBVar>>& gvar_z_rival,
                                             const vector<GRBConstr>& gconstrs_1,
                                             const vector<unordered_map<Path, GRBConstr>>& gconstrs_2,
                                             const vector<unordered_map<Path, GRBConstr>>& gconstrs_3,
                                             const vector<unordered_map<Path, GRBConstr>>& gconstrs_4,
                                             const map<int, map<int, GRBConstr>>& gconstrs_5,
                                             const map<int, map<int, GRBConstr>>& gconstrs_6,
                                             const map<int, map<int, GRBConstr>>& gconstrs_7) {
    try {
        // Extract model objective result.
        extract_objective(model);
        
        // Extract model variable results.
        extract_variables(model, gvar_u, gvar_z_target, gvar_z_rival);
        
        // Extract constrain dual values.
        extract_constr1_dual_values(model, gconstrs_1);
        extract_constr234_dual_values(model, gconstrs_2, gconstrs_3, gconstrs_4);
        extract_constr567_dual_values(model, gconstrs_5, gconstrs_6, gconstrs_7);
        
    
    } catch(GRBException e) {
        cout << "Invaild model or variable while construct solution profile." << endl;
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    }
}

void BPGurobiModelSolution::extract_objective(const GRBModel& model) {
    objValue = model.get(GRB_DoubleAttr_ObjVal);
}

void BPGurobiModelSolution::extract_variables(const GRBModel &model,
                                              const vector<unordered_map<Path, GRBVar>>& gvar_u,
                                              const vector<unordered_map<Path, GRBVar>>& gvar_z_target,
                                              const vector<unordered_map<Path, GRBVar>>& gvar_z_rival) {
    u = vector<unordered_map<Path, double>>();
    target_z = vector<unordered_map<Path, double>>();
    rival_z = vector<unordered_map<Path, double>>();
    
    // Extract target z.
    for (auto vars: gvar_z_target) {
        auto z_values = unordered_map<Path, double>();
        for (auto pair: vars) {
            z_values[pair.first] = pair.second.get(GRB_DoubleAttr_X);
        }
        target_z.push_back(z_values);
    }
    
    // Extract rival z
    for (auto vars: gvar_z_rival) {
        auto z_values = unordered_map<Path, double>();
        for (auto pair: vars) {
            z_values[pair.first] = pair.second.get(GRB_DoubleAttr_X);
        }
        rival_z.push_back(z_values);
    }
    
    // Extract u value.
    for (auto vars: gvar_u) {
        auto u_values = unordered_map<Path, double>();
        for (auto pair: vars) {
            u_values[pair.first] = pair.second.get(GRB_DoubleAttr_X);
        }
        u.push_back(u_values);
    }
}

void BPGurobiModelSolution::extract_constr1_dual_values(const GRBModel& model,
                                                        const vector<GRBConstr>& gconstr_1) {
    constr1_pi_values = vector<double>();
    for(auto constr: gconstr_1) {
        constr1_pi_values.push_back(constr.get(GRB_DoubleAttr_Pi));
    }
}

void BPGurobiModelSolution::extract_constr234_dual_values(const GRBModel&model,
                                                          const vector<unordered_map<Path, GRBConstr>>& gconstrs_2,
                                                          const vector<unordered_map<Path, GRBConstr>>& gconstrs_3,
                                                          const vector<unordered_map<Path, GRBConstr>>& gconstrs_4) {
    constr2_pi_values = vector<unordered_map<Path, double>>();
    for(auto constrs: gconstrs_2) {
        unordered_map<Path, double> pi_values = unordered_map<Path, double>();
        for(auto pair: constrs) {
            pi_values[pair.first] = pair.second.get(GRB_DoubleAttr_Pi);
        }
        constr2_pi_values.push_back(pi_values);
    }
    
    constr3_pi_values = vector<unordered_map<Path, double>>();
    for(auto constrs: gconstrs_3) {
        unordered_map<Path, double> pi_values = unordered_map<Path, double>();
        for(auto pair: constrs) {
            pi_values[pair.first] = pair.second.get(GRB_DoubleAttr_Pi);
        }
        constr3_pi_values.push_back(pi_values);
    }
    
    constr4_pi_values = vector<unordered_map<Path, double>>();
    for(auto constrs: gconstrs_4) {
        unordered_map<Path, double> pi_values = unordered_map<Path, double>();
        for(auto pair: constrs) {
            pi_values[pair.first] = pair.second.get(GRB_DoubleAttr_Pi);
        }
        constr4_pi_values.push_back(pi_values);
    }
}

void BPGurobiModelSolution::extract_constr567_dual_values(const GRBModel &model,
                                                          const map<int, map<int, GRBConstr>>& gconstrs_5,
                                                          const map<int, map<int, GRBConstr>>& gconstrs_6,
                                                          const map<int, map<int, GRBConstr>>& gconstrs_7) {
    constr5_pi_values = map<int, map<int, double>>();
    for (auto begin_pair: gconstrs_5) {
        int begin_idx = begin_pair.first;
        for (auto dest_pair: begin_pair.second) {
            int dest_idx = dest_pair.first;
            
            constr5_pi_values[begin_idx][dest_idx] = dest_pair.second.get(GRB_DoubleAttr_Pi);
        }
    }
    
    constr6_pi_values = map<int, map<int, double>>();
    for (auto begin_pair: gconstrs_6) {
        int begin_idx = begin_pair.first;
        for (auto dest_pair: begin_pair.second) {
            int dest_idx = dest_pair.first;
            
            constr6_pi_values[begin_idx][dest_idx] = dest_pair.second.get(GRB_DoubleAttr_Pi);
        }
    }
    
    constr7_pi_values = map<int, map<int, double>>();
    for (auto begin_pair: gconstrs_7) {
        int begin_idx = begin_pair.first;
        for (auto dest_pair: begin_pair.second) {
            int dest_idx = dest_pair.first;
            
            constr7_pi_values[begin_idx][dest_idx] = dest_pair.second.get(GRB_DoubleAttr_Pi);
        }
    }
}

bool BPGurobiModelSolution::is_integer_solution() {
    for(auto cargos: u) {
        for(auto pair: cargos){
            double u_value = pair.second;
            if(u_value != 1.0 && u_value != 0.0) {
                return false;
            }
        }
    }
    return true;
}
