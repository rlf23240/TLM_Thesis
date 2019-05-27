#include <iostream>

#include "src/Network.h"
using namespace std;
int main() {
    Network network = Network();
    network.read_data("../Data/Data1");
    return 0;
}