#include <iostream>

#include "src/Dantzig_wolfe.h"
#include "src/GurobiModel.h"
#include <ctime>

using namespace std;
int main() {
    EntireNetwork network = EntireNetwork("A");
    vector<string> data_sets{"A"};
    vector<double> times{};
    clock_t start;
    for(const string &data_set : data_sets) {
        start = clock();
        Dantzig_wolfe dantzig_wolfe = Dantzig_wolfe(CargoRoute(data_set));
        dantzig_wolfe.output_result("Result_DW_iter50_" + data_set, double(clock() - start));
    }
    return 0;
}