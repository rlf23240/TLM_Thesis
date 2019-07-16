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

    double best = -INT_MAX;
    int count = 0;
    for(const auto& ship_route : candidate_designed_ship_routes){
        for(const auto& air_route : candidate_designed_flight_routes){
            sea_network.set_designed_ship(*ship_route);
            cargo_route.getNetworks().setSea_network(sea_network);
            air_network.set_designed_flight(*air_route);
            cargo_route.getNetworks().setAir_network(air_network);
            cargo_route.rebuild_entire_network();

            double result = cargo_route.Run_full_model() -ship_route->cost -air_route->cost * air_network.getFlights()[0].freq * 4;
            if(result > best)
                best = result;
            cout << count++ << endl;
            cout << "Best : " << best << endl;
        }
    }
    cout << "Solution : " << best << endl;

}
