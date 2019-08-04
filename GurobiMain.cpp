//
// Created by Ashee on 2019/7/16.
//
#include <iostream>
#include "src/GurobiModel.h"
#include <ctime>

int main() {

    vector<string> data_sets{"A1"};

    for(const string &data_set : data_sets) {
        clock_t start = clock();
        GurobiModel model = GurobiModel(data_set);
        model.output_result("Result_model_" + data_set, double(clock() - start));
    }
    return 0;
}