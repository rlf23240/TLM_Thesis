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
        start = clock();
        ostringstream oss;
        oss << MU_THRESHOLD;

        Dantzig_wolfe dantzig_wolfe = Dantzig_wolfe(CargoRoute(data_set));
        dantzig_wolfe.output_result("Result_DW_" + data_set
        , double(clock() - start));
    }
    return 0;
}