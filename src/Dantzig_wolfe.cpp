//
// Created by Ashee on 2019/7/8.
//

#include "Dantzig_wolfe.h"

Dantzig_wolfe::Dantzig_wolfe(const CargoRoute &cargoRoute) : cargoRoute(cargoRoute) {
    for(int i = 0; i < 1; i++){
        P.push_back(this->cargoRoute.get_P_value());
        cout << "obj : " << this->cargoRoute.getObjVal() << endl;
        append_R_column(this->cargoRoute.get_r_column());

        vector<double> shadow_price = Run_Dantzig_wolfe();
        update_arc_by_pi(shadow_price);
    }
}

void Dantzig_wolfe::append_R_column(vector<double> r_column) {
    if(R.empty()){
        for(int r = 0; r < r_column.size(); r++){
            R.emplace_back();
        }
    }

    for(int row = 0; row < r_column.size(); row++){
        R[row].push_back(r_column[row]);
    }
}

vector<double> Dantzig_wolfe::Run_Dantzig_wolfe() {
    try{
        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);

        int n = P.size(); //size of column
        int m = R.size(); //size of constraints
        int bigM = 99999;


        cout << "R : ";
        for (auto &row : R) {
            for (double val : row) {
                if (val != 0)
                    cout << val << endl;
            }
//            cout << endl;
        }
//        cout << "P : ";
//        for (double i : P) {
//            cout << i << " ";
//        }
//        cout << endl;

        GRBVar* lambda = new GRBVar[n];
        GRBVar* v = new GRBVar[m];
        GRBVar w = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "w");
        for (int i = 0; i < n; i++) {
            lambda[i] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "lambda");
        }
        for (int i = 0; i < m; i++) {
            v[i] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "v");
        }

        GRBLinExpr obj = GRBLinExpr();
        for (int i = 0; i < m; i++) {
            obj += (v[i]);
        }
        obj += w;
        obj *= (-bigM);
        for (int i = 0; i < n; i++) {
            obj += P[i] * lambda[i];
        }
        model.setObjective(obj, GRB_MAXIMIZE);


        for (int row = 0; row < m; row++) {
            GRBLinExpr cons = GRBLinExpr();
            for (int col = 0; col < n; col++) {
                cons +=  R[row][col] * lambda[col];
            }
            cons += (v[row]) - w;
            model.addConstr(cons <= 0, "c" + to_string(row));
        }
        GRBLinExpr cons = GRBLinExpr();
        for (int i = 0; i < n; i++) {
            cons += lambda[i];
        }
        model.addConstr(cons == 1, "c" + to_string(m));
        model.optimize();

        cout << "lambda : " ;
        for(int i = 0; i < n; i++)
            cout << lambda[i].get(GRB_DoubleAttr_X) << " ";
        cout << endl;
        cout << "v : ";
        for(int i = 0; i < m ; i++){
            cout << v[i].get(GRB_DoubleAttr_X) << " ";
        }
        cout << endl;
        cout << "w : " <<w.get(GRB_DoubleAttr_X) << endl;


        cout << "Shadow Price : ";
        vector<double> pi = vector<double>();
        for (int i = 0; i < m; i++) {
            pi.push_back(model.getConstr(i).get(GRB_DoubleAttr_Pi));
            cout << pi.back() << " ";
        }

//
        double delta;
        delta = model.getConstr(m).get(GRB_DoubleAttr_Pi);
        cout << "delta : " << delta;

        return pi;


    }catch (GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    }
    catch (...) {
        cout << "Exception during optimization" << endl;
    }
    return vector<double>{};
}

void Dantzig_wolfe::update_arc_by_pi(vector<double> pi) {
    vector<pair<int, int>> sea_arc_pair = cargoRoute.getSea_arc_pairs();
    vector<pair<int, int>> air_arc_pair = cargoRoute.getAir_arc_pairs();
    EntireNetwork networks = cargoRoute.getNetworks();

    if(sea_arc_pair.size() + air_arc_pair.size() * 2 != pi.size()){
        cout << "update arcs fail !!!";
        exit(1);
    }
    cout << endl;
    for(int i = 0; i < sea_arc_pair.size(); i++){
        if(pi[i] != 0){
            Point start = networks.idx_to_point(sea_arc_pair[i].first);
            Point end = networks.idx_to_point(sea_arc_pair[i].second);

            SeaNetwork sea_network = networks.getSea_network();
            string end_node_name = sea_network.nodes[(char) start.node +65][start.time]->out_arcs[0]->end_node->getName();
            cout << start << endl << end << endl << end_node_name << endl;

        }
    }

    for(unsigned long long int i = 0; i <  air_arc_pair.size(); i++){
        if(pi[sea_arc_pair.size() + i] != 0){
            Point start = networks.idx_to_point(air_arc_pair[i].first);
            Point end = networks.idx_to_point(air_arc_pair[i].second);

            AirNetwork air_network = networks.getAir_network();
            cout << air_network.nodes[(char) start.node +65][start.time % (7 * TIME_SLOT_A_DAY)]->getName() << endl;
            string end_node_name = air_network.nodes[(char) start.node +65][start.time % (7 * TIME_SLOT_A_DAY) ]->out_arcs[2]->end_node->getName();
            for(auto &arc : air_network.nodes[(char) start.node +65][start.time % (7 * TIME_SLOT_A_DAY)]->out_arcs){
                if((int) arc->end_node->getName()[0] - 65 == end.node){
                    cout << arc->cost << endl;
                    arc->cost += pi[sea_arc_pair.size() + i];
                    cout << arc->cost << endl;
                }
            }
            cout << start << endl << end << endl << end_node_name << endl;
            air_network.run_algo();
            cout << air_network.getFlights()[0].routes[0];
        }
    }

    for(unsigned long long int i = 0; i < air_arc_pair.size(); i++){
        if(pi[ sea_arc_pair.size() + air_arc_pair.size() + i] != 0){
            Point start = networks.idx_to_point(air_arc_pair[i].first);
            Point end = networks.idx_to_point(air_arc_pair[i].second);

            AirNetwork air_network = networks.getAir_network();
            string end_node_name = air_network.nodes[(char) start.node +65][start.time]->out_arcs[0]->end_node->getName();
            cout << start << endl << end << endl << end_node_name << endl;

        }
    }
}

