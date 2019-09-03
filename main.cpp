#include <iostream>

#include "src/Dantzig_wolfe.h"
#include "src/GurobiModel.h"
#include <ctime>

using namespace std;

bool is_designed_route_added = true;
bool iter_added = false;


void generate_paths_arcs(vector<string> data_sets);;
void compare_grb_algo(vector<string> data_sets);
void compare_designed_route_added(vector<string> data_sets);
void compare_iter_added(vector<string> datasets);
unordered_map<string, pair<double, double>> run_danzig_wolfe(const vector<string>& data_sets);
unordered_map<string, pair<double, double>> run_danzig_wolfe_without_designed(const vector<string>& data_sets);
unordered_map<string, pair<double, double>> run_gurobi_model(const vector<string>& data_sets);

void generate_paths_arcs(vector<string> data_sets){
    for(const string &data_set : data_sets) {

        CargoRoute cr = CargoRoute(data_set);
        cr.arcs_to_file(data_set);

        GurobiModel model = GurobiModel(data_set);
        model.all_paths_for_GurobiModel(data_set);
    }
}

void compare_grb_algo(vector<string> data_sets){
    unordered_map<string, pair<double, double>> grb_results, algo_results;
    grb_results = run_gurobi_model(data_sets);
    algo_results = run_danzig_wolfe(data_sets);

    for(const auto &data_set : data_sets){
        cout << data_set<< "\t grb_Sol : " << grb_results[data_set].first << "\tgrb_Time : " << grb_results[data_set].second
             << "\tAlgo_Sol : " << algo_results[data_set].first << "\tAlgo_Time : " << algo_results[data_set].second << "\t"
             << "Gap : " << (grb_results[data_set].first - algo_results[data_set].first) / algo_results[data_set].first * 100 << "%""\n";
    }
}

void compare_designed_route_added(vector<string> data_sets){
    unordered_map<string, pair<double, double>> without_results, with_results;
    with_results = run_danzig_wolfe(data_sets);
    without_results = run_danzig_wolfe_without_designed(data_sets);

    for(const auto &data_set : data_sets){
        cout << data_set<< "\t without_Sol : " << without_results[data_set].first << "\tgrb_Time : " << without_results[data_set].second
             << "\twith_Sol : " << with_results[data_set].first << "\twith_Time : " << with_results[data_set].second << "\t"
             << "Gap : " << (without_results[data_set].first - with_results[data_set].first) / with_results[data_set].first * 100 << "%""\n";
    }
}

void compare_iter_added(vector<string> data_sets){
    unordered_map<string, pair<double, double>> iter_added_results, no_iter_added_results;
    iter_added = true;
    iter_added_results = run_danzig_wolfe(data_sets);
    iter_added = false;
    no_iter_added_results = run_danzig_wolfe(data_sets);


    for(const auto &data_set : data_sets){
        cout << data_set<< "\t iter_added_Sol : " << iter_added_results[data_set].first << "\titer_added_Time : " << iter_added_results[data_set].second
             << "\tno_iter_added_Sol : " << no_iter_added_results[data_set].first << "\tno_iter_added_Time : " <<no_iter_added_results[data_set].second << "\t"
             << "Gap : " << (iter_added_results[data_set].first - no_iter_added_results[data_set].first) / no_iter_added_results[data_set].first * 100 << "%""\n";
    }
}

unordered_map<string, pair<double, double>> run_danzig_wolfe(const vector<string>& data_sets){
    vector<double> times{};
    clock_t start;
    is_designed_route_added = true;
    unordered_map<string, pair<double, double>> results;
    for(const string &data_set : data_sets) {
        start = clock();

        Dantzig_wolfe dantzig_wolfe = Dantzig_wolfe(CargoRoute(data_set));
        string file_prefix = "Result_DW_";
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
        double run_time = double(clock() - start)/CLOCKS_PER_SEC;
        model.output_result("Result_model_" + data_set, MIN(time_limit_for_gurobi, run_time));
        results[data_set] = make_pair(model.getBestSol()->P, run_time);
    }
    for(const auto &data_set : data_sets){
        cout << data_set<< "\tSol : " << results[data_set].first << "\tTime : " << results[data_set].second << "\n" ;
    }
    return results;
}


unordered_map<string, pair<double, double>> run_danzig_wolfe_without_designed(const vector<string>& data_sets){
    vector<double> times{};
    clock_t start;
    is_designed_route_added = false;
    unordered_map<string, pair<double, double>> results;
    for(const string &data_set : data_sets) {
        start = clock();

        Dantzig_wolfe dantzig_wolfe = Dantzig_wolfe(CargoRoute(data_set));
        string file_prefix = "Result_DW_";
        file_prefix += "noDesign_";
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


int main() {
    vector<string> data_sets{"A1", "A2", "A3", "A4", "A5"};
    vector<string> data_sets2{"A1", "A2", "A3"};
    vector<string> data_sets3{"A", "B", "C", "D", "E"};
//    run_gurobi_model(data_sets3);
//    run_danzig_wolfe(data_sets3);

//    compare_grb_algo(data_sets2);
//    compare_designed_route_added(data_sets3);
    compare_iter_added(data_sets3);
    return 0;
}