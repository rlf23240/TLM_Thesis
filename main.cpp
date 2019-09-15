#include <iostream>

#include "src/Dantzig_wolfe.h"
#include "src/GurobiModel.h"
#include <ctime>
#include "numeric"

using namespace std;

bool is_designed_route_added = true;
bool iter_added = true;
bool col_deletion = false;

void generate_paths_arcs(vector<string> data_sets);
void compare_grb_algo(vector<string> data_sets);
void compare_designed_route_added(vector<string> data_sets);
void compare_iter_added(vector<string> data_sets);
void compare_col_deletion(vector<string> data_sets);
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
    vector<double> gaps;

    for(const auto &data_set : data_sets){
        gaps.push_back((grb_results[data_set].first - algo_results[data_set].first) / algo_results[data_set].first * 100);
        cout << data_set<< "\t grb_Sol : " << grb_results[data_set].first << "\tgrb_Time : " << grb_results[data_set].second
             << "\tAlgo_Sol : " << algo_results[data_set].first << "\tAlgo_Time : " << algo_results[data_set].second << "\t"
             << "Gap : " << gaps.back() << "%""\n";
    }
    cout << "Max gap :" <<*max_element(gaps.begin(), gaps.end()) << "\t"
         <<"Min gap :" << *min_element(gaps.begin(), gaps.end()) << "\t"
         << "Average : " << accumulate(gaps.begin(), gaps.end(),0.0) /(double) gaps.size() << "\n";
}

void compare_designed_route_added(vector<string> data_sets){
    unordered_map<string, pair<double, double>> without_results, with_results;
    with_results = run_danzig_wolfe(data_sets);
    without_results = run_danzig_wolfe_without_designed(data_sets);
    vector<double> gaps;

    for(const auto &data_set : data_sets){
        gaps.push_back((without_results[data_set].first - with_results[data_set].first) / with_results[data_set].first * 100);
        cout << data_set<< "\t without_Sol : " << without_results[data_set].first << "\tgrb_Time : " << without_results[data_set].second
             << "\twith_Sol : " << with_results[data_set].first << "\twith_Time : " << with_results[data_set].second << "\t"
             << "Gap : " << gaps.back() << "%""\n";
    }
    cout << "Max gap :" <<*max_element(gaps.begin(), gaps.end()) << "\t"
         <<"Min gap :" << *min_element(gaps.begin(), gaps.end()) << "\t"
         << "Average : " << accumulate(gaps.begin(), gaps.end(),0.0) /(double) gaps.size() << "\n";
}

void compare_iter_added(vector<string> data_sets){
    is_designed_route_added = true;
    unordered_map<string, pair<double, double>> iter_added_results, no_iter_added_results;
    iter_added = true;
    iter_added_results = run_danzig_wolfe(data_sets);
    iter_added = false;
    no_iter_added_results = run_danzig_wolfe(data_sets);
    vector<double> gaps;

    for(const auto &data_set : data_sets){
        gaps.push_back((iter_added_results[data_set].first - no_iter_added_results[data_set].first) / no_iter_added_results[data_set].first * 100);
        cout << data_set<< "\t iter_added_Sol : " << iter_added_results[data_set].first << "\titer_added_Time : " << iter_added_results[data_set].second
             << "\tno_iter_added_Sol : " << no_iter_added_results[data_set].first << "\tno_iter_added_Time : " <<no_iter_added_results[data_set].second << "\t"
             << "Gap : " << gaps.back() << "%""\n";
    }
    cout << "Max gap :" <<*max_element(gaps.begin(), gaps.end()) << "\t"
         <<"Min gap :" << *min_element(gaps.begin(), gaps.end()) << "\t"
         << "Average : " << accumulate(gaps.begin(), gaps.end(),0.0) /(double) gaps.size() << "\n";
}

void compare_col_deletion(vector<string> data_sets){
    unordered_map<string, pair<double, double>> col_del_results, no_col_del_results;
    col_deletion = true;
    col_del_results = run_danzig_wolfe(data_sets);
    col_deletion = false;
    no_col_del_results = run_danzig_wolfe(data_sets);
    vector<double> gaps;

    for(const auto &data_set : data_sets){
        gaps.push_back((col_del_results[data_set].first - no_col_del_results[data_set].first) / no_col_del_results[data_set].first * 100);
        cout << data_set<< "\t col_del_Sol : " << no_col_del_results[data_set].first << "\tcol_del_Time : " << col_del_results[data_set].second
             << "\tno_col_del_Sol : " << col_del_results[data_set].first << "\tno_col_del_Time : " <<no_col_del_results[data_set].second << "\t"
             << "Gap : " << gaps.back() << "%""\n";
    }
    cout << "Max gap :" <<*max_element(gaps.begin(), gaps.end()) << "\t"
         <<"Min gap :" << *min_element(gaps.begin(), gaps.end()) << "\t"
         << "Average : " << accumulate(gaps.begin(), gaps.end(),0.0) /(double) gaps.size() << "\n";
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
    vector<string> data_sets2{"A4", "A5"};
    vector<string> data_sets3{"A", "B", "C", "D", "E"};
//    run_gurobi_model(data_sets3);
//    run_danzig_wolfe(data_sets3);
    vector<string> A1_sets{"A1_1","A1_2","A1_3","A1_4","A1_5","A1_6","A1_7","A1_8","A1_9","A1_10"};
    vector<string> A2_scvbhnbjkbets{"A2_1","A2_2","A2_3","A2_4","A2_5","A2_6","A2_7","A2_8","A2_9","A2_10"};
    vector<string> A3_sets{"A3_1","A3_2","A3_3","A3_4","A3_5"};

    compare_grb_algo(data_sets2);
//    compare_designed_route_added(data_sets3);
//    compare_iter_added(data_sets3);
//    compare_col_deletion(data_sets3);
//    return 0;
}