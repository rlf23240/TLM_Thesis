#include <iostream>

#include "src/AirNetwork.h"
#include "src/SeaNetwork.h"
using namespace std;
int main() {
    AirNetwork air_network = AirNetwork("../Data/air1");
    cout << endl ;
    SeaNetwork sea_network = SeaNetwork("../Data/sea1");
    return 0;
}