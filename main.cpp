#include <iostream>

#include "src/Dantzig_wolfe.h"
#include "src/GurobiModel.h"
#include"sstream"
#include <ctime>

using namespace std;
int main() {
    vector<string> data_sets{"B"};
    vector<double> MU_THRESHOLD_GROUP{0, 0.1, 0.5};
    vector<int> BP_ITER_GROUP{10, 30, 50};
    vector<double> times{};
    clock_t start;
    for(const string &data_set : data_sets) {
        for (const double threshold : MU_THRESHOLD_GROUP) {
            MU_THRESHOLD = threshold;
            for(const int iter : BP_ITER_GROUP) {
                MAX_BP_ITER = iter;
                start = clock();
                ostringstream oss;
                oss << threshold;

                Dantzig_wolfe dantzig_wolfe = Dantzig_wolfe(CargoRoute(data_set));
                dantzig_wolfe.output_result("Result_DW_" + data_set
                + "_iter" + to_string(iter) + "_thres"
                + oss.str(), double(clock() - start));
            }
        }
    }
    return 0;
}