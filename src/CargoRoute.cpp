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


CargoRoute::CargoRoute(string data)  {
    read_cargo_file(data);
    networks = EntireNetwork(data);
    num_nodes = networks.getNumNodes();
    arcs = networks.getArcs();
    find_sea_arcs();
    find_air_arcs();
    arcs_to_file();
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

void CargoRoute::cal_paths_profit() {
    for(const auto &path : all_paths) {
        cal_path_profit(path);
//        cout << path->path_profit <<  " " << *path;
    }
}

void CargoRoute::cal_path_profit(Path* path)/**/{
    double profit = 0;
    double pi = 0;
    bool only_rival = true;
    Point *cur,*next;
    for(int p = 0; p < path->points.size()-1; p++){
        cur = &path->points[p];
        next = &path->points[p+1];
        profit += arcs[networks.get_node_idx(*cur)]
                      [networks.get_node_idx(*next)]->unit_profit;
        pi +=  arcs[networks.get_node_idx(*cur)][networks.get_node_idx(*next)]->fixed_profit;
        if(next->layer == 0 || next->layer == 1 || next->layer == 3 || next->layer == 4)
            only_rival = false;
//        if(arcs[networks.get_node_idx(cur_node.layer,cur_node.node, cur_node.time)]
//            [networks.get_node_idx(next_node.layer,next_node.node, next_node.time)]->unit_profit == 0){
//            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//        }
    }
//    cout << *path << profit << endl;
    path->path_profit = profit;
    path->pi = pi;
    path->only_rival = only_rival;
}

void CargoRoute::cal_paths_cost() {
    for(const auto &path : all_paths){
        cal_path_cost(path);
    }
}

void CargoRoute::cal_path_cost(Path *path) {

    Point *current;
    Point *next;
    double cost = 0;
    for(int p = 0; p < path->points.size() -1 ; p++){
        current = &path->points[p];
        next = &path->points[p+1];
        double arc_cost = arcs[networks.get_node_idx(current->layer, current->node, current->time)]
                              [networks.get_node_idx(next->layer, next->node, next->time)]->getUnitCost();

        cost += arc_cost;
    }
    path->path_cost = cost;
    path->last_time = path->points.back().time - path->points.front().time;
}

Solution* CargoRoute::branch_and_price() {
    try{
        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);
        model.set(GRB_IntParam_OutputFlag, false);
        //initialize
        z = new vector<GRBVar>[cargos.size()];
        z_ = new vector<GRBVar>[cargos.size()];
        u = new vector<GRBVar>[cargos.size()];
        target_path = new vector<Path *>[cargos.size()];
        rival_path = new vector<Path *>[cargos.size()];
        chosen_paths = new unordered_set<int>[cargos.size()];


        bp_init(model);
        priority_queue<BB_node> bb_pool;
        BB_node::cargo_size = cargos.size();
        bb_pool.push(BB_node(model.get(GRB_DoubleAttr_ObjVal), target_path, rival_path, chosen_paths, integer_set));
        int iter = 0;
        while (!bb_pool.empty()) {
            if (bb_pool.top().getObj() < incumbent) {
                bb_pool.pop();
                continue;
            }

            target_path = bb_pool.top().getTargetPath();
            rival_path = bb_pool.top().getRivalPath();
            chosen_paths = bb_pool.top().getChosenPaths();
            integer_set = bb_pool.top().getIntegerSet();

            bb_pool.pop();
            column_generation(model);
            show_model_result(model);
            if (is_integral()) break;

            pair<int, int> kp_pair = find_kp_pair();

            //branching
            integer_set[kp_pair.first][kp_pair.second] = false;
            LP_relaxation(model);
            bb_pool.push(BB_node(model.get(GRB_DoubleAttr_ObjVal), target_path, rival_path, chosen_paths, integer_set));
            if (is_integral() && incumbent < model.get(GRB_DoubleAttr_ObjVal))
                incumbent = model.get(GRB_DoubleAttr_ObjVal);

            integer_set[kp_pair.first][kp_pair.second] = true;
            LP_relaxation(model);
            bb_pool.push(BB_node(model.get(GRB_DoubleAttr_ObjVal), target_path, rival_path, chosen_paths, integer_set));
            if (is_integral() && incumbent < model.get(GRB_DoubleAttr_ObjVal))
                incumbent = model.get(GRB_DoubleAttr_ObjVal);

            iter++;
            cout << "BP_iter : " << iter << endl;
            if (iter > MAX_BP_ITER) {
                set_all_u_integer(model, u);
                LP_relaxation(model);
                break;
            }
        }

        objVal = model.get(GRB_DoubleAttr_ObjVal);
//        for(auto  &k : integer_set){
//            for(auto &p : integer_set[k.first]){
//                cout << k.first << " " << p.first << " " << p.second <<endl;
//            }
//        }
        z_value = new vector<double>[cargos.size()];
        for (int k = 0; k < cargos.size(); k++) {
            for (int p = 0; p < target_path[k].size(); p++) {
                z_value[k].push_back(z[k][p].get(GRB_DoubleAttr_X));
            }
        }

        Solution *sol = new Solution(cargos.size(), target_path, z_value, get_P_value(), get_r_column(), networks.getSea_Air_Route());
        return sol;
    } catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }
    return nullptr;
}

void CargoRoute::bp_init(GRBModel &model) {
    path_categories = networks.getPaths_categories();
    get_available_path(path_categories, all_paths);
    arcs = networks.getArcs();
    cal_paths_profit();
    cal_paths_cost();

    select_init_path();
    Var_init(model);
    Obj_init(model);
    Constr_init(model);
    model.optimize();
//    show_model_result(model, z, z_, u);
}

void CargoRoute::select_init_path() {
    for (int k = 0 ; k < cargos.size(); k++) {
        int departure = cargos[k]->departure - 65;
        int destination = cargos[k]->destination - 65 ;

//        Path *best_path = nullptr;
        int path_count = 0 ;

        for (const auto &path : path_categories[departure][destination]) {
//            cout << path->path_profit << " " << *path ;
            if(cargos[k]->start_time <= path->get_start_time()
            && cargos[k]->arrive_time >= path->get_end_time()
            && path_count < NUM_INIT_PATHS
            ){
//                if(!path->only_rival) {
//                    target_path[k].emplace_back(path);
//                    path_count += 1;
//                    chosen_paths[k].insert(path->index);
//                }
                if(path->only_rival){
                    rival_path[k].emplace_back(path);
                    path_count += 1;
                    chosen_paths[k].insert(path->index);
                }
            }
        }
    }
}

void CargoRoute::LP_relaxation(GRBModel &model) {
    model.reset();
    for(int k = 0; k<cargos.size();k++) {
        z[k].clear();
        z_[k].clear();
        u[k].clear();
    }
    Var_init(model);
    Obj_init(model);
    Constr_init(model);

    model.optimize();
}

void CargoRoute::column_generation(GRBModel &model) {
    Path *best_path;
    int iter = 0;
    while (true) {
        LP_relaxation(model);

        update_arcs();
        pair<Path*, int> path_pair = select_most_profit_path();
        best_path = path_pair.first;
        if (path_pair.first->reduced_cost <= 0) {
            break;
        }else{
            append_column(path_pair.first, path_pair.second);
        }

        cout << best_path->reduced_cost << " " << *best_path;
        cout << "Iter : " << iter << " Obj : " << model.get(GRB_DoubleAttr_ObjVal) << endl;
        iter++;
    }
}

void CargoRoute::Var_init(GRBModel &model) {
    for(int k = 0; k< cargos.size(); k++){
        for(int p = 0; p < target_path[k].size(); p++){
            z[k].push_back(model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS));
            u[k].push_back(model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS));
        }
        for(int n = 0; n < rival_path[k].size(); n++){
            z_[k].push_back(model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS));
        }
    }
}

void CargoRoute::Obj_init(GRBModel &model) {
    GRBLinExpr obj = 0;
    for(int k = 0; k < cargos.size(); k++) {
        for (int p = 0; p < target_path[k].size(); p++) {
            obj += z[k][p] * cargos[k]->volume * target_path[k][p]->path_profit;
        }
    }
    model.setObjective(obj, GRB_MAXIMIZE);
}

void CargoRoute::Constr_init(GRBModel &model) {
    set_constr1(model);
    set_constr2(model);
    cal_e();
    set_constr3(model);
    set_constr4(model);
    set_constr5(model);
    set_constr6(model);
    set_constr7(model);
    set_integer(model);
}

void CargoRoute::set_constrs(GRBModel &model) {
    set_constr1(model);
    set_constr2(model);
    cal_e();
    set_constr3(model);
    set_constr4(model);
    set_constr5(model);
    set_constr6(model);
    set_constr7(model);
    set_complicate_constr1(model);
    set_complicate_constr2(model);
    set_complicate_constr3(model);
}

void CargoRoute::set_constr1(GRBModel &model){
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

void CargoRoute::set_constr2(GRBModel &model) {
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

void CargoRoute::set_constr3(GRBModel &model) {
    GRBLinExpr lhs, rhs;
    cons3 = new vector<GRBConstr>[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_path[k].size();p++){
            for(int n = 0; n < rival_path[k].size(); n++){
                lhs += e_[k][n];
                rhs += z_[k][n];
            }
//            cout << e[k][p] << endl;
            cons3[k].push_back(model.addConstr(lhs* z[k][p] - e[k][p] * rhs <= 0, "cons3" + to_string(k) + to_string(p)));
        }
    }
}

void CargoRoute::set_constr4(GRBModel &model) {
    GRBLinExpr lhs, rhs;
    cons4 = new vector<GRBConstr>[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_path[k].size();p++){
            for(int n = 0; n < rival_path[k].size(); n++){
                lhs += e_[k][n]  ;
                rhs += e[k][p] ;
            }
            cons4[k].push_back(model.addConstr(lhs * z[k][p] - rhs * z[k][p] - u[k][p]>= - 1, "cons4" + to_string(k) + to_string(p)));
        }
    }
}

void CargoRoute::set_constr5(GRBModel &model) {
    vector<Ship> ships = networks.get_cur_ships();
    for(int w = 0; w < TOTAL_WEEK; w++) {
        for (const auto &ship : ships) {
            vector<string> nodes = ship.route.nodes;
            for (int i = 0; i < nodes.size() - 1; i++) {
                Point cur_point = Point(3, (int) nodes[i][0] - 65, stoi(nodes[i].substr(1)) + w * 7 * TIME_SLOT_A_DAY);
                Point next_point = Point(3, (int) nodes[i + 1][0] - 65, stoi(nodes[i + 1].substr(1)) + w * 7 * TIME_SLOT_A_DAY);
                if(next_point.time >= TOTAL_TIME_SLOT) break;
                GRBLinExpr lhs = 0;
                for (int k = 0; k < cargos.size(); k++) {
                    for (int p = 0; p < target_path[k].size(); p++) {
                        vector<Point> points = target_path[k][p]->points;
                        for (int n = 0; n < points.size() - 1; n++) {
                            if (points[n] == cur_point && points[n + 1] == next_point) {
//                            cout << k << " " << p << " "<< points[n] << cur_point << " "<<  points[n+1] <<  next_point << endl;
                                lhs += cargos[k]->volume * z[k][p];
                            }
                        }
                    }
                }
                cons5[networks.get_node_idx(cur_point)][networks.get_node_idx(next_point)] = model.addConstr(
                        lhs <= ship.volume_ub);
            }
//        cout << endl;
        }
    }
}

void CargoRoute::set_constr6(GRBModel &model) {
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

void CargoRoute::set_constr7(GRBModel &model) {
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

void CargoRoute::set_complicate_constr1(GRBModel &model) {
    GRBLinExpr lhs;
    double rhs;
    for (auto &sea_arc_pair : sea_arc_pairs) {
        lhs = complicate_constr_lhs(sea_arc_pair.first, sea_arc_pair.second);
        rhs = complicate_sea_rhs(sea_arc_pair.first, sea_arc_pair.second, networks.getSea_network().getShips()[0].volume_ub);

        model.addConstr(lhs <= rhs);
    }
}

void CargoRoute::set_complicate_constr2(GRBModel &model) {
    GRBLinExpr lhs;
    double rhs;

    for (auto &air_arc_pair : air_arc_pairs) {
        lhs= complicate_constr_lhs(air_arc_pair.first, air_arc_pair.second);
        rhs= complicate_air_rhs(air_arc_pair.first, air_arc_pair.second, networks.getAir_network().getFlights()[0].volume_ub);

        model.addConstr(lhs <= rhs);
    }
}

void CargoRoute::set_complicate_constr3(GRBModel &model) {
    GRBLinExpr lhs;
    double rhs;

    for (auto &air_arc_pair : air_arc_pairs) {
        lhs= complicate_constr_lhs(air_arc_pair.first, air_arc_pair.second);
        rhs = complicate_air_rhs(air_arc_pair.first, air_arc_pair.second, networks.getAir_network().getFlights()[0].weight_ub);

        model.addConstr(lhs <= rhs);
    }
}

void CargoRoute::set_integer(GRBModel &model) {
    for(auto  &k : integer_set){
        for(auto &p : integer_set[k.first]){
            model.addConstr(u[k.first][p.first] == p.second);
        }
    }
}

void CargoRoute::set_all_u_integer(GRBModel &model, vector<GRBVar> *u) {
    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_path[k].size(); p++){
            if(u[k][p].get(GRB_DoubleAttr_X) > MU_THRESHOLD){
                model.addConstr(u[k][p] == 1);
            }else{
                model.addConstr(u[k][p] == 0);
            }
        }
    }
}

void CargoRoute::update_arcs() {
    vector<Ship> ships = networks.get_cur_ships();
    for(int w = 0; w < TOTAL_WEEK; w++) {
        for (const auto &ship : ships) {
            vector<string> nodes = ship.route.nodes;
            for (int i = 0; i < nodes.size() - 1; i++) {
                Point cur_point = Point(3, (int) nodes[i][0] - 65, stoi(nodes[i].substr(1)) + w * 7 * TIME_SLOT_A_DAY);
                Point next_point = Point(3, (int) nodes[i + 1][0] - 65, stoi(nodes[i + 1].substr(1)) + w * 7 * TIME_SLOT_A_DAY);
                if(next_point.time >= TOTAL_TIME_SLOT) break;
                double pi5 = cons5[networks.get_node_idx(cur_point)][networks.get_node_idx(next_point)].get(
                        GRB_DoubleAttr_Pi);
                arcs[networks.get_node_idx(cur_point)][networks.get_node_idx(next_point)]->minus_fixed_profit(pi5);
//            cout << 5 << networks.get_node_idx(cur_point) << " " << networks.get_node_idx(next_point) << endl;
            }
        }
    }

    vector<Flight> flights = networks.get_cur_flights();
    for (const auto &flight : flights) {
        for (int week = 0; week < TIME_PERIOD / 7; week++) {
            for (const auto &route : flight.routes) {
                vector<string> nodes = route.nodes;
                for (int i = 0; i < nodes.size() - 1; i++) {
                    Point cur_point = Point(4, (int) nodes[i][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(nodes[i].substr(1)));
                    Point next_point = Point(4, (int) nodes[i+1][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(nodes[i+1].substr(1)));
                    double pi6 = cons6[networks.get_node_idx(cur_point)][networks.get_node_idx(next_point)].get(GRB_DoubleAttr_Pi);
                    double pi7 = cons7[networks.get_node_idx(cur_point)][networks.get_node_idx(next_point)].get(GRB_DoubleAttr_Pi);
                    arcs[networks.get_node_idx(cur_point)][networks.get_node_idx(next_point)]->minus_fixed_profit(pi6);
                    arcs[networks.get_node_idx(cur_point)][networks.get_node_idx(next_point)]->minus_fixed_profit( pi7);
//                    cout << 6 << networks.get_node_idx(cur_point) << " " << networks.get_node_idx(next_point) << endl;
                }
            }
        }
    }

}

pair<Path*, int> CargoRoute::select_most_profit_path() {
    Path* best_path = nullptr;
    int best_k = -1;
    for (int k = 0 ; k < cargos.size(); k++) {
        int departure = cargos[k]->departure - 65;
        int destination = cargos[k]->destination - 65 ;
//        cout << departure << " " << destination << " " << path_categories[departure][destination].size() << endl;
        for (const auto &path : path_categories[departure][destination]) {
            cal_path_reduced_cost(path, k);
            if(cargos[k]->start_time <= path->get_start_time() &&
            cargos[k]->arrive_time >= path->get_end_time() &&
            chosen_paths[k].find(path->index) == chosen_paths[k].end()) {
                if (!best_path || (best_path->reduced_cost < path->reduced_cost)) {
                    best_path = path;
                    best_k = k;
                }
            }
        }
    }
    return make_pair(best_path, best_k);
}

void CargoRoute::append_column(Path *best_path, int best_k) {
    if(best_path) {
        if (!best_path->only_rival) {
            target_path[best_k].push_back(best_path);
        } else {
            rival_path[best_k].push_back(best_path);
        }
        chosen_paths[best_k].insert(best_path->index);
    }
}

void CargoRoute::cal_path_reduced_cost(Path* path, int k){
    double reduced_cost = 0;
    Point *cur,*next;
    for(int p = 0; p < path->points.size()-1; p++){
        cur = &path->points[p];
        next = &path->points[p+1];
        reduced_cost += arcs[networks.get_node_idx(*cur)][networks.get_node_idx(*next)]->unit_profit +
                        arcs[networks.get_node_idx(*cur)][networks.get_node_idx(*next)]->fixed_profit;
    }

    reduced_cost -= cons1[k].get(GRB_DoubleAttr_Pi);

    for(int p = 0; p < target_path[k].size(); p++){
        reduced_cost -= cons2[k][p].get(GRB_DoubleAttr_Pi);
        for(int n = 0; n < rival_path[k].size(); n++){
            reduced_cost -= (e_[k][n] * cons3[k][p].get(GRB_DoubleAttr_Pi));
            reduced_cost += (e_[k][n] * cons4[k][p].get(GRB_DoubleAttr_Pi));
        }
    }
    reduced_cost = MAX(reduced_cost, 0);
    path->reduced_cost = reduced_cost;
}

bool CargoRoute::is_integral() {
    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_path[k].size(); p++){
            if(u[k][p].get(GRB_DoubleAttr_X) != 1.0 && u[k][p].get(GRB_DoubleAttr_X) != 0.0){
                return false;
            }
        }
    }
    return true;
}

pair<int,int> CargoRoute::find_kp_pair(){
    pair<int, int> kp_pair;
    double closest_diff = 1;
    for(int k = 0; k < cargos.size(); k++){
//        cout << target_path[k].size() << " " << u[k].size() << endl;
        for(int p = 0; p < target_path[k].size(); p++){
//            cout << k << " " << p << " " << u[k][p].get(GRB_DoubleAttr_X) << endl;
            if(abs(u[k][p].get(GRB_DoubleAttr_X) - 0.5) < closest_diff){
                kp_pair = pair(k,p);
                closest_diff = abs(u[k][p].get(GRB_DoubleAttr_X) - 0.5);
            }
        }
    }
    return kp_pair;
}

void CargoRoute::find_sea_arcs() {
    SeaNetwork seaNetwork = networks.getSea_network();
    for(int i = 65 ; i < 65+num_nodes ;i++){
        for(int t = 0; t < TOTAL_TIME_SLOT; t++){
            for(const auto& arc : seaNetwork.nodes[(char) i][t]->out_arcs){
                string out_node = arc->start_node->getName();
                string in_node = arc->end_node->getName();
                Point out_point = Point(0, (int) out_node[0] -65, stoi(out_node.substr(1)));
                Point in_point = Point(0, (int) in_node[0] -65, stoi(in_node.substr(1)));

                sea_arc_pairs.emplace_back(networks.get_node_idx(out_point), networks.get_node_idx(in_point));
//                cout << out_point << " " << in_point << " " << networks.get_node_idx(out_point) << " " << networks.get_node_idx(in_point) << networks.idx_to_point(networks.get_node_idx(out_point)) << " "<< networks.idx_to_point(networks.get_node_idx(in_point)) << " " << endl;
            }
        }
    }
}

void CargoRoute::find_air_arcs() {
    AirNetwork airNetwork = networks.getAir_network();
    for(int i = 65 ; i < 65+num_nodes ;i++){
        for(int t = 0; t < TOTAL_TIME_SLOT; t++){
            for(const auto& arc : airNetwork.nodes[(char) i][t]->out_arcs){
                string out_node = arc->start_node->getName();
                string in_node = arc->end_node->getName();
                Point out_point = Point(1, (int) out_node[0] -65, stoi(out_node.substr(1)));
                Point in_point = Point(1, (int) in_node[0] -65, stoi(in_node.substr(1)));

                air_arc_pairs.emplace_back(networks.get_node_idx(out_point), networks.get_node_idx(in_point));
//                cout << out_point << " " << in_point << " " << networks.get_node_idx(out_point) << " " << networks.get_node_idx(in_point) << " " << networks.idx_to_point(networks.get_node_idx(out_point)) << " "<< networks.idx_to_point(networks.get_node_idx(in_point)) << " " << endl;
            }
        }
    }
}

void CargoRoute::arcs_to_file() {
	ofstream sea_file;
	sea_file.open("A1_sea_arcs.csv");
	for(const auto arc : sea_arc_pairs){

		Point first = networks.idx_to_point(arc.first);
		Point second = networks.idx_to_point(arc.second);
		sea_file << to_string(first.layer) + (char) (first.node +65) + to_string(first.time) << ",";
		sea_file << to_string(second.layer) + (char) (second.node +65) + to_string(second.time);
		cout << to_string(first.layer) + (char) (first.node +65) + to_string(first.time) << " " << to_string(second.layer) + (char) (second.node +65) + to_string(second.time) << endl;

		sea_file << "\n";
	}
	sea_file.close();

	ofstream air_file;
	air_file.open("A1_air_arcs.csv");
	for(const auto arc : air_arc_pairs){

		Point first = networks.idx_to_point(arc.first);
		Point second = networks.idx_to_point(arc.second);
		air_file << to_string(first.layer) + (char) (first.node +65) + to_string(first.time) << ",";
		air_file << to_string(second.layer) + (char) (second.node +65) + to_string(second.time) ;
		cout << to_string(first.layer) + (char) (first.node +65) + to_string(first.time) << " " << to_string(second.layer) + (char) (second.node +65) + to_string(second.time) << endl;

		air_file << "\n";
	}
	air_file.close();
}

double* CargoRoute::cal_constr1_val() {
    double* val = new double[sea_arc_pairs.size()];
    for(int i = 0; i < sea_arc_pairs.size(); i++){
        val[i] = get_sea_complicate_constr_val(sea_arc_pairs[i].first, sea_arc_pairs[i].second, networks.getSea_network().getShips()[0].volume_ub);
    }

    return val;
}

double* CargoRoute::cal_constr2_val() {
    double* val = new double[air_arc_pairs.size()];
    for(int i = 0; i < air_arc_pairs.size(); i++){
        val[i] = get_air_complicate_constr_val(air_arc_pairs[i].first, air_arc_pairs[i].second, networks.getAir_network().getFlights()[0].volume_ub);
    }
    return val;
}

double* CargoRoute::cal_constr3_val() {
    double* val = new double[air_arc_pairs.size()];
    for(int i = 0; i < air_arc_pairs.size(); i++){
        val[i] = get_air_complicate_constr_val(air_arc_pairs[i].first, air_arc_pairs[i].second, networks.getAir_network().getFlights()[0].weight_ub);
    }
    return val;
}

double CargoRoute::get_sea_complicate_constr_val(int start_idx, int end_idx, int ub) {
    double val = 0;
    // left hand side

    for(int k = 0; k < cargos.size(); k++){
        for (int p = 0; p < target_path[k].size(); p++) {
            Path* path = target_path[k][p];
            for(int i = 0; i < path->points.size()-1; i++){
                int out_point_idx = networks.get_node_idx(path->points[i]);
                int in_point_idx = networks.get_node_idx(path->points[i+1]);
                if(out_point_idx == start_idx && in_point_idx == end_idx){
                    val += cargos[k]->volume * z_value[k][p];
                }
            }
        }
    }
    //right hand side
    Route route = networks.getSea_network().getShips()[0].route;
    for(int i = 0; i < route.nodes.size()-1; i++){
        Point out_point = Point(0, (int) route.nodes[i][0] - 65 , stoi(route.nodes[i].substr(1)));
        Point in_point = Point(0, (int) route.nodes[i+1][0] - 65 , stoi(route.nodes[i+1].substr(1)));
        int out_point_idx = networks.get_node_idx(out_point);
        int in_point_idx = networks.get_node_idx(in_point);
        if(out_point_idx == start_idx && in_point_idx == end_idx){
            val -= ub;
        }
    }

    return val;
}

double CargoRoute::get_air_complicate_constr_val(int start_idx, int end_idx, int ub) {
    double val = 0;
    // left hand side

    for(int k = 0; k < cargos.size(); k++){
        for (int p = 0; p < target_path[k].size(); p++) {
            Path* path = target_path[k][p];
            for(int i = 0; i < path->points.size()-1; i++){
                int out_point_idx = networks.get_node_idx(path->points[i]);
                int in_point_idx = networks.get_node_idx(path->points[i+1]);
                if(out_point_idx == start_idx && in_point_idx == end_idx){
                    val += cargos[k]->volume * z_value[k][p];
                }
            }
        }
    }
    //right hand side
    for(int week = 0; week < TIME_PERIOD / 7; week++) {
        for(const auto &route : networks.getAir_network().getFlights()[0].routes) {
            for (int i = 0; i < route.nodes.size() - 1; i++) {
                Point out_point = Point(1, (int) route.nodes[i][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i].substr(1)));
                Point in_point = Point(1, (int) route.nodes[i+1][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i + 1].substr(1)));
                int out_point_idx = networks.get_node_idx(out_point);
                int in_point_idx = networks.get_node_idx(in_point);
                if (out_point_idx == start_idx && in_point_idx == end_idx) {
                    val -= ub;
                }
            }
        }
    }

    return val;
}

GRBLinExpr CargoRoute::complicate_constr_lhs(int start_idx, int end_idx){
    GRBLinExpr cons = 0;
    for(int k = 0; k < cargos.size(); k++){
        for (int p = 0; p < target_path[k].size(); p++) {
            Path* path = target_path[k][p];
            for(int i = 0; i < path->points.size()-1; i++){
                int out_point_idx = networks.get_node_idx(path->points[i]);
                int in_point_idx = networks.get_node_idx(path->points[i+1]);
                if(out_point_idx == start_idx && in_point_idx == end_idx){
                    cons += cargos[k]->volume * z[k][p];
                }
            }
        }
    }
    return cons;
}

double CargoRoute::complicate_sea_rhs(int start_idx, int end_idx, int ub) {
    double val = 0;
    Route route = networks.getSea_network().getShips()[0].route;
    for(int w = 0; w < TOTAL_WEEK ; w++) {
        for (int i = 0; i < route.nodes.size() - 1; i++) {
            Point out_point = Point(0, (int) route.nodes[i][0] - 65, stoi(route.nodes[i].substr(1)) + w * 7 * TIME_SLOT_A_DAY);
            Point in_point = Point(0, (int) route.nodes[i + 1][0] - 65, stoi(route.nodes[i + 1].substr(1))+ w * 7 * TIME_SLOT_A_DAY);
            int out_point_idx = networks.get_node_idx(out_point);
            int in_point_idx = networks.get_node_idx(in_point);
            if (out_point_idx == start_idx && in_point_idx == end_idx) {
                val += ub;
                return val;
            }
        }
    }

    return val;
}

double CargoRoute::complicate_air_rhs(int start_idx, int end_idx, int ub) {
    double val = 0;
    for(int week = 0; week < TIME_PERIOD / 7; week++) {
        for(const auto &route : networks.getAir_network().getFlights()[0].routes) {
            for (int i = 0; i < route.nodes.size() - 1; i++) {
                Point out_point = Point(1, (int) route.nodes[i][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i].substr(1)));
                Point in_point = Point(1, (int) route.nodes[i+1][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i + 1].substr(1)));
                int out_point_idx = networks.get_node_idx(out_point);
                int in_point_idx = networks.get_node_idx(in_point);
                if (out_point_idx == start_idx && in_point_idx == end_idx) {
                    val += ub;
                    return val;
                }
            }
        }
    }
    return val;
}

void CargoRoute::show_model_result(GRBModel &model) {
    cout << "Obj : " << model.get(GRB_DoubleAttr_ObjVal) << "\tIncumbent : " << incumbent << endl;
    cout << "==============LP relaxation===============" << endl;
}

unordered_set<pair<int, int>, pair_hash> CargoRoute::get_arc_set(Path *path) {
    unordered_set<pair<int, int>, pair_hash> arc_set{};
    for(int i = 0; i < path->points.size()-1; i++){
        int out_point = networks.get_node_idx(path->points[i]);
        int in_point = networks.get_node_idx(path->points[i+1]);
        pair<int, int> arc_pair = pair(out_point, in_point);
        arc_set.insert(arc_pair);
    }
    return arc_set;
}

double CargoRoute::getObjVal() const {
    return objVal;
}

double CargoRoute::get_P_value(){
    double P_val = 0;
    // first subproblem
    P_val -= networks.getSea_network().getShips()[0].route.cost;

    //second subproblem
    vector<Route> routes = networks.getAir_network().getFlights()[0].routes;
    P_val -= routes[0].cost * routes.size() * TOTAL_WEEK;

    P_val += objVal;
    return P_val;
}

vector<double> CargoRoute::get_r_column() {
    vector<double> r_column;

    double* cons1_vals = cal_constr1_val();
    double* cons2_vals = cal_constr2_val();
    double* cons3_vals = cal_constr3_val();

    for(int i = 0; i < sea_arc_pairs.size(); i++){
        r_column.push_back(cons1_vals[i]);
    }
    for(int i = 0; i < air_arc_pairs.size(); i++){
        r_column.push_back(cons2_vals[i]);
    }
    for(int i = 0; i < air_arc_pairs.size(); i++){
        r_column.push_back(cons3_vals[i]);
    }
    return r_column;

}

const vector<pair<int, int>> &CargoRoute::getSea_arc_pairs() const {
    return sea_arc_pairs;
}

const vector<pair<int, int>> &CargoRoute::getAir_arc_pairs() const {
    return air_arc_pairs;
}

EntireNetwork &CargoRoute::getNetworks(){
    return networks;
}

Solution* CargoRoute::run_bp() {
    Solution* sol = branch_and_price();
    reset_bp();
    return sol;
}

void CargoRoute::rebuild_entire_network() {
    networks.rebuild_networks();
}

void CargoRoute::reset_bp() {

    for(auto &path: all_paths){
        path->reduced_cost = 0;
        Point *cur,*next;
        for(int p = 0; p < path->points.size()-1; p++){
            cur = &path->points[p];
            next = &path->points[p+1];
            arcs[networks.get_node_idx(*cur)][networks.get_node_idx(*next)]->fixed_profit = 0;
            arcs[networks.get_node_idx(*cur)][networks.get_node_idx(*next)]->fixed_cost = 0;
        }
    }
    incumbent = 0;
    integer_set.clear();
    all_paths.clear();
    delete[] chosen_paths;
    delete[] z;
    delete[] z_;
    delete[] u;
//    delete[] z_value;
    delete[] cons1;
    delete[] cons2;
    delete[] cons3;
    delete[] cons4;
    cons5.clear();
    cons6.clear();
    cons6.clear();
}

Solution* CargoRoute::Run_full_model() {
    path_categories = networks.getPaths_categories();
    get_available_path(path_categories, all_paths);
    arcs = networks.getArcs();
    cal_paths_profit();
    cal_paths_cost();

    z = new vector<GRBVar>[cargos.size()];
    z_ = new vector<GRBVar>[cargos.size()];
    u = new vector<GRBVar>[cargos.size()];
    target_path = new vector<Path*>[cargos.size()];
    rival_path = new vector<Path*>[cargos.size()];



    for (int k = 0 ; k < cargos.size(); k++) {
        int departure = cargos[k]->departure - 65;
        int destination = cargos[k]->destination - 65 ;

        for (const auto &path : path_categories[departure][destination]) {
            if(cargos[k]->start_time <= path->get_start_time()
               && cargos[k]->arrive_time >= path->get_end_time()) {
                if(!path->only_rival) {
                    target_path[k].emplace_back(path);
                }
                else{
                    rival_path[k].emplace_back(path);
                }
            }
        }
    }



    try {
        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);

        Var_init(model);
        for(int k = 0; k < cargos.size(); k++){
            for(int p = 0; p < target_path[k].size(); p++){
                u[k][p].set(GRB_CharAttr_VType, GRB_INTEGER);
            }
        }
        Obj_init(model);
        set_constrs(model);
        model.optimize();


        objVal = model.get(GRB_DoubleAttr_ObjVal);
        z_value = new vector<double>[cargos.size()];
        for(int k = 0; k < cargos.size(); k++){
            for(int p = 0; p < target_path[k].size(); p++){
                z_value[k].push_back(z[k][p].get(GRB_DoubleAttr_X));
            }
        }
        Solution* sol = new Solution(cargos.size(), target_path, z_value, get_P_value(), get_r_column(),
                                     networks.getSea_Air_Route());
        return sol;

    } catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }
    return 0;
}

vector<Path *> CargoRoute::find_all_paths() {
    path_categories = networks.getPaths_categories();
    get_available_path(path_categories, all_paths);
    return all_paths;
}

