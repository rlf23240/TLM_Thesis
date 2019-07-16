//
// Created by Ashee on 2019/7/16.
//
#include "GurobiModel.h"

GurobiModel::GurobiModel(const EntireNetwork &network) : network(network) {
    SeaNetwork seaNetwork = this->network.getSea_network();
    AirNetwork airNetwork = this->network.getAir_network();

    vector<Route*> candidate_designed_flight_routes = airNetwork.find_all_routes();
    vector<Route*> candidate_designed_ship_routes = seaNetwork.find_all_routes();
    cout  << candidate_designed_ship_routes.size()<<endl;
    cout  << candidate_designed_flight_routes.size() << endl;

    for(const auto& route : candidate_designed_ship_routes){
        cout << *route ;
    }
    for(const auto& route : candidate_designed_flight_routes){
        cout << *route ;
    }

}
