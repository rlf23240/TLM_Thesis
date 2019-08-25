//
// Created by Ashee on 2019/7/16.
//
#include <iostream>
#include "src/GurobiModel.h"
#include <ctime>

int main() {

    vector<string> data_sets{"A4", "A5"};

    for(const string &data_set : data_sets) {
        clock_t start;
        GurobiModel model = GurobiModel(data_set);
        model.Run_GurobiModel(data_set);
        model.output_result("Result_model_" + data_set, MIN(time_limit_for_gurobi, double(clock() - start)));
    }
    return 0;
}