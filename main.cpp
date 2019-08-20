#include <iostream>

#include "src/Dantzig_wolfe.h"
#include "src/GurobiModel.h"
#include"sstream"
#include <ctime>

using namespace std;
int main() {
    vector<string> data_sets{"F"};
    vector<double> times{};
    clock_t start;
    for(const string &data_set : data_sets) {
//        CargoRoute cr = CargoRoute(data_set);
        start = clock();

        Dantzig_wolfe dantzig_wolfe = Dantzig_wolfe(CargoRoute(data_set));
        string file_prefix = "Result_DW_";
        if(!is_desinged_route_added)
            file_prefix += "noDesign_";
        dantzig_wolfe.output_result(file_prefix + data_set
        , double(clock() - start));
    }
    return 0;
}