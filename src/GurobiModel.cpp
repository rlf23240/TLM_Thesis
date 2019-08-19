//
// Created by Ashee on 2019/7/16.
//
#include "GurobiModel.h"
struct path_hash{
    size_t operator() (const Path& path) const
    {
        int hash = 0;
        for(const auto& point : path.points){
            hash += (point.layer + point.node + point.time);
        }

        return std::hash<int>()(hash);
    }
};

struct path_equal{
public:
    bool operator()(const Path& lhs, const Path& rhs) const{
        return lhs.points == rhs.points;
    }
};

GurobiModel::GurobiModel(string data){
//    all_paths_for_GurobiModel(data);
////    Run_GurobiModel(data);
}

void GurobiModel::Run_GurobiModel(string data) {
    clock_t start = clock();
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
    this->output_result("Result_model_" + data, double(clock() - start));

}

void GurobiModel::output_result(string name, double run_time) {
    if(best_sol == nullptr){
        cout << "Fail to output results" << endl;
        exit(1);
    }

    best_sol->to_file(name, run_time);
}

void GurobiModel::all_paths_for_GurobiModel(string data) {
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

    unordered_set<Path, path_hash, path_equal> all_path;
//
    for(const auto& sea_route : candidate_designed_ship_routes){
        for(const auto& air_route : candidate_designed_flight_routes){
            cargo_route.getNetworks().set_sea_air_route(*sea_route, *air_route);
            cargo_route.rebuild_entire_network();

            vector<Path*> paths = cargo_route.find_all_paths();
            for(const auto& path : paths){
                all_path.insert(*path);
            }

        }
    }

    ofstream file;
    file.open(data + "_all_paths.csv");
    for(const Path& path : all_path){
        for(const auto& point : path.points){
            file << to_string(point.layer) + (char) (point.node + 65) + to_string(point.time);
            if(point != path.points.back())  file << ",";
        }
        file << "\n";
    }
    file.close();
    cout << all_path.size();
}