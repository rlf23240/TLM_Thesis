#include <iostream>

#include "src/Dantzig_wolfe.h"
#include "src/GurobiModel.h"
#include <ctime>

using namespace std;

void generate_paths_arcs(vector<string> data_sets){
    for(const string &data_set : data_sets) {

        CargoRoute cr = CargoRoute(data_set);
        cr.arcs_to_file(data_set);

        GurobiModel model = GurobiModel(data_set);
        model.all_paths_for_GurobiModel(data_set);

    }
}

unordered_map<string, pair<double, double>> run_danzig_wolfe(const vector<string>& data_sets){
    vector<double> times{};
    clock_t start;
    unordered_map<string, pair<double, double>> results;
    for(const string &data_set : data_sets) {
        start = clock();

        Dantzig_wolfe dantzig_wolfe = Dantzig_wolfe(CargoRoute(data_set));
        string file_prefix = "Result_DW_";
//        if(!is_desinged_route_added)
//            file_prefix += "noDesign_";
        double run_time = double(clock() - start)/CLOCKS_PER_SEC;
        dantzig_wolfe.output_result(file_prefix + data_set
                , run_time);
        results[data_set] = make_pair(dantzig_wolfe.getBestSol()->P, run_time);
    }
    for(const auto &data_set : data_sets){
        cout << data_set<< "\tSol : " << results[data_set].first << "\tTime : " << results[data_set].second << "\n" ;
    }
    return results;
}

unordered_map<string, pair<double, double>> run_gurobi_model(const vector<string>& data_sets){
    vector<double> times{};
    clock_t start;
    unordered_map<string, pair<double, double>> results;
    for(const string &data_set : data_sets) {
        start = clock();

        GurobiModel model = GurobiModel(data_set);
        model.Run_GurobiModel(data_set);
        string file_prefix = "Result_DW_";
//        if(!is_desinged_route_added)
//            file_prefix += "noDesign_";
        double run_time = double(clock() - start)/CLOCKS_PER_SEC;
        model.output_result("Result_model_" + data_set, MIN(time_limit_for_gurobi, double(clock() - start)));
        results[data_set] = make_pair(model.getBestSol()->P, run_time);
    }
    for(const auto &data_set : data_sets){
        cout << data_set<< "\tSol : " << results[data_set].first << "\tTime : " << results[data_set].second << "\n" ;
    }
    return results;
}
void compare_gap(vector<string> data_sets){
    unordered_map<string, pair<double, double>> grb_results, algo_results;
    grb_results = run_gurobi_model(data_sets);
    algo_results = run_danzig_wolfe(data_sets);

    for(const auto &data_set : data_sets){
        cout << data_set<< "\t grb_Sol : " << grb_results[data_set].first << "\tgrb_Time : " << grb_results[data_set].second
             << "\tAlgo_Sol : " << algo_results[data_set].first << "\tAlgo_Time : " << algo_results[data_set].second << "\n";
    }
}

int main() {
    vector<string> data_sets{"A1", "A2", "A3", "A4", "A5"};
//    run_gurobi_model(data_sets);
//    run_danzig_wolfe(data_sets);
    compare_gap(data_sets);
    return 0;
}