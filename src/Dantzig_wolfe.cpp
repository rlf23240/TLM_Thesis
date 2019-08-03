//
// Created by Ashee on 2019/7/8.
//

#include "Dantzig_wolfe.h"

Dantzig_wolfe::Dantzig_wolfe(const CargoRoute &cargoRoute) : cargoRoute(cargoRoute) {
    vector<double> shadow_price;
    Solution* sol;

    sol = this->cargoRoute.run_bp();
    cout << *sol;
    P.push_back(sol->P);
    append_R_column(sol->r);
    shadow_price = Run_Dantzig_wolfe();
    update_arc_by_pi(shadow_price);
    solutions.push_back(sol);

    while(true){
        sol = this->cargoRoute.run_bp();
        cout << *sol;
        if(end_condition(shadow_price)){
            break;
        }
        P.push_back(this->cargoRoute.get_P_value());
        stop_iter++;
        append_R_column(this->cargoRoute.get_r_column());
        shadow_price = Run_Dantzig_wolfe();
        update_arc_by_pi(shadow_price);
        solutions.push_back(sol);
    }
    Final_result();

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

        auto * lambda = new GRBVar[n];
        auto * v = new GRBVar[m];
        GRBVar w = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "w");

        for (int i = 0; i < n; i++) {
                lambda[i] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "lambda");
        }
        for (int i = 0; i < m; i++) {
            v[i] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "v");
        }

        GRBLinExpr obj = GRBLinExpr();
        for (int i = 0; i < m; i++) {
            obj += (v[i] + w);
        }
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
            cons -= (v[row]);
            model.addConstr(cons <= 0, "c" + to_string(row));
        }
        GRBLinExpr cons = GRBLinExpr();
        for (int i = 0; i < n; i++) {
            cons += lambda[i];
        }
        model.addConstr(cons + w == 1, "c" + to_string(m));
        model.optimize();


//        cout << endl;
//        cout << "v : ";
//        for(int i = 0; i < m ; i++){
//            cout << v[i].get(GRB_DoubleAttr_X) << " ";
//        }
//        cout << endl;
//        cout << "w : " <<w.get(GRB_DoubleAttr_X) << endl;



//        cout << " R\tShadow Price : " << endl;
        vector<double> pi = vector<double>();
        for (int i = 0; i < m; i++) {
            pi.push_back(model.getConstr(i).get(GRB_DoubleAttr_Pi));
            bool isPrinted = (accumulate(R[i].begin(), R[i].end(), 0) != 0);
            for(int j = 0 ; j < n ; j++) {
//               cout << R[i][j] << "\t" ;
            }
//            if(isPrinted) cout << pi.back() << endl;
        }
        cout << endl;

        sigma = model.getConstr(m).get(GRB_DoubleAttr_Pi);
        cout << "sigma : " << sigma << endl;

        cout << "lambda : " ;
        for(int i = 0; i < n; i++)
            cout << lambda[i].get(GRB_DoubleAttr_X) << " ";
        cout << endl;
        cout << "P : " ;
        for (double i : P) {
            cout << i << " " ;
        }
        cout << endl << "iter : " << stop_iter << endl;

        if(!model_result.empty() && (model.get(GRB_DoubleAttr_ObjVal) - model_result.back()) / model_result.back() > DW_STOP_THRESHOLD){
            stop_iter = 0;
        }
        model_result.push_back(model.get(GRB_DoubleAttr_ObjVal));


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

void Dantzig_wolfe::Final_result() {
    try{
        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);

        int n = P.size(); //size of column
        int m = R.size(); //size of constraints
        int bigM = 99999;

        auto * lambda = new GRBVar[n];
        auto * v = new GRBVar[m];

        for (int i = 0; i < n; i++) {
            lambda[i] = model.addVar(0.0, 1.0, 0.0, GRB_INTEGER, "lambda");
        }
        for (int i = 0; i < m; i++) {
            v[i] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "v");
        }

        GRBLinExpr obj = GRBLinExpr();
        for (int i = 0; i < m; i++) {
            obj += (v[i]);
        }
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
            cons -= (v[row]);
            model.addConstr(cons <= 0, "c" + to_string(row));
        }
        GRBLinExpr cons = GRBLinExpr();
        for (int i = 0; i < n; i++) {
            cons += lambda[i];
        }
        model.addConstr(cons == 1, "c" + to_string(m));
        model.optimize();

        cout << "========================FINAL RESULT========================" << endl;
        cout << "lambda : " ;
        for(int i = 0; i < n; i++)
            cout << lambda[i].get(GRB_DoubleAttr_X) << " ";
        cout << endl;
        cout << "P : " ;
        for (double p : P) {
            cout << p << " " ;
        }
        cout << endl;

        for(int i = 0; i < P.size(); i++){
            if(lambda[i].get(GRB_DoubleAttr_X) == 1){
                cout << *solutions[i];
                best_sol = solutions[i];
            }
        }

    }catch (GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    }
    catch (...) {
        cout << "Exception during optimization" << endl;
    }
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

    SeaNetwork sea_network = networks.getSea_network();
    for(int i = 0; i < sea_arc_pair.size(); i++){
        if(pi[i] != 0){
            int start_idx = sea_arc_pair[i].first;
            int end_idx = sea_arc_pair[i].second;
            Point start = networks.idx_to_point(start_idx);
            Point end = networks.idx_to_point(end_idx);
            for(auto &arc : sea_network.nodes[(char) start.node +65][start.time]->out_arcs){
                if((int) arc->end_node->getName()[0] - 65 == end.node){
                    arc->fixed_cost -= pi[i];  //update cost
                }
            }
            if(networks.arcs.find(start_idx) != networks.arcs.end() && networks.arcs[start_idx].find(end_idx) != networks.arcs[start_idx].end()){ //found arc in entirenetwork
                networks.arcs[start_idx][end_idx]->minus_fixed_profit(pi[i]); //update profit
//                cout << start << " " << end << " "<< networks.arcs[start_idx][end_idx]->fixed_profit << endl;
            }
        }
    }



    AirNetwork air_network = networks.getAir_network();
    for(unsigned long long int i = 0; i <  air_arc_pair.size(); i++){
        if(pi[sea_arc_pair.size() + i] != 0){
            int start_idx = air_arc_pair[i].first;
            int end_idx = air_arc_pair[i].second;
            Point start = networks.idx_to_point(air_arc_pair[i].first);
            Point end = networks.idx_to_point(air_arc_pair[i].second);
            for(auto &arc : air_network.nodes[(char) start.node +65][start.time % (7 * TIME_SLOT_A_DAY)]->out_arcs){
                if((int) arc->end_node->getName()[0] - 65 == end.node){
                    arc->fixed_cost -= pi[sea_arc_pair.size() + i];
                }
            }
            if(networks.arcs.find(start_idx) != networks.arcs.end() && networks.arcs[start_idx].find(end_idx) != networks.arcs[start_idx].end()){ //found arc in entirenetwork
                networks.arcs[start_idx][end_idx]->minus_fixed_profit(pi[sea_arc_pair.size() + i]); //update profit
//                cout << start << " " << end << " "<< networks.arcs[start_idx][end_idx]->fixed_profit << endl;
            }
        }
    }


    for(unsigned long long int i = 0; i < air_arc_pair.size(); i++){
        if(pi[sea_arc_pair.size() + air_arc_pair.size() + i] != 0){
            int start_idx = air_arc_pair[i].first;
            int end_idx = air_arc_pair[i].second;
            Point start = networks.idx_to_point(air_arc_pair[i].first);
            Point end = networks.idx_to_point(air_arc_pair[i].second);
            for(auto &arc : air_network.nodes[(char) start.node +65][start.time % (7 * TIME_SLOT_A_DAY)]->out_arcs){
                if((int) arc->end_node->getName()[0] - 65 == end.node){
                    arc->fixed_cost -= pi[sea_arc_pair.size() + air_arc_pair.size() + i];
                }
            }
            if(networks.arcs.find(start_idx) != networks.arcs.end() && networks.arcs[start_idx].find(end_idx) != networks.arcs[start_idx].end()){ //found arc in entirenetwork
                networks.arcs[start_idx][end_idx]->minus_fixed_profit(pi[sea_arc_pair.size() + air_arc_pair.size() + i]); //update profit
//                cout << start << " " << end << " "<< networks.arcs[start_idx][end_idx]->fixed_profit << endl;
            }
        }
    }

//    air_network.run_algo();
//    sea_network.run_algo();
//    air_network.generate_designed_flight();
//    this->cargoRoute.getNetworks().setAir_network(air_network);
//    sea_network.generate_designed_ship();
//    this->cargoRoute.getNetworks().setSea_network(sea_network);
    this->cargoRoute.getNetworks().generate_new_routes();
    this->cargoRoute.rebuild_entire_network();
}

bool Dantzig_wolfe::end_condition(vector<double> pi) {
    if(pi.size() != R.size()){
        cout << "Fail" <<endl;
        exit(1);
    }
    if(stop_iter > MAX_DW_ITER) return true;

    double val = 0;

    double P_val = cargoRoute.get_P_value();
    val += P_val;
    vector<double> r = cargoRoute.get_r_column();
    for(int i = 0; i < r.size(); i++){
        val -= r[i] * pi[i];
    }
    val -= sigma;
//    cout << "----------------P_bar : " << val << "-------------------"<< endl;

    return val < 0;
}

void Dantzig_wolfe::output_result(string name, double run_time) {
    if(best_sol == nullptr){
        cout << "Fail to output results" << endl;
        exit(1);
    }
    best_sol->to_file(name, run_time);
}






