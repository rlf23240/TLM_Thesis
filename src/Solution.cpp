#include <utility>
#include <ostream>

//
// Created by Ashee on 2019/7/16.
//
#include "param.h"
#include "Network.h"

struct Solution{
    Solution(int cargo_size, vector<Path *> *target_path, vector<double> *z_value, double P, vector<double> r,
             vector<Route> seaAirRoutes)
            : P(P), r(std::move(r)), sea_air_routes(seaAirRoutes) {
        // copy solution
        this->cargo_size = cargo_size;
        this->target_path = new vector<Path*>[cargo_size];
        this->z_value = new vector<double>[cargo_size];

        for(int k = 0; k < cargo_size; k++){
            this->target_path[k].assign(target_path[k].begin(), target_path[k].end());
            this->z_value[k].assign(z_value[k].begin(), z_value[k].end());
        }
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
        file.open("../results/" + name + ".txt", ios::out);
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
        for(const auto route : sea_air_routes){
            file << route;
        }
        file << "Run time : " << run_time / 1000 << " sec. " << endl;
        file.close();
    }


    vector<Path*>* target_path{};
    vector<double>* z_value{};
    vector<Route> sea_air_routes{};
    int cargo_size;
    double P{};
    vector<double> r;
};
