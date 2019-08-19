//
// Created by 黃朕庭 on 2019-08-19.
//

#include <iostream>
#include "src/GurobiModel.h"
#include "src/Dantzig_wolfe.h"

int main() {

    vector<string> data_sets{"A1", "A2"};

    for(const string &data_set : data_sets) {

        CargoRoute cr = CargoRoute(data_set);
        cr.arcs_to_file(data_set);

        GurobiModel model = GurobiModel(data_set);
        model.all_paths_for_GurobiModel(data_set);

    }
    return 0;
}
