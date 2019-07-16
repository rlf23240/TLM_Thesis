//
// Created by Ashee on 2019/7/16.
//

#ifndef TLM_THESIS_GUROBIMODEL_H
#define TLM_THESIS_GUROBIMODEL_H

#include "EntireNetwork.h"
#include "param.h"

class GurobiModel {
public:
    explicit GurobiModel(const EntireNetwork &network);

private:
    EntireNetwork network;
    vector<Route*> all_ship_routes;
    vector<Route*> all_flight_routes;

};


#endif //TLM_THESIS_GUROBIMODEL_H
