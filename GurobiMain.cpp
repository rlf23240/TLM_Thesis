//
// Created by Ashee on 2019/7/16.
//
#include <iostream>
#include "src/GurobiModel.h"
#include <ctime>

int main() {

    vector<string> data_sets{"A1"};

    for(const string &data_set : data_sets) {
        GurobiModel model = GurobiModel(data_set);
    }
    return 0;
}