//
// Created by Ashee on 2019/6/27.
//

#include "CargoRoute.h"


//Can't use pair as element in set without this struct
struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator () (std::pair<T1, T2> const &pair) const
    {
        std::size_t h1 = std::hash<T1>()(pair.first);
        std::size_t h2 = std::hash<T2>()(pair.second);

        return h1 ^ h2;
    }
};


CargoRoute::CargoRoute(string data) {
    read_cargo_file(data);
    networks = EntireNetwork(data);
    path_categories = networks.getPaths_categories();
    get_available_path(path_categories, all_paths);
    num_nodes = networks.getNumNodes();
    arcs = networks.getArcs();
    cal_path_cost_profit();

    branch_and_price();
}

void CargoRoute::read_cargo_file(string data) {
    fstream file;
    file.open("../Data/" + data + "_cargo.txt");

    if(file.is_open()){
        string line;
        getline(file, line);
        int num_cargos = stoi(line);

        for(int i = 0; i < num_cargos; i++){
            getline(file, line);
            istringstream iss(line);
            string token;

            getline(iss, token, '\t');
            char departure = (char)token[0];

            getline(iss, token, '\t');
            char destination = (char)token[0];

            getline(iss, token, '\t');
            int starting_time = stoi(token);

            getline(iss, token, '\t');
            int end_time = stoi(token);

            getline(iss, token, '\t');
            int weight = stoi(token);

            getline(iss, token, '\t');
            int volume = stoi(token);

            getline(iss, token, '\t');
            char time_sensitivity = token[0];

            getline(iss, token, '\t');
            char product_value = token[0];

            getline(iss, token, '\t');
            float alpha = stof(token);

            getline(iss, token, '\t');
            float beta = stof(token);

            cargo_type type;
            if(time_sensitivity == 'H' && product_value == 'H') {
                type = only_air;
            }else if(time_sensitivity == 'H' && product_value == 'L')
            {
                type = sea_both;
            }else if(time_sensitivity == 'L' && product_value == 'H'){
                type = air_both;
            }else{
                type = only_sea;
            }

            auto * new_cargo = new Cargo(departure, destination, starting_time, end_time, volume, weight, type);
            new_cargo->setAlpha(alpha);
            new_cargo->setBeta(beta);
            cargos.push_back(new_cargo);
        }
    }
    else{
        cout << "cargo file cannot open !!!";
    }
}

void CargoRoute::get_available_path(vector<Path*>** path_categories, vector<Path*>& paths) {
    unordered_set<pair<char, char>, pair_hash> used_OD;
    for(auto cargo : cargos){
        pair<char, char> OD(cargo->departure, cargo->destination);

        if(used_OD.find(OD) == used_OD.end()){ //OD not in the set
            vector<Path*>  od_available_path = path_categories[(int) cargo->departure - 65][(int) cargo->destination - 65];
//            cout << od_available_path.size() << endl;
            paths.insert(paths.end(), od_available_path.begin(), od_available_path.end()); //append available path to paths
//            cout << paths.size() << " " << cargo->departure << cargo->destination << " " << path_categories[(int) cargo->departure - 65][(int) cargo->destination - 65].size() << endl;
        }
        else{
//            cout << "collision" << endl;
        }
        used_OD.insert(OD);
    }

    for(int i = 0; i < paths.size(); i++){
        paths[i]->setIndex(i);
    }
}

void CargoRoute::cal_path_cost_profit() {
    for(const auto &path : all_paths){
        cal_path_cost(path);
        cal_path_profit(path);
    }
}

void CargoRoute::cal_path_profit(Path* path)/**/{
    double profit = 0;
    Point *cur,*next;
    for(int p = 0; p < path->points.size()-1; p++){
        cur = &path->points[p];
        next = &path->points[p+1];
        profit += arcs[networks.get_node_idx(cur->layer,cur->node, cur->time)]
                      [networks.get_node_idx(next->layer,next->node, next->time)]->unit_profit;
//        if(arcs[networks.get_node_idx(cur_node.layer,cur_node.node, cur_node.time)]
//            [networks.get_node_idx(next_node.layer,next_node.node, next_node.time)]->unit_profit == 0){
//            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//        }
    }
//    cout << *path << profit << endl;
    path->path_profit = profit;

}

void CargoRoute::cal_path_cost(Path *path) {

    Point *current;
    Point *next;
    double cost = 0;
    for(int p = 0; p < path->points.size() -1 ; p++){
        current = &path->points[p];
        next = &path->points[p+1];
        double arc_cost = arcs[networks.get_node_idx(current->layer, current->node, current->time)]
                              [networks.get_node_idx(next->layer, next->node, next->time)]->unit_cost;
        cost += arc_cost;
    }
    path->path_cost = cost;
    path->last_time = path->points.back().time - path->points.front().time;
}

void CargoRoute::branch_and_price() {
    try {
        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);
        vector<GRBVar> *z, *z_, *u;
        target_path = new vector<Path*>[cargos.size()];
        rival_path = new vector<Path*>[cargos.size()];
        z = new vector<GRBVar>[cargos.size()];
        z_ = new vector<GRBVar>[cargos.size()];
        u = new vector<GRBVar>[cargos.size()];

        bp_init(model,z,z_,u);

    } catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }

}

void CargoRoute::bp_init(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *z_, vector<GRBVar> *u) {
    Var_init(model, z, z_, u);
    Obj_init(model, z);
    Constr_init(model, z, z_, u);
    model.optimize();
}

void CargoRoute::Var_init(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *z_, vector<GRBVar> *u) {

    for (int c = 0 ; c < cargos.size(); c++) {
        int departure = cargos[c]->departure - 65;
        int destination = cargos[c]->destination - 65 ;

//        Path *best_path = nullptr;
        int path_count = 0 ;
        for (const auto &path : path_categories[departure][destination]) {
//            cout << path->path_profit << " " << *path ;
            if(cargos[c]->start_time < path->get_start_time() && cargos[c]->arrive_time > path->get_end_time() && path_count < 5) {
                if(path->path_profit != 0) {
                    z[c].push_back(model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS));
                    u[c].push_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY));
                    target_path[c].emplace_back(path);
                }
                else{
                    rival_path[c].emplace_back(path);
                    z_[c].push_back(model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS));
                }
                path_count += 1;
            }
        }
//        if(best_path) {
//            if (best_path->path_profit != 0) {
//                z[c].push_back(model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS));
//                u[c].push_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY));
//                target_path[c].emplace_back(best_path);
//            } else {
//                rival_path[c].emplace_back(best_path);
//                z_[c].push_back(model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS));
//            }
//        }
    }
}

void CargoRoute::Obj_init(GRBModel &model, vector<GRBVar> *z) {
    GRBLinExpr obj = 0;
    for(int k = 0; k < cargos.size(); k++) {
        for (int p = 0; p < target_path[k].size(); p++) {
            obj += z[k][p] * cargos[k]->volume * target_path[k][p]->net_profit();
        }
    }
    model.setObjective(obj, GRB_MAXIMIZE);
}

void CargoRoute::Constr_init(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *z_, vector<GRBVar> *u) {
    set_constr1(model, z, z_);
    set_constr2(model, z, u);
    cal_e();
    set_constr3(model, z, z_, u);
    set_constr4(model, z, z_, u);
    set_constr5(model, z);
    set_constr6(model, z);
    set_constr7(model, z);

}

void CargoRoute::set_constr1(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *z_){
    cons1 = new GRBConstr[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        GRBLinExpr expr = 0;
        for(int p = 0; p < target_path[k].size(); p++){
            expr += z[k][p];
        }
        for(int n = 0; n < rival_path[k].size(); n++){
            expr += z_[k][n];
        }
        cons1[k] = model.addConstr(expr <= 1, "Cons1" + to_string(k));
    }
}

void CargoRoute::set_constr2(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *u) {
    cons2 = new vector<GRBConstr>[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_path[k].size(); p++){
            cons2[k].push_back(model.addConstr(z[k][p] <= u[k][p], "Cons2" + to_string(k) + to_string(p)));
        }
    }
}
void CargoRoute::cal_e() {
    cal_v();
    e = new double*[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        e[k] = new double[target_path[k].size()];
    }
    e_ = new double*[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        e_[k] = new double[rival_path[k].size()];
    }

    for(int k = 0; k < cargos.size(); k++){
        double sum = 0;
        for(int p = 0; p < target_path[k].size(); p++) {
            sum += exp(v[k][p]);
        }
        for(int n = 0; n < rival_path[k].size(); n++) {
            sum += exp(v_[k][n]);
        }
        for(int p = 0; p < target_path[k].size(); p++){
            e[k][p] = exp(v[k][p]) / sum ;
//            cout << e[k][p] << " ";
        }
        for(int n = 0; n < rival_path[k].size(); n++){
            e_[k][n] = exp(v_[k][n]) / sum ;
        }
//        cout << endl;
    }

}

void CargoRoute::cal_v() {
    //init v
    v = new double*[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        v[k] = new double[target_path[k].size()];
    }
    v_ = new double*[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        v_[k] = new double[rival_path[k].size()];
    }

    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_path[k].size(); p++){
            v[k][p] = cargos[k]->alpha * target_path[k][p]->path_cost + cargos[k]->beta * target_path[k][p]->last_time;
        }
        for(int n = 0; n < rival_path[k].size(); n++){
            v_[k][n] = cargos[k]->alpha * rival_path[k][n]->path_cost + cargos[k]->beta * rival_path[k][n]->last_time;
        }
    }
}



void CargoRoute::set_constr3(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *z_, vector<GRBVar> *u) {
    GRBLinExpr lhs, rhs;
    cons3 = new vector<GRBConstr>[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_path[k].size();p++){
            for(int n = 0; n < rival_path[k].size(); n++){
                lhs += e_[k][n] ;
                rhs += e[k][p] * z_[k][n];
            }
//            cout << e[k][p] << endl;
            cons3[k].push_back(model.addConstr(lhs * z[k][p] <=  rhs, "cons3" + to_string(k) + to_string(p)));
        }
    }
}

void CargoRoute::set_constr4(GRBModel &model, vector<GRBVar> *z, vector<GRBVar> *z_, vector<GRBVar> *u) {
    GRBLinExpr lhs, rhs;
    cons4 = new vector<GRBConstr>[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_path[k].size();p++){
            for(int n = 0; n < rival_path[k].size(); n++){
                lhs += e_[k][n] ;
                rhs += e[k][p] * z_[k][n];
            }
            cons4[k].push_back(model.addConstr(lhs * z[k][p] >=  rhs + u[k][p] - 1, "cons4" + to_string(k) + to_string(p)));
        }
    }
}

void CargoRoute::set_constr5(GRBModel &model, vector<GRBVar> *z) {
    vector<Ship> ships = networks.get_cur_ships();
    for(const auto &ship : ships){
        vector<string> nodes = ship.route.nodes;
        for(int i = 0; i < nodes.size() -1; i++){
            Point cur_point = Point(3, (int) nodes[i][0] - 65, stoi(nodes[i].substr(1)));
            Point next_point = Point(3, (int) nodes[i+1][0] - 65, stoi(nodes[i+1].substr(1)));
            GRBLinExpr lhs = 0;
            for(int k = 0; k < cargos.size(); k++){
                for(int p = 0; p < target_path[k].size(); p++){
                    vector<Point> points = target_path[k][p]->points;
                    for(int n = 0; n < points.size()-1; n++){
                        if(points[n] == cur_point && points[n+1] == next_point){
//                            cout << k << " " << p << " "<< points[n] << cur_point << " "<<  points[n+1] <<  next_point << endl;
                            lhs += cargos[k]->volume * z[k][p];
                        }
                    }
                }
            }
            cons5[networks.get_node_idx(cur_point)][networks.get_node_idx(next_point)] = model.addConstr(lhs <= ship.volume_ub);
        }
//        cout << endl;
    }
}

void CargoRoute::set_constr6(GRBModel &model, vector<GRBVar> *z) {
    vector<Flight> flights = networks.get_cur_flights();
    for(const auto &flight : flights){
        for(int week = 0 ; week < TIME_PERIOD / 7; week++) {
            for(const auto &route : flight.routes){
                vector<string> nodes = route.nodes;
                for(int i = 0; i < nodes.size() -1; i++){
                    Point cur_point = Point(4, (int) nodes[i][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(nodes[i].substr(1)));
                    Point next_point = Point(4, (int) nodes[i+1][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(nodes[i+1].substr(1)));
                    GRBLinExpr lhs = 0;
                    for(int k = 0; k < cargos.size(); k++){
                        for(int p = 0; p < target_path[k].size(); p++){
                            vector<Point> points = target_path[k][p]->points;
                            for(int n = 0; n < points.size()-1; n++){
                                if(points[n] == cur_point && points[n+1] == next_point){
//                                    cout << k << " " << p << " "<< points[n] << cur_point << " "<<  points[n+1] <<  next_point << endl;
                                    lhs += cargos[k]->volume * z[k][p];
                                }
                            }
                        }
                    }
                    cons6[networks.get_node_idx(cur_point)][networks.get_node_idx(next_point)] = model.addConstr(lhs <= flight.volume_ub);
                }
            }
        }
    }
}

void CargoRoute::set_constr7(GRBModel &model, vector<GRBVar> *z) {
    vector<Flight> flights = networks.get_cur_flights();
    for(const auto &flight : flights){
        for(int week = 0 ; week < TIME_PERIOD / 7; week++) {
            for(const auto &route : flight.routes){
                vector<string> nodes = route.nodes;
                for(int i = 0; i < nodes.size() -1; i++){
                    Point cur_point = Point(4, (int) nodes[i][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(nodes[i].substr(1)));
                    Point next_point = Point(4, (int) nodes[i+1][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(nodes[i+1].substr(1)));
                    GRBLinExpr lhs = 0;
                    for(int k = 0; k < cargos.size(); k++){
                        for(int p = 0; p < target_path[k].size(); p++){
                            vector<Point> points = target_path[k][p]->points;
                            for(int n = 0; n < points.size()-1; n++){
                                if(points[n] == cur_point && points[n+1] == next_point){
//                                    cout << k << " " << p << " "<< points[n] << cur_point << " "<<  points[n+1] <<  next_point << endl;
                                    lhs += cargos[k]->weight * z[k][p];
                                }
                            }
                        }
                    }
                    cons7[networks.get_node_idx(cur_point)][networks.get_node_idx(next_point)] = model.addConstr(lhs <= flight.weight_ub);
                }
            }
        }
    }
}