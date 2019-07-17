//
// Created by Ashee on 2019/7/16.
//
#include "GurobiModel.h"

GurobiModel::GurobiModel(string data) {
    CargoRoute cargo_route = CargoRoute(data);
    EntireNetwork network = cargo_route.getNetworks();
    SeaNetwork sea_network = network.getSea_network();
    AirNetwork air_network = network.getAir_network();

    vector<Route*> candidate_designed_flight_routes = air_network.find_all_routes();
    vector<Route*> candidate_designed_ship_routes = sea_network.find_all_routes();
    cout  << candidate_designed_ship_routes.size()<<endl;
    cout  << candidate_designed_flight_routes.size() << endl;

    for(const auto& route : candidate_designed_ship_routes){
        cout << *route ;
    }
    for(const auto& route : candidate_designed_flight_routes){
        cout << *route ;
    }

    Solution* best = nullptr;
    int count = 0;
    for(const auto& sea_route : candidate_designed_ship_routes){
        for(const auto& air_route : candidate_designed_flight_routes){
            cargo_route.getNetworks().set_sea_air_route(*sea_route, *air_route);
            cargo_route.rebuild_entire_network();

            Solution* result = cargo_route.Run_full_model();
            if(!best || result->P > best->P)
                best = result;
            cout << count++ << endl;
            cout << "Best : " << best->P << endl;
        }
    }
    best_sol = best;
    cout << *best_sol;
}

void GurobiModel::output_result(string name, double run_time) {
    if(best_sol == nullptr){
        cout << "Fail to output results" << endl;
        exit(1);
    }

    best_sol->to_file(name, run_time);
}
