#include <iostream>

#include "src/AirNetwork.h"
#include "src/SeaNetwork.h"
//#include "gurobi_c++.h"
using namespace std;
int main() {
    AirNetwork air_network = AirNetwork("../Data/air10");
    cout << endl ;
    SeaNetwork sea_network = SeaNetwork("../Data/sea10");
    return 0;
}