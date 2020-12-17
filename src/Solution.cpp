#include <utility>
#include <ostream>

//
// Created by Ashee on 2019/7/16.
//
#include "param.h"
#include "hash.hpp"
#include "Network.h"

struct Solution{
    Solution(int cargo_size,
             vector<Path *> *target_path,
             vector<Path *> *rival_path,
             vector<unordered_map<Path, double>> z_value,
             double P,
             vector<double> r,
             unordered_map<pair<int, int>, bool, pair_hash> arc_usage_in_design,
             vector<Route> seaAirRoutes):
    P(P),
    arc_usage_in_design(std::move(arc_usage_in_design)),
    r(std::move(r)),
    sea_air_routes(seaAirRoutes) {
        // copy solution
        this->cargo_size = cargo_size;
        this->target_path = new vector<Path*>[cargo_size];
        this->rival_path = new vector<Path*>[cargo_size];
        
        this->z_value = new vector<double>[cargo_size];

        for(int k = 0; k < cargo_size; k++){
           // TODO: Check this if need deep copy or not?
            // Deep copy all path or it will all be release before next run.
            for (const auto &path: target_path[k]) {
                this->target_path[k].push_back(new Path(*path));
                this->z_value[k].push_back(z_value[k][*path]);
            }
            
            for (const auto &path: rival_path[k]) {
                this->rival_path[k].push_back(new Path(*path));
            }
        }
    }
    
    ~Solution() {
        delete target_path;
        delete z_value;
    }

    friend ostream &operator<<(ostream &os, const Solution &solution) {
        os << "-----------------------------------------------------------------------------------" << endl;
        for(int k = 0; k < solution.cargo_size; k++){
            for(int p = 0; p < solution.target_path[k].size(); p++){
                os << "Cargo :"<< k << ",\tz :" << solution.z_value[k][p] <<",\t" << *solution.target_path[k][p] ;
            }
        }
//        for(const auto& route : solution.sea_air_routes){
//            os << route;
//        }
        os << "-----------------------------------------------------------------------------------" << endl;
        return os;
    }

    void to_file(string name, double run_time){
        fstream file;
        file.open(name + ".txt", ios::out);
        if(!file.is_open()){
            cout << "fail to open results file" << endl;
            exit(1);
        }

        file << "P : " << P << endl;
        for(int k = 0; k < cargo_size; k++){
            for(int p = 0; p < target_path[k].size(); p++){
                file << "Cargo :"<< k << ",\tz :" << z_value[k][p] <<",\t" << *target_path[k][p] ;
            }
        }
        
        file << "\n";
        for(const auto &route: sea_air_routes) {
            file << route;
        }
        file << "Run time : " << run_time << " sec. " << endl;
        file.close();
    }


    vector<Path*>* target_path = NULL;
    vector<Path*>* rival_path = NULL;
    vector<double>* z_value = NULL;
    vector<Route> sea_air_routes{};
    
    unordered_map<pair<int, int>, bool, pair_hash> arc_usage_in_design = {};
    
    int cargo_size;
    double P{};
    vector<double> r;
};
