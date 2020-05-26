//
// Created by Ashee on 2019/7/8.
//

#ifndef TLM_THESIS_DANTZIG_WOLFE_H
#define TLM_THESIS_DANTZIG_WOLFE_H

#include "param.h"
#include "CargoRoute.h"

#define DW_ITER_LOG(LOG_COMMAND) TLMLOG("DW Iter", LOG_COMMAND)

class Dantzig_wolfe {
public:
    explicit Dantzig_wolfe(const CargoRoute &cargoRoute);
    void output_result(string name, double run_time);
    Solution *getBestSol() const;
    
    const CargoRoute getCargoRoute();
private:
    CargoRoute cargoRoute;
    
    // Objective value of every run.
    vector<double> P = vector<double>();
    
    // Constrain matrix.
    vector<vector<double>> R;
    
    // Dual price of constrain sum(lambda) + tau = 1.
    double sigma;
    
    vector<double> previous_lambda = vector<double>();
    
    vector<Solution*> solutions;
    Solution* best_sol = nullptr;
    vector<double> model_result;
    int stop_iter = 1;
    void append_R_column(vector<double> r_column);
    vector<double> Run_Dantzig_wolfe();
    void Final_result();
    void update_arc_by_pi(vector<double> pi);
    bool end_condition(vector<double> pi);
};


#endif //TLM_THESIS_DANTZIG_WOLFE_H
