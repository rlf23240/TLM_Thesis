#include <iostream>

#include "src/Dantzig_wolfe.h"
#include <ctime>

using namespace std;
int main() {
    vector<string> data_sets{"E"};
    vector<double> times{};
    for(const string &data_set : data_sets) {
        clock_t start = clock();
        Dantzig_wolfe dantzig_wolfe = Dantzig_wolfe(CargoRoute(data_set));
        dantzig_wolfe.output_result("Result_" + data_set);
        times.push_back(double(clock() - start));
    }
    for(int i = 0; i < times.size(); i++){
        cout << data_sets[i] << " : " << times[i] << " sec." << endl;
    }
    return 0;
}