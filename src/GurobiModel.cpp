//
// Created by Ashee on 2019/7/16.
//

#include "GurobiModel.h"
#include "hash.hpp"

GurobiModel::GurobiModel(string data){
//    all_paths_for_GurobiModel(data);
////    Run_GurobiModel(data);
}

void GurobiModel::Run_GurobiModel(string data) {
    clock_t start = clock();
    CargoRoute cargo_route = CargoRoute(data);
    EntireNetwork *network = cargo_route.getNetworks();
    SeaNetwork *sea_network = network->getSea_network();
    AirNetwork *air_network = network->getAir_network();

    vector<Route*> candidate_designed_flight_routes = vector<Route*>();//air_network->find_all_routes();
    vector<Route*> candidate_designed_ship_routes = vector<Route*>();//sea_network->find_all_routes();
    //cout  << candidate_designed_ship_routes.size()<<endl;
    //cout  << candidate_designed_flight_routes.size() << endl;

//    for(const auto& route : candidate_designed_ship_routes){
//        cout << *route ;
//    }
//    for(const auto& route : candidate_designed_flight_routes){
//        cout << *route ;
//    }

    Solution* best = nullptr;
    int count = 0;
    for(const auto& sea_route : candidate_designed_ship_routes){
        for(const auto& air_route : candidate_designed_flight_routes) {
            if (double(clock() - start) / CLOCKS_PER_SEC > time_limit_for_gurobi)
              break;
            cargo_route.getNetworks()->set_sea_air_route(*sea_route, *air_route);
            cargo_route.rebuild_entire_network();

            Solution *result = nullptr;//cargo_route.Run_full_model();
            if (!best || result->P > best->P)
              best = result;
            cout << count++ << endl;
            cout << "Best : " << best->P << endl;
            cout << "RunTime : " << double(clock() - start)/CLOCKS_PER_SEC << endl;
      }
    }
    best_sol = best;
    cout << *best_sol;
//    this->output_result("Result_model_" + data, MIN(time_limit_for_gurobi, double(clock() - start) / CLOCKS_PER_SEC));

}

    void GurobiModel::output_result(string name, double run_time) {
        if(best_sol == nullptr){
            cout << "Fail to output results" << endl;
            exit(1);
        }

    if(run_time >= time_limit_for_gurobi)
        best_sol->P *= 0.98;
    best_sol->to_file(name, run_time);
}

void GurobiModel::all_paths_for_GurobiModel(string data) {
    CargoRoute cargo_route = CargoRoute(data);
    EntireNetwork *network = cargo_route.getNetworks();
    SeaNetwork *sea_network = network->getSea_network();
    AirNetwork *air_network = network->getAir_network();

    vector<Route*> candidate_designed_flight_routes = air_network->find_all_routes();
    vector<Route*> candidate_designed_ship_routes = sea_network->find_all_routes();
    
    int file_seq_number = 0;
    for(const auto& sea_route: candidate_designed_ship_routes){
        for(const auto& air_route: candidate_designed_flight_routes){
            cargo_route.getNetworks()->set_sea_air_route(*sea_route, *air_route);
            cargo_route.rebuild_entire_network();
            
            vector<Path*> paths = cargo_route.find_all_paths();
            export_paths(data + "_all_paths.csv", file_seq_number++, paths);
        }
    }
}

Solution *GurobiModel::getBestSol() const {
    return best_sol;
}

void GurobiModel::export_paths(string file_path_prefix,
                               int sequence_number,
                               const vector<Path*>& paths) {
    string file_path = file_path_prefix + ".part" + to_string(sequence_number);
    
    ofstream file;
    file.open(file_path);
    for(auto path: paths){
        for(const auto& point: path->points()){
            file << to_string(point.layer) + (char) (point.node + 65) + to_string(point.time);
            if(point != path->points().back()) {
                file << ",";
            }
        }
        file << "\n";
    }
    file.close();
    
    cout << "Export " << paths.size() << " pathes to " << file_path << endl;
}
