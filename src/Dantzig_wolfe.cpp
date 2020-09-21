//
// Created by Ashee on 2019/7/8.
//

#include "Dantzig_wolfe.h"

Dantzig_wolfe::Dantzig_wolfe(const CargoRoute &cargoRoute): cargoRoute(cargoRoute) {
    vector<double> shadow_price;
    Solution* sol;
    
    sol = this->cargoRoute.run_bp();
    
    #ifdef DEBUG_DW_ITER
    DW_ITER_LOG(*sol);
    #endif
    
    P.push_back(sol->P);
        
    append_R_column(sol->r);
    shadow_price = Run_Dantzig_wolfe();
    update_arc_by_pi(shadow_price);
    solutions.push_back(sol);
    
    #ifdef DEBUG_DW_ITER_DUAL_VALUES
    DW_ITER_LOG("dual: ");
    
    stringstream dual_debug_ss;
    for (auto p: shadow_price) {
        dual_debug_ss << p << ",";
    }
    dual_debug_ss << endl;
    
    TLMLOG(NULL, dual_debug_ss.str());
    #endif

    while(true) {
        sol = this->cargoRoute.run_bp();

        #ifdef DEBUG_DW_ITER
        DW_ITER_LOG(*sol);
        #endif
        
        if(end_condition(shadow_price)){
            break;
        }
        P.push_back(this->cargoRoute.get_P_value());
        
        stop_iter++;
        append_R_column(this->cargoRoute.get_r_column());
        shadow_price = Run_Dantzig_wolfe();
        
        #ifdef DEBUG_DW_ITER_DUAL_VALUES
        DW_ITER_LOG("dual: ");
        
        stringstream dual_debug_ss;
        for (auto p: shadow_price) {
            dual_debug_ss << p << ",";
        }
        dual_debug_ss << endl;
        
        TLMLOG(NULL, dual_debug_ss.str());
        #endif
        
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
        
        #ifdef DEBUG_GUROBI_DW
            model.set(GRB_IntParam_OutputFlag, true);
        #else
            model.set(GRB_IntParam_OutputFlag, false);
        #endif
        
        // Size of column.
        int n = (int)P.size();
        
        // Size of constraints.
        // This should be equal #(sea arcs) + 2*#(air arcs).
        int m = (int)R.size();
        cout << "m : " << m << endl;
        
        double bigM = M_value;

        auto * lambda = new GRBVar[n];
        auto * v = new GRBVar[m];
        GRBVar w = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "w");

        for (int i = 0; i < n; i++) {
            lambda[i] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "lambda_" + to_string(i));
        }
        for (int i = 0; i < m; i++) {
            v[i] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "v_" + to_string(i));
        }

        GRBLinExpr obj = GRBLinExpr();
        for (int i = 0; i < m; i++) {
            obj += v[i];
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
            cons -= (v[row]);
            model.addConstr(cons <= 0, "R_lambda_" + to_string(row));
        }
        
        GRBLinExpr cons = GRBLinExpr();
        for (int i = 0; i < n; i++) {
            cons += lambda[i];
        }
        
        model.addConstr(cons + w == 1, "lambda_w");
        
        model.optimize();
        
        vector<double> pi = vector<double>();
        for (int i = 0; i < m; i++) {
            GRBConstr cons = model.getConstrByName("R_lambda_" + to_string(i));
            pi.push_back(cons.get(GRB_DoubleAttr_Pi));
        }

        sigma = model.getConstrByName("lambda_w").get(GRB_DoubleAttr_Pi);
        
        double improvement = 0.0;
        if (!model_result.empty()) {
            improvement = (model.get(GRB_DoubleAttr_ObjVal) - model_result.back()) / model_result.back();
        }
        
        if(!model_result.empty() && improvement > DW_STOP_THRESHOLD){
            stop_iter = 0;
        }
        model_result.push_back(model.get(GRB_DoubleAttr_ObjVal));
        
        previous_lambda = vector<double>();
        for(int i = 0; i < n; i++) {
            previous_lambda.push_back(lambda[i].get(GRB_DoubleAttr_X));
        }
        
        #ifdef DEBUG_DW_ITER
            DW_ITER_LOG("Stop iter: " << stop_iter << "/" << MAX_DW_ITER);
            DW_ITER_LOG("Improvment: " << improvement*100.0 << "%/" << DW_STOP_THRESHOLD << "%");
        #endif

        #ifdef DEBUG_DW_ITER_THETA_AND_SIGMA
            DW_ITER_LOG("v :");
        
            stringstream v_debug_ss;
            for(int i = 0; i < m ; i++){
                v_debug_ss << v[i].get(GRB_DoubleAttr_X) << " ";
            }
            v_debug_ss << endl;
            TLMLOG(NULL, v_debug_ss.str());
            
            DW_ITER_LOG("w : " << w.get(GRB_DoubleAttr_X));
            DW_ITER_LOG("sigma : " << sigma);
        #endif
        
        #ifdef DEBUG_DW_ITER_R_MATRIX
            DW_ITER_LOG("R\tShadow Price: ");
            stringstream r_matrix_debug_ss;
            for (int i = 0; i < m; ++i) {
                for (int j = 0; j < n; ++j) {
                    r_matrix_debug_ss << R[i][j] << "\t";
                }
                r_matrix_debug_ss << endl;
            }
        
            TLMLOG(NULL, r_matrix_debug_ss.str());
        #endif
        
        #ifdef DEBUG_DW_ITER_LAMBDA
            DW_ITER_LOG("lambda: ");
        
            stringstream lambda_debug_ss;
            for(int i = 0; i < n; i++) {
                lambda_debug_ss << lambda[i].get(GRB_DoubleAttr_X) << " ";
            }
            TLMLOG(NULL, lambda_debug_ss.str());
        #endif
        
        #ifdef DEBUG_DW_ITER_SOL_OBJS
            DW_ITER_LOG("P: ");
            
            stringstream pi_debug_ss;
            for (double i: P) {
                pi_debug_ss << i << " " ;
            }
            pi_debug_ss << endl;
        
            TLMLOG(NULL, pi_debug_ss.str());
        #endif
        
        #ifdef DEBUG_DW_ITER_SOL
            for (int i = 0; i < solutions.size(); ++i) {
                cout << i << endl;
                cout << *(solutions[i]) << endl;
            }
        #endif

        return pi;

    } catch (GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    }
    catch (...) {
        cout << "Exception during optimization" << endl;
    }
    return vector<double>{};
}

// TODO: Is this should be same as run_dw?
void Dantzig_wolfe::Final_result() {
    // Find a lambda equals 1.
    for(int i = 0; i < P.size(); i++){
        if(previous_lambda[i] == 1){
            cout << "========================FINAL RESULT========================" << endl;
            cout << *solutions[i];
            best_sol = solutions[i];
            
            return;
        }
    }
    
    // Else we need to solve it again with some other constrain.
    resolve_lambda();
    
    Final_result();
}

void Dantzig_wolfe::resolve_lambda() {
    try{
        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);
        
        // Size of column.
        int n = (int)P.size();
        
        // Size of constraints.
        // This should be equal #(sea arcs) + 2*#(air arcs).
        int m = (int)R.size();
        cout << "m : " << m << endl;
        
        double bigM = M_value;

        auto * lambda = new GRBVar[n];
        auto * v = new GRBVar[m];
        GRBVar w = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "w");

        for (int i = 0; i < n; i++) {
            lambda[i] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, "lambda_" + to_string(i));
        }
        for (int i = 0; i < m; i++) {
            v[i] = model.addVar(0.0, GRB_INFINITY, 0.0, GRB_CONTINUOUS, "v_" + to_string(i));
        }
        
        auto arc_usage = vector<GRBVar>();
        for (int i = 0; i < cargoRoute.num_sea_arc() + cargoRoute.num_air_arc(); ++i) {
            arc_usage.push_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "arc_usage_" + to_string(i)));
        }

        GRBLinExpr obj = GRBLinExpr();
        for (int i = 0; i < m; i++) {
            obj += v[i];
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
            cons -= (v[row]);
            model.addConstr(cons <= 0, "R_lambda_" + to_string(row));
        }
        
        GRBLinExpr cons = GRBLinExpr();
        for (int i = 0; i < n; i++) {
            cons += lambda[i];
        }
        
        model.addConstr(cons + w == 1, "lambda_w");
        
        // Arc usage constrain with lambda.
        auto arc_usage_expr = vector<GRBLinExpr>();
        for (int i = 0; i < cargoRoute.num_sea_arc() + cargoRoute.num_air_arc(); ++i) {
            arc_usage_expr.push_back(GRBLinExpr());
        }
        
        for (int sol_index = 0; sol_index < solutions.size(); ++sol_index) {
            Solution *sol = solutions[sol_index];
            
            for (int i = 0; i < cargoRoute.num_sea_arc(); ++i) {
                pair<int, int> arc = cargoRoute.getSea_arc_pairs()[i];
                if (sol->arc_usage_in_design[arc]) {
                    arc_usage_expr[i] += lambda[sol_index];
                }
            }
            
            for (int i = 0; i < cargoRoute.num_air_arc(); ++i) {
                int index = cargoRoute.num_sea_arc() + i;
                
                pair<int, int> arc = cargoRoute.getAir_arc_pairs()[i];
                
                if (sol->arc_usage_in_design[arc]) {
                    arc_usage_expr[index] += lambda[sol_index];
                }
            }
        }
        
        for (int i = 0; i < cargoRoute.num_sea_arc() + cargoRoute.num_air_arc(); ++i) {
            model.addConstr(arc_usage_expr[i] == arc_usage[i], "uasge_constr_" + to_string(i));
        }
        
        model.optimize();
        
        model_result.push_back(model.get(GRB_DoubleAttr_ObjVal));
        
        previous_lambda = vector<double>();
        for(int i = 0; i < n; i++) {
            previous_lambda.push_back(lambda[i].get(GRB_DoubleAttr_X));
        }

        #ifdef DEBUG_DW_ITER_LAMBDA
            DW_ITER_LOG("lambda: ");
        
            stringstream lambda_debug_ss;
            for(int i = 0; i < n; i++) {
                lambda_debug_ss << lambda[i].get(GRB_DoubleAttr_X) << " ";
            }
            lambda_debug_ss << endl;
        
            TLMLOG(NULL, lambda_debug_ss.str());
        #endif
        
        #ifdef DEBUG_DW_ITER_SOL_OBJS
            DW_ITER_LOG("P: ");
            
            stringstream pi_debug_ss;
            for (double i: P) {
                pi_debug_ss << i << " " ;
            }
            pi_debug_ss << endl;
        
            TLMLOG(NULL, pi_debug_ss.str());
        #endif

    } catch (GRBException e) {
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
    EntireNetwork *networks = cargoRoute.getNetworks();

    if(sea_arc_pair.size() + air_arc_pair.size() * 2 != pi.size()){
        cout << "update arcs fail !!!";
        exit(1);
    }
    cout << endl;

    SeaNetwork *sea_network = networks->getSea_network();
    
    // Volume upperbound of ship. Use for update fixed cost.
    double sea_volume_ub = sea_network->getDesignedShips()[0].volume_ub;
    
    for(int i = 0; i < sea_arc_pair.size(); i++){
        if(pi[i] != 0){
            int start_idx = sea_arc_pair[i].first;
            int end_idx = sea_arc_pair[i].second;
            Point start = networks->idx_to_point(start_idx);
            Point end = networks->idx_to_point(end_idx);
            for(auto &arc : sea_network->nodes[(char) start.node +65][start.time]->out_arcs){
                if((int) arc->end_node->getName()[0] - 65 == end.node){
                    arc->fixed_cost -= sea_volume_ub*pi[i];  //update cost
                }
            }
            
            const vector<Cargo*> cargos = cargoRoute.getCargos();
            for (int k = 0; k < cargos.size(); ++k) {
                double volume_k = cargos[k]->volume;
                
                if(networks->arcs.find(start_idx) != networks->arcs.end() && networks->arcs[start_idx].find(end_idx) != networks->arcs[start_idx].end()){ //found arc in entirenetwork
                    networks->arcs[start_idx][end_idx]->minus_fixed_profit(k, volume_k*pi[i]); //update profit
                }
            }
        }
    }
    
    AirNetwork *air_network = networks->getAir_network();
    
    // Volume upperbound of flight. Use for update fixed cost.
    double air_volume_ub = air_network->getDesignedFlights()[0].volume_ub;
    // Weight upperbound of flight. Use for update fixed cost.
    double air_weight_ub = air_network->getDesignedFlights()[0].weight_ub;
    
    for(unsigned long long int i = 0; i <  air_arc_pair.size(); i++){
        if(pi[sea_arc_pair.size() + i] != 0){
            int start_idx = air_arc_pair[i].first;
            int end_idx = air_arc_pair[i].second;
            Point start = networks->idx_to_point(air_arc_pair[i].first);
            Point end = networks->idx_to_point(air_arc_pair[i].second);
            for(auto &arc : air_network->nodes[(char) start.node +65][start.time % (7 * TIME_SLOT_A_DAY)]->out_arcs){
                if((int) arc->end_node->getName()[0] - 65 == end.node){
                    arc->fixed_cost -= (air_volume_ub*pi[sea_arc_pair.size() + i] +
                                        air_weight_ub*pi[sea_arc_pair.size() + air_arc_pair.size() + i]);
                }
            }
            
            const vector<Cargo*> cargos = cargoRoute.getCargos();
            for (int k = 0; k < cargos.size(); ++k) {
                double volume_k = cargos[k]->volume;
                
                //found arc in entirenetwork
                if(networks->arcs.find(start_idx) != networks->arcs.end() && networks->arcs[start_idx].find(end_idx) != networks->arcs[start_idx].end()) {
                    
                    //update profit
                    networks->arcs[start_idx][end_idx]->minus_fixed_profit(k, volume_k*pi[sea_arc_pair.size() + i]);
                }
            }
            
        }
    }


    for(unsigned long long int i = 0; i < air_arc_pair.size(); i++){
        if(pi[sea_arc_pair.size() + air_arc_pair.size() + i] != 0){
            int start_idx = air_arc_pair[i].first;
            int end_idx = air_arc_pair[i].second;
            Point start = networks->idx_to_point(air_arc_pair[i].first);
            Point end = networks->idx_to_point(air_arc_pair[i].second);
            for(auto &arc : air_network->nodes[(char) start.node +65][start.time % (7 * TIME_SLOT_A_DAY)]->out_arcs){
                if((int) arc->end_node->getName()[0] - 65 == end.node){
                    arc->fixed_cost -= pi[sea_arc_pair.size() + air_arc_pair.size() + i];
                }
            }
            
            const vector<Cargo*> cargos = cargoRoute.getCargos();
            for (int k = 0; k < cargos.size(); ++k) {
                double weight_k = cargos[k]->weight;
                
                //found arc in entirenetwork
                if(networks->arcs.find(start_idx) != networks->arcs.end() && networks->arcs[start_idx].find(end_idx) != networks->arcs[start_idx].end()){
                    networks->arcs[start_idx][end_idx]->minus_fixed_profit(k, weight_k*pi[sea_arc_pair.size() + air_arc_pair.size() + i]); //update profit
                }
            }
        }
    }

    air_network->run_algo();
    sea_network->run_algo();
//    air_network.generate_designed_flight();
//    this->cargoRoute.getNetworks().setAir_network(air_network);
//    sea_network.generate_designed_ship();
//    this->cargoRoute.getNetworks().setSea_network(sea_network);
    //this->cargoRoute.getNetworks()->generate_new_routes();
    this->cargoRoute.rebuild_entire_network();
}

bool Dantzig_wolfe::end_condition(vector<double> pi) {
    if(pi.size() != R.size()){
        cout << "Fail" <<endl;
        exit(1);
    }
    if(stop_iter > MAX_DW_ITER) return true;
//    return false;
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
    
    fstream v_profile;
    v_profile.open(name+"_v_profile.txt", ios::out);
    
    cargoRoute.out_put_v_value_with_path(v_profile,
                                         best_sol->target_path,
                                         best_sol->rival_path);
    
    v_profile.close();
}

Solution *Dantzig_wolfe::getBestSol() const {
    return best_sol;
}


const CargoRoute Dantzig_wolfe::getCargoRoute() {
    return cargoRoute;
}
