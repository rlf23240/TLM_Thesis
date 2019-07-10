//
// Created by Ashee on 2019/7/8.
//

#ifndef TLM_THESIS_DANTZIG_WOLFE_H
#define TLM_THESIS_DANTZIG_WOLFE_H

#include "param.h"
#include "CargoRoute.h"

class Dantzig_wolfe {
public:
    explicit Dantzig_wolfe(const CargoRoute &cargoRoute);

private:
    CargoRoute cargoRoute;
    vector<double> P;
    vector<vector<double>> R;
    void append_R_column(vector<double> r_column);
};


#endif //TLM_THESIS_DANTZIG_WOLFE_H
