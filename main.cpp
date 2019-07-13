#include <iostream>

#include "src/Dantzig_wolfe.h"


using namespace std;
int main() {
    Dantzig_wolfe dantzig_wolfe = Dantzig_wolfe(CargoRoute("A1"));
    return 0;
}