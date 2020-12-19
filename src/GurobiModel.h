//
// Created by Ashee on 2019/7/16.
//

#ifndef TLM_THESIS_GUROBIMODEL_H
#define TLM_THESIS_GUROBIMODEL_H

#include "CargoRoute.h"
#include "param.h"

class GurobiModel {
public:
    explicit GurobiModel(string data);
    void Run_GurobiModel(string data);
    void all_paths_for_GurobiModel(string data);
    void output_result(string name, double run_time);
    EntireNetwork network;
    Solution* best_sol;
    Solution *getBestSol() const;

private:
    vector<Route*> all_ship_routes;
    vector<Route*> all_flight_routes;
    
    void create_export_dest(string file_path);
    void export_paths(string file_path,
                      const vector<Path*>& path);
};


#endif //TLM_THESIS_GUROBIMODEL_H
