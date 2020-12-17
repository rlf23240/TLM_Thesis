//
//  BPGurobiModelSolution.hpp
//  TLM_Thesis
//
//  Created by Ian Wang on 2020/12/14.
//  Copyright © 2020 ian wang. All rights reserved.
//

#ifndef BPGurobiModelSolution_hpp
#define BPGurobiModelSolution_hpp

#include <stdio.h>
#include <vector>
#include <map>
#include <unordered_map>

#include "gurobi_c++.h"

#include "param.h"

using namespace std;

/// A class aim to capture essential result from solved Gurobi model
/// and provide those data to advanced calculations.
class BPGurobiModelSolution {
public:
    BPGurobiModelSolution();
    BPGurobiModelSolution(const GRBModel& model,
                          const vector<unordered_map<Path, GRBVar>>& gvar_u,
                          const vector<unordered_map<Path, GRBVar>>& gvar_z_target,
                          const vector<unordered_map<Path, GRBVar>>& gvar_z_rival,
                          const vector<GRBConstr>& gconstr_1,
                          const vector<unordered_map<Path, GRBConstr>>& gconstrs_2,
                          const vector<unordered_map<Path, GRBConstr>>& gconstrs_3,
                          const vector<unordered_map<Path, GRBConstr>>& gconstrs_4,
                          const map<int, map<int, GRBConstr>>& gconstrs_5,
                          const map<int, map<int, GRBConstr>>& gconstrs_6,
                          const map<int, map<int, GRBConstr>>& gconstrs_7);
    
    // Objective result.
    double objValue;
    
    // Variable result.
    vector<unordered_map<Path, double>> u;
    vector<unordered_map<Path, double>> target_z;
    vector<unordered_map<Path, double>> rival_z;
    
    // Essential dual values.
    vector<double> constr1_pi_values;
    vector<unordered_map<Path, double>> constr2_pi_values;
    vector<unordered_map<Path, double>> constr3_pi_values;
    vector<unordered_map<Path, double>> constr4_pi_values;
    map<int, map<int, double>> constr5_pi_values;
    map<int, map<int, double>> constr6_pi_values;
    map<int, map<int, double>> constr7_pi_values;
    
    // Additional properties.
    
    // Is all u either 0 or 1.
    bool is_integer_solution();
    
private:
    // Helper functions to extract values.
    void extract_objective(const GRBModel& model);
    void extract_variables(const GRBModel& model,
                           const vector<unordered_map<Path, GRBVar>>& gvar_u,
                           const vector<unordered_map<Path, GRBVar>>& gvar_z_target,
                           const vector<unordered_map<Path, GRBVar>>& gvar_z_rival);
    void extract_constr1_dual_values(const GRBModel& model,
                                     const vector<GRBConstr>& gconstr_1);
    
    void extract_constr234_dual_values(const GRBModel &model,
                                       const vector<unordered_map<Path, GRBConstr>>& gconstrs_2,
                                       const vector<unordered_map<Path, GRBConstr>>& gconstrs_3,
                                       const vector<unordered_map<Path, GRBConstr>>& gconstrs_4);
    
    void extract_constr567_dual_values(const GRBModel &model,
                                       const map<int, map<int, GRBConstr>>& gconstrs_5,
                                       const map<int, map<int, GRBConstr>>& gconstrs_6,
                                       const map<int, map<int, GRBConstr>>& gconstrs_7);
};

#endif /* BPGurobiModelSolution_hpp */
