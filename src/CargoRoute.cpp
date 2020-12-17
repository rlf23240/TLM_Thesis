//
// Created by Ashee on 2019/6/27.
//

#include <cstdlib>

#include "hash.hpp"
#include "CargoRoute.h"

#define COLUMN_GENERATIONS_LOG(LOG_COMMAND) TLMLOG("Column Generations", LOG_COMMAND)

CargoRoute::CargoRoute(string data)  {
    read_cargo_file(data);
    networks = new EntireNetwork(data);
    num_nodes = networks->getNumNodes();
    
    for (const auto &arc_column: networks->getArcs()) {
        for (const auto &arc: arc_column.second) {
            if (arc.second->fixed_profits.size() == 0) {
                arc.second->fixed_profits = vector<double>(cargos.size(), 0.0);
            }
        }
    }
    
    find_sea_arcs();
    find_air_arcs();
}

void CargoRoute::read_cargo_file(string data) {
    fstream file;
    file.open(data + "_cargo.txt");

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
                type = AirOnly;
            }else if(time_sensitivity == 'H' && product_value == 'L')
            {
                type = SeaBoth;
            }else if(time_sensitivity == 'L' && product_value == 'H'){
                type = AirBoth;
            }else{
                type = SeaOnly;
            }

            auto * new_cargo = new Cargo(departure, destination, starting_time, end_time, volume, weight, type);
            new_cargo->setAlpha(alpha);
            new_cargo->setBeta(beta);
            cargos.push_back(new_cargo);
        }
    }
    else {
        cout << "cargo file cannot open !!!";
    }
}

void CargoRoute::get_available_path(vector<vector<vector<Path*>>> path_categories, vector<Path*>& paths) {
    for(auto cargo: cargos){
        vector<Path*> od_available_path = path_categories[(int) cargo->departure - 65][(int) cargo->destination - 65];
        for (const auto &path: od_available_path) {
            if (is_path_feasible_for_cargo(path, cargo)) {
                paths.push_back(path);
            }
        }
    }

    for(int i = 0; i < paths.size(); i++){
        paths[i]->setIndex(i);
    }
}

double CargoRoute::cal_path_profit(int cargo_index, Path *path, Cargo *cargo) {
    double profit = 0;
    double pi = 0;
    bool only_rival = true;
    const Point *cur, *next;
    for(int p = 0; p < path->points().size()-1; p++){
        cur = &path->points()[p];
        next = &path->points()[p+1];
        
        Arc *arc = networks->getArc(networks->get_node_idx(*cur), networks->get_node_idx(*next));
        
        double phi = 1.0;
        
        bool is_starting_from_air = (arc->start_node->getLayer() == 1 || arc->start_node->getLayer() == 4);
        bool is_stop_to_air = (arc->end_node->getLayer() == 1 || arc->end_node->getLayer() == 4);
        
        bool is_air_trans = is_starting_from_air && is_stop_to_air;
        
        if (is_air_trans == true) {
            phi = MAX(1.0/EPSILON, cargo->weight/cargo->volume);
        }
        
        profit += arc->unit_profit*phi;
        
        
        pi += networks->getArc(networks->get_node_idx(*cur),
                               networks->get_node_idx(*next))->fixed_profits[cargo_index];
        if(next->layer == 0 || next->layer == 1 || next->layer == 3 || next->layer == 4) {
            only_rival = false;
        }
//        if(arcs[networks.get_node_idx(cur_node.layer,cur_node.node, cur_node.time)]
//            [networks.get_node_idx(next_node.layer,next_node.node, next_node.time)]->unit_profit == 0){
//            cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
//        }
    }
//    cout << *path << profit << endl;
    path->path_profit = profit;
    path->pi = pi;
    path->only_rival = only_rival;
    
    return profit;
}

void CargoRoute::cal_paths_cost() {
    for(const auto &path : all_paths){
        cal_path_cost(path);
    }
}

void CargoRoute::cal_path_cost(Path *path) {
    const Point *current;
    const Point *next;
    double cost = 0;
    
    // Check whether path contains rival only.
    // Not a good place to do this but...
    bool only_rival = true;
    
    for(int p = 0; p < path->points().size() -1 ; p++){
        current = &path->points()[p];
        next = &path->points()[p+1];
        double arc_cost = networks->getArc(*current, *next)->getUnitCost();
                
        // Is this a mistake I accidentally add?
        // if (current->layer == 2)
        cost += arc_cost;
        
        if(next->layer == 0 || next->layer == 1 || next->layer == 3 || next->layer == 4) {
            only_rival = false;
        }
    }

    path->path_cost = cost;
    path->last_time = path->points().back().time - path->points().front().time;
        
    path->only_rival = only_rival;
}

Solution* CargoRoute::branch_and_price() {
    try {
        target_path = new vector<Path *>[cargos.size()];
        rival_path = new vector<Path *>[cargos.size()];
        chosen_paths = vector<unordered_set<Path*>>(cargos.size());
        not_use_count = new vector<unordered_map<Path, int>>(cargos.size(), unordered_map<Path, int>());
        
        bp_init();
        priority_queue<BB_node> bb_pool;
        BB_node::cargo_size = cargos.size();
        bb_pool.push(BB_node(model_solution.objValue,
                             target_path,
                             rival_path,
                             chosen_paths,
                             not_use_count,
                             integer_set));
        bp_iter = 0;
        while (!bb_pool.empty()) {
            BB_node node = bb_pool.top();
            bp_iter += 1;
            
            cout << "BP_iter : " << bp_iter << endl;
            
            try {
                if (bp_iter > MAX_BP_ITER) {
                    //bp_iter = 0;
                    set_all_u_integer();
                    LP_relaxation();
                    break;
                }
                
                if (node.getObj() < incumbent) {
                    bb_pool.pop();
                    
                    delete[] node.getTargetPath();
                    delete[] node.getRivalPath();
                    delete node.getNotUseCount();
                    
                    continue;
                }
                
                bb_pool.pop();
                
                delete[] target_path;
                delete[] rival_path;
                delete not_use_count;
                
                chosen_paths.clear();
                
                target_path = node.getTargetPath();
                rival_path = node.getRivalPath();
                chosen_paths = node.getChosenPaths();
                integer_set = node.getIntegerSet();

                #pragma mark Column Deletion
                not_use_count = node.getNotUseCount();
                
                column_generation();
                show_model_result();
                
                if (model_solution.is_integer_solution()) {
                    break;
                }

                pair<int, Path*> kp_pair = find_kp_pair();
                
                //branching
                integer_set[kp_pair.first][*kp_pair.second] = false;
                LP_relaxation();
                bb_pool.push(BB_node(model_solution.objValue,
                                     target_path,
                                     rival_path,
                                     chosen_paths,
                                     not_use_count,
                                     integer_set));
                
                if (model_solution.is_integer_solution() && incumbent < model_solution.objValue)
                    incumbent = model_solution.objValue;
                
                integer_set[kp_pair.first][*kp_pair.second] = true;
                LP_relaxation();
                bb_pool.push(BB_node(model_solution.objValue,
                                     target_path,
                                     rival_path,
                                     chosen_paths,
                                     not_use_count,
                                     integer_set));
                
                if (model_solution.is_integer_solution() && incumbent < model_solution.objValue)
                    incumbent = model_solution.objValue;
                
            } catch(GRBException e) {
                cout << "Error code = " << e.getErrorCode() << endl;
                cout << e.getMessage() << endl;
                
                cout << "Infeasible solution. Node discarded." << endl;
                cout << "bb size: " << bb_pool.size() <<endl;
            } catch(...) {
                cout << "Exception during optimization" << endl;
            }
        }
        
        //        for(auto  &k : integer_set){
        //            for(auto &p : integer_set[k.first]){
        //                cout << k.first << " " << p.first << " " << p.second <<endl;
        //            }
        //        }
        
        Solution *sol = new Solution(cargos.size(),
                                     target_path,
                                     rival_path,
                                     model_solution.target_z,
                                     get_P_value(),
                                     get_r_column(),
                                     get_arc_usage_in_design(),
                                     networks->getSea_Air_Route());
        
        
        return sol;
    } catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    }
    return NULL;
}

void CargoRoute::bp_init() {
    path_categories = networks->getPaths_categories();
    get_available_path(path_categories, all_paths);
    
    auto& arcs = networks->getArcs();
    for (const auto &arc_column: arcs) {
        for (const auto &arc: arc_column.second) {
            if (arc.second->fixed_profits.size() == 0) {
                arc.second->fixed_profits = vector<double>(cargos.size(), 0.0);
            }
        }
    }
    
    //cal_paths_profit();
    cal_paths_cost();

    select_init_path();
    
    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env);

    #ifdef DEBUG_GUROBI_BP
        model.set(GRB_IntParam_OutputFlag, true);
    #else
        model.set(GRB_IntParam_OutputFlag, false);
    #endif
    
    Var_init(model);
    Obj_init(model);
    Constr_init(model);
    model.optimize();
    
    model_solution = BPGurobiModelSolution(model, u, z, z_, cons1, cons2, cons3, cons4, cons5, cons6, cons7);
}

void CargoRoute::select_init_path() {
    for (int k = 0 ; k < cargos.size(); k++) {
        int departure = cargos[k]->departure - 65;
        int destination = cargos[k]->destination - 65 ;

//        Path *best_path = nullptr;
        int path_count = 0 ;

        for (const auto &path : path_categories[departure][destination]) {
//            cout << path->path_profit << " " << *path ;
            if (is_path_feasible_for_cargo(path, cargos[k])
                && path_count < NUM_INIT_PATHS){
//                if(!path->only_rival) {
//                    target_path[k].emplace_back(path);
//                    path_count += 1;
//                    chosen_paths[k].insert(path->index);
//                }
                if(path->only_rival){
                    rival_path[k].emplace_back(path);
                    path_count += 1;
                    chosen_paths[k].insert(path);
                }
            }
        }
    }
}

bool CargoRoute::is_path_feasible_for_cargo(Path* path, Cargo* cargo) {
    if(cargo->start_time <= path->get_start_time() && cargo->arrive_time >= path->get_end_time()){
        if(path->type == Path::Sea && (cargo->type == SeaOnly || cargo->type == SeaBoth)){
            return true;
        }else if(path->type == Path::Air && (cargo->type == AirOnly || cargo->type == AirBoth)){
            return true;
        }else if(path->type == Path::SeaAir && (cargo->type == AirBoth || cargo->type == SeaBoth)){
            return true;
        }
        return false;
    }
    return false;
}

void CargoRoute::LP_relaxation() {
    GRBEnv env = GRBEnv();
    GRBModel model = GRBModel(env);

    #ifdef DEBUG_GUROBI_BP
        model.set(GRB_IntParam_OutputFlag, true);
    #else
        model.set(GRB_IntParam_OutputFlag, false);
    #endif
    
    for(int k = 0; k < cargos.size();k++) {
        z[k].clear();
        z_[k].clear();
        u[k].clear();
    }
    
    Var_init(model);
    Obj_init(model);
    Constr_init(model);

    model.optimize();
    
    model_solution = BPGurobiModelSolution(model, u, z, z_, cons1, cons2, cons3, cons4, cons5, cons6, cons7);
    
    #ifdef ENABLE_COLUMN_DELETION
    column_deletion();
    #endif
}

void CargoRoute::column_deletion() {
    for (int k = 0; k < cargos.size(); ++k) {
        for (int p = 0; p < target_path[k].size(); ++p) {
            Path *path = target_path[k][p];
            
            // Check column usage.
            if (model_solution.target_z[k][*path] == 0) {
                (*not_use_count)[k][*path] += 1;
            } else {
                (*not_use_count)[k][*path] = 0;
            }
            
            #ifdef DEBUG_COLUMN_GENERATIONS
            
            COLUMN_GENERATIONS_LOG("Current column (" << "Non-rival, " << k << ", " << p << ") havn't been use for " << (*not_use_count)[k][*path] << " times.");
            COLUMN_GENERATIONS_LOG(*path);
            
            #endif

            // If column not use for the time being...
            if ((*not_use_count)[k][*path] >= COLUMN_GENERATION_MAX_NOT_USE_IN_THRESHOLD) {
                (*not_use_count)[k][*path] = 0;
                
                // Erase both from target_path and chosen_paths.
                target_path[k].erase(target_path[k].begin() + p);
                integer_set[k].erase(*path);
                chosen_paths[k].erase(path);
                
                p -= 1;
            }
        }
        
        // And do it again for rival_path
        for (int n = 0; n < rival_path[k].size(); ++n) {
            Path *path = rival_path[k][n];
            
            // Check column usage.
            if (model_solution.rival_z[k][*path] == 0) {
                (*not_use_count)[k][*path] += 1;
            } else {
                (*not_use_count)[k][*path] = 0;
            }
            
            #ifdef DEBUG_COLUMN_GENERATIONS
            
            COLUMN_GENERATIONS_LOG("Current column (Rival, " << k << ", " << n << ") havn't been use for " << (*not_use_count)[k][*path] << " times.");
            COLUMN_GENERATIONS_LOG(*path);
            
            #endif
            
            // If column not use for the time being...
            if ((*not_use_count)[k][*path] >= COLUMN_GENERATION_MAX_NOT_USE_IN_THRESHOLD) {
                (*not_use_count)[k][*path] = 0;
                
                // Erase both from rival_path and chosen_paths.
                rival_path[k].erase(rival_path[k].begin() + n);
                chosen_paths[k].erase(path);
                
                n -= 1;
            }
        }
    }
}

void CargoRoute::column_generation() {
    Path *best_target_path;
    Path *best_rival_path;
    int iter = 0;
    
    double previous = 99999;
    
    int cont_in_thres = 0;
        
    while (true) {
        LP_relaxation();
        update_arcs();
        
        pair<Path*, int> target_path_pair = select_most_profit_target_path();
        pair<Path*, int> rival_path_pair = select_most_profit_rival_path();
        best_target_path = target_path_pair.first;
        best_rival_path = rival_path_pair.first;
        
        double objValue = model_solution.objValue;
        double delta = abs(objValue-previous);
        previous = objValue;
        
        #ifdef DEBUG_COLUMN_GENERATIONS_REDUCED_COST
        if (target_path_pair.first) {
            COLUMN_GENERATIONS_LOG("Selected target path reduced cost: " << target_path_pair.first->reduced_cost);
        }
        
        if (rival_path_pair.first) {
            COLUMN_GENERATIONS_LOG("Selected rival path reduced cost: " << rival_path_pair.first->reduced_cost);
        }
        #endif
        
        if (cont_in_thres >= COLUMN_GENERATION_MAX_CONTINUE_IN_THRESHOLD) {
            break;
        }
        
        double target_reduced_cost = -99999;
        if (target_path_pair.first != NULL) {
            target_reduced_cost = target_path_pair.first->reduced_cost;
            if (target_reduced_cost > 0.0) {
                append_column(target_path_pair.first, target_path_pair.second);
            }
        }
        
        double rival_reduced_cost = -99999;
        if (rival_path_pair.first != NULL) {
            rival_reduced_cost = rival_path_pair.first->reduced_cost;
            if (rival_reduced_cost > 0.0) {
                append_column(rival_path_pair.first, rival_path_pair.second);
            }
        }
        
        if (target_reduced_cost <= 0.0 && rival_reduced_cost <= 0.0) {
            break;
        }
        
        if (delta <= COLUMN_GENERATION_THRESHOLE) {
            cont_in_thres += 1;
            
            #ifdef DEBUG_COLUMN_GENERATIONS_COLUMN_DELETIONS
            COLUMN_GENERATIONS_LOG("Iter " << iter << ": in thres " << delta << "/" << COLUMN_GENERATION_THRESHOLE);
            COLUMN_GENERATIONS_LOG("Continued " << cont_in_thres << "/" << COLUMN_GENERATION_MAX_CONTINUE_IN_THRESHOLD << " times.");
            #endif
            
        } else {
            #ifdef DEBUG_COLUMN_GENERATIONS_COLUMN_DELETIONS
            COLUMN_GENERATIONS_LOG("Iter " << iter << ": out of thres," << delta << "/" << COLUMN_GENERATION_THRESHOLE);
            COLUMN_GENERATIONS_LOG("Continued " << cont_in_thres << "/" << COLUMN_GENERATION_MAX_CONTINUE_IN_THRESHOLD << " times.");
            #endif
            
            cont_in_thres = 0;
        }
        
        #ifdef DEBUG_COLUMN_GENERATIONS_OBJ
        COLUMN_GENERATIONS_LOG("Iter : " << iter << " Obj : " << objValue);
        #endif
                
        iter++;
    }
    
}

void CargoRoute::Var_init(GRBModel &model) {
    z = vector<unordered_map<Path, GRBVar>>();
    z_ = vector<unordered_map<Path, GRBVar>>();
    u = vector<unordered_map<Path, GRBVar>>();
    
    for(int k = 0; k < cargos.size(); k++){
        z.push_back(unordered_map<Path, GRBVar>());
        z_.push_back(unordered_map<Path, GRBVar>());
        u.push_back(unordered_map<Path, GRBVar>());
        
        for(auto path: target_path[k]){
            z[k][*path] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS);
            u[k][*path] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS);
            
            // TODO: Is this is vaild force stop method?
            /*if (bp_iter > MAX_BP_ITER) {
                u[k].push_back(model.addVar(0.0, 1.0, 0.0, GRB_BINARY));
            } else {
                u[k].push_back(model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS));
            }*/
        }
        
        for(auto path: rival_path[k]){
            z_[k][*path] = model.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS);
        }
    }
}

void CargoRoute::Obj_init(GRBModel &model) {
    GRBLinExpr obj = 0;
    for(int k = 0; k < cargos.size(); k++) {
        for (int p = 0; p < target_path[k].size(); p++) {
            Path* path = target_path[k][p];
            
            double target_path_profit = cal_path_profit(k, path, cargos[k]);
                        
            obj += z[k][*path] * cargos[k]->volume * target_path_profit;
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
    cons1 = vector<GRBConstr>();
    for(int k = 0; k < cargos.size(); k++){
        GRBLinExpr expr = 0;
        for(auto path: target_path[k]){
            expr += z[k][*path];
        }
        for(auto path: rival_path[k]){
            expr += z_[k][*path];
        }
        cons1.push_back(model.addConstr(expr <= 1, "Cons1" + to_string(k)));
    }
}

void CargoRoute::set_constr2(GRBModel &model) {
    cons2 = vector<unordered_map<Path, GRBConstr>>();
    for(int k = 0; k < cargos.size(); k++){
        unordered_map<Path, GRBConstr> constrs = unordered_map<Path, GRBConstr>();
        for(int p = 0; p < target_path[k].size(); p++){
            Path* path = target_path[k][p];
            constrs[*path] = model.addConstr(z[k][*path] <= u[k][*path],
                                            "Cons2" + to_string(k) + to_string(p));
        }
        cons2.push_back(constrs);
    }
}

void CargoRoute::cal_e() {
    cal_v();
    
    e = vector<unordered_map<Path, double>>(cargos.size());
    e_ = vector<unordered_map<Path, double>>(cargos.size());

    for(int k = 0; k < cargos.size(); k++){
        double sum = 0;
        for(auto path: target_path[k]) {
            sum += exp(v[k][*path]);
        }
        for(auto path: rival_path[k]) {
            sum += exp(v_[k][*path]);
        }
        
        for(auto path: target_path[k]){
            e[k][*path] = exp(v[k][*path]) / sum ;
        }
        
        for(auto path: target_path[k]){
            e_[k][*path] = exp(v_[k][*path]) / sum ;
        }
    }
}

void CargoRoute::cal_v() {
    v = vector<unordered_map<Path, double>>(cargos.size());
    v_ = vector<unordered_map<Path, double>>(cargos.size());
    
    for(int k = 0; k < cargos.size(); k++){
        for(auto path: target_path[k]){
            v[k][*path] = cargos[k]->alpha * path->path_cost + cargos[k]->beta * path->last_time;
        }
        
        for(auto path: rival_path[k]){
            v_[k][*path] = cargos[k]->alpha * path->path_cost + cargos[k]->beta * path->last_time;
        }
    }
}

void CargoRoute::set_constr3(GRBModel &model) {
    cons3 = vector<unordered_map<Path, GRBConstr>>();
    for(int k = 0; k < cargos.size(); k++){
        unordered_map<Path, GRBConstr> constrs = unordered_map<Path, GRBConstr>();
        for(int p = 0; p < target_path[k].size(); p++) {
            GRBLinExpr lhs, rhs;
            for(auto rival_path: rival_path[k]){
                lhs += e_[k][*rival_path];
                rhs += z_[k][*rival_path];
            }
            
            Path* path = target_path[k][p];
            constrs[*path] = model.addConstr(
                lhs* z[k][*path] - e[k][*path] * rhs <= 0,
                "cons3" + to_string(k) + to_string(p)
            );
        }
        cons3.push_back(constrs);
    }
}

void CargoRoute::set_constr4(GRBModel &model) {
    cons4 = vector<unordered_map<Path, GRBConstr>>();
    for(int k = 0; k < cargos.size(); k++){
        unordered_map<Path, GRBConstr> constrs = unordered_map<Path, GRBConstr>();
        for(int p = 0; p < target_path[k].size();p++){
            GRBLinExpr lhs, rhs;
            for(auto rival_path: rival_path[k]){
                lhs += e_[k][*rival_path];
                rhs += z_[k][*rival_path];
            }
            
            Path* path = target_path[k][p];
            constrs[*path] = model.addConstr(
                lhs * z[k][*path] - rhs * e[k][*path] - u[k][*path] >= - 1,
                "cons4" + to_string(k) + to_string(p)
            );
        }
        cons4.push_back(constrs);
    }
}

void CargoRoute::set_constr5(GRBModel &model) {
    vector<Ship> ships = networks->get_cur_ships();
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
                        Path* path = target_path[k][p];
                        vector<Point> points = path->points();
                        for (int n = 0; n < points.size() - 1; n++) {
                            if (points[n] == cur_point && points[n + 1] == next_point) {
//                            cout << k << " " << p << " "<< points[n] << cur_point << " "<<  points[n+1] <<  next_point << endl;
                                lhs += cargos[k]->volume * z[k][*path];
                            }
                        }
                    }
                }
                cons5[networks->get_node_idx(cur_point)][networks->get_node_idx(next_point)] = model.addConstr(
                        lhs <= ship.volume_ub);
            }
//        cout << endl;
        }
    }
}

void CargoRoute::set_constr6(GRBModel &model) {
    vector<Flight> flights = networks->get_cur_flights();
    for(const auto &flight : flights) {
        for(int week = 0 ; week < TIME_PERIOD / 7; week++) {
            for(const auto &route : flight.routes){
                vector<string> nodes = route.nodes;
                for(int i = 0; i < nodes.size() -1; i++){
                    Point cur_point = Point(4, (int) nodes[i][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(nodes[i].substr(1)));
                    Point next_point = Point(4, (int) nodes[i+1][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(nodes[i+1].substr(1)));
                    GRBLinExpr lhs = 0;
                    for(int k = 0; k < cargos.size(); k++){
                        for(int p = 0; p < target_path[k].size(); p++){
                            Path* path = target_path[k][p];
                            vector<Point> points = path->points();
                            
                            for(int n = 0; n < points.size()-1; n++){
                                if(points[n] == cur_point && points[n+1] == next_point){
//                                    cout << k << " " << p << " "<< points[n] << cur_point << " "<<  points[n+1] <<  next_point << endl;
                                    lhs += cargos[k]->volume * z[k][*path];
                                }
                            }
                        }
                    }
                    cons6[networks->get_node_idx(cur_point)][networks->get_node_idx(next_point)] = model.addConstr(lhs <= flight.volume_ub);
                }
            }
        }
    }
}

void CargoRoute::set_constr7(GRBModel &model) {
    vector<Flight> flights = networks->get_cur_flights();
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
                            Path* path = target_path[k][p];
                            vector<Point> points = path->points();
                            
                            for(int n = 0; n < points.size()-1; n++){
                                if(points[n] == cur_point && points[n+1] == next_point){
//                                    cout << k << " " << p << " "<< points[n] << cur_point << " "<<  points[n+1] <<  next_point << endl;
                                    lhs += cargos[k]->weight * z[k][*path];
                                }
                            }
                        }
                    }
                    cons7[networks->get_node_idx(cur_point)][networks->get_node_idx(next_point)] = model.addConstr(lhs <= flight.weight_ub);
                }
            }
        }
    }
}

void CargoRoute::set_complicate_constr1(GRBModel &model) {
    for (auto &sea_arc_pair : sea_arc_pairs) {
        GRBLinExpr lhs;
        double rhs;
        
        lhs = complicate_constr_lhs_volume(sea_arc_pair.first, sea_arc_pair.second);
        rhs = complicate_sea_rhs(sea_arc_pair.first, sea_arc_pair.second, networks->getSea_network()->getShips()[0].volume_ub);

        model.addConstr(lhs <= rhs);
    }
}

void CargoRoute::set_complicate_constr2(GRBModel &model) {
    for (auto &air_arc_pair : air_arc_pairs) {
        GRBLinExpr lhs;
        double rhs;
        
        lhs= complicate_constr_lhs_volume(air_arc_pair.first, air_arc_pair.second);
        rhs= complicate_air_rhs(air_arc_pair.first, air_arc_pair.second, networks->getAir_network()->getFlights()[0].volume_ub);

        model.addConstr(lhs <= rhs);
    }
}

void CargoRoute::set_complicate_constr3(GRBModel &model) {
    for (auto &air_arc_pair : air_arc_pairs) {
        GRBLinExpr lhs;
        double rhs;
        
        lhs = complicate_constr_lhs_weight(air_arc_pair.first, air_arc_pair.second);
        rhs = complicate_air_rhs(air_arc_pair.first, air_arc_pair.second, networks->getAir_network()->getFlights()[0].weight_ub);

        model.addConstr(lhs <= rhs);
    }
}

void CargoRoute::set_integer(GRBModel &model) {
    for (int k = 0; k < cargos.size(); k++){
        for (int p = 0; p < target_path[k].size(); p++){
            Path *path = target_path[k][p];
            
            auto iter = integer_set[k].find(*path);
            if (iter != integer_set[k].end()) {
                model.addConstr(u[k][*path] == (*iter).second);
            }
        }
    }
}

void CargoRoute::set_all_u_integer() {
    for (int k = 0; k < cargos.size(); k++){
        for (int p = 0; p < target_path[k].size(); p++){
            Path *path = target_path[k][p];
            
            if (integer_set[k].find(*path) == integer_set[k].end()) {
                if (model_solution.u[k][*path] > MU_THRESHOLD) {
                    integer_set[k][Path(*path)] = true;
                } else {
                    integer_set[k][Path(*path)] = false;
                }
            }
        }
    }
}

void CargoRoute::update_arcs() {
    vector<Ship> ships = networks->get_cur_ships();
    for(int w = 0; w < TOTAL_WEEK; w++) {
        for (const auto &ship : ships) {
            vector<string> nodes = ship.route.nodes;
            for (int i = 0; i < nodes.size() - 1; i++) {
                Point cur_point = Point(3, (int) nodes[i][0] - 65, stoi(nodes[i].substr(1)) + w * 7 * TIME_SLOT_A_DAY);
                Point next_point = Point(3, (int) nodes[i + 1][0] - 65, stoi(nodes[i + 1].substr(1)) + w * 7 * TIME_SLOT_A_DAY);
                if(next_point.time >= TOTAL_TIME_SLOT) break;
                double pi5 = model_solution.constr5_pi_values[networks->get_node_idx(cur_point)][networks->get_node_idx(next_point)];
                
                for (int k = 0; k < cargos.size(); ++k) {
                    double cargo_volume = cargos[k]->volume;
                    networks->getArc(cur_point, next_point)->minus_fixed_profit(k, cargo_volume*pi5);
                }
            }
        }
    }

    vector<Flight> flights = networks->get_cur_flights();
    for (const auto &flight : flights) {
        for (int week = 0; week < TIME_PERIOD / 7; week++) {
            for (const auto &route : flight.routes) {
                vector<string> nodes = route.nodes;
                for (int i = 0; i < nodes.size() - 1; i++) {
                    Point cur_point = Point(4, (int) nodes[i][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(nodes[i].substr(1)));
                    Point next_point = Point(4, (int) nodes[i+1][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(nodes[i+1].substr(1)));
                    
                    if (next_point.time >= TOTAL_TIME_SLOT) break;
                    
                    double pi6 = model_solution.constr6_pi_values[networks->get_node_idx(cur_point)][networks->get_node_idx(next_point)];
                    double pi7 =  model_solution.constr7_pi_values[networks->get_node_idx(cur_point)][networks->get_node_idx(next_point)];
                    
                    for (int k = 0; k < cargos.size(); ++k) {
                        double cargo_volume = cargos[k]->volume;
                        double cargo_weight = cargos[k]->weight;
                        
                        networks->getArc(cur_point, next_point)->minus_fixed_profit(k, cargo_volume*pi6);
                        networks->getArc(cur_point, next_point)->minus_fixed_profit(k, cargo_weight*pi7);
                    }
                }
            }
        }
    }

}

pair<Path*, int> CargoRoute::select_most_profit_target_path() {
    Path* best_path = nullptr;
    int best_k = -1;
    for (int k = 0 ; k < cargos.size(); k++) {
        int departure = cargos[k]->departure - 65;
        int destination = cargos[k]->destination - 65;

        for (const auto &path: path_categories[departure][destination]) {
            if (path->only_rival == true) {
                continue;
            }
            
            cal_target_path_reduced_cost(path, k);
            if(is_path_feasible_for_cargo(path, cargos[k]) &&
               chosen_paths[k].find(path) == chosen_paths[k].end()) {
                if (!best_path || (best_path->reduced_cost < path->reduced_cost)) {
                    best_path = path;
                    best_k = k;
                }
            }
        }
    }
    return make_pair(best_path, best_k);
}

pair<Path*, int> CargoRoute::select_most_profit_rival_path() {
    Path* best_path = nullptr;
    int best_k = -1;
    for (int k = 0 ; k < cargos.size(); k++) {
        int departure = cargos[k]->departure - 65;
        int destination = cargos[k]->destination - 65;

        for (const auto &path: path_categories[departure][destination]) {
            if (path->only_rival == false) {
                continue;
            }
            
            cal_rival_path_reduced_cost(path, k);
            if(is_path_feasible_for_cargo(path, cargos[k]) &&
                chosen_paths[k].find(path) == chosen_paths[k].end()) {
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
        chosen_paths[best_k].insert(best_path);
    }
}

void CargoRoute::cal_target_path_reduced_cost(Path* path, int cargo_index) {
    double reduced_cost = 0;
    const Point *cur,*next;
    
    Cargo* cargo =  cargos[cargo_index];
    
    // Volume
    double cargo_volume = cargo->volume;
    double cargo_weight = cargo->weight;
    
    for(int p = 0; p < path->points().size()-1; p++){
        cur = &path->points()[p];
        next = &path->points()[p+1];
        
        Arc *arc = networks->getArc(*cur, *next);
        
        double phi = 1.0;
        
        bool is_starting_from_air = (arc->start_node->getLayer() == 1 || arc->start_node->getLayer() == 4);
        bool is_stop_to_air = (arc->end_node->getLayer() == 1 || arc->end_node->getLayer() == 4);
        
        bool is_air_trans = is_starting_from_air && is_stop_to_air;
        
        if (is_air_trans == true) {
            phi = MAX(1.0/EPSILON, cargo_weight/cargo_volume);
        }
        
        reduced_cost += cargo_volume*phi*arc->unit_profit + arc->fixed_profits[cargo_index];
    }

    reduced_cost -= model_solution.constr1_pi_values[cargo_index];

    int path_index = -1;
    for(int p = 0; p < target_path[cargo_index].size(); p++){
        if (target_path[cargo_index][p] == path) {
            path_index = p;
            break;
        }
    }
    
    if (path_index != -1) {
        reduced_cost -= model_solution.constr2_pi_values[cargo_index][*path];
        for(int n = 0; n < rival_path[cargo_index].size(); n++){
            reduced_cost -= (e_[cargo_index][*path] * model_solution.constr3_pi_values[cargo_index][*path]);
            reduced_cost += (e_[cargo_index][*path] * model_solution.constr4_pi_values[cargo_index][*path]);
        }
    }
//    reduced_cost = MAX(reduced_cost, 0);
    path->reduced_cost = reduced_cost;
}

void CargoRoute::cal_rival_path_reduced_cost(Path* path, int cargo_index) {
    double reduced_cost = 0;

    reduced_cost -= model_solution.constr1_pi_values[cargo_index];
    
    for(int p = 0; p < target_path[cargo_index].size(); p++){
        reduced_cost += (e[cargo_index][*path] * model_solution.constr3_pi_values[cargo_index][*path]);
        reduced_cost -= (e[cargo_index][*path] * model_solution.constr4_pi_values[cargo_index][*path]);
    }
//    reduced_cost = MAX(reduced_cost, 0);
    path->reduced_cost = reduced_cost;
}

pair<int, Path*> CargoRoute::find_kp_pair() {
    pair<int, Path*> kp_pair;
    double closest_diff = 1;
    for(int k = 0; k < cargos.size(); k++) {
        for(int p = 0; p < target_path[k].size(); p++){
            Path* path = target_path[k][p];
            
            double u_value = model_solution.u[k][*path];
            double diff = abs((u_value - 0.5));
            if( diff < closest_diff) {
                kp_pair = pair<int, Path*>(k, path);
                closest_diff = diff;
            }
        }
    }
    return kp_pair;
}

void CargoRoute::find_sea_arcs() {
    /*SeaNetwork *seaNetwork = networks->getSea_network();
    for (Ship ship: seaNetwork->getDesignedShips()) {
        vector<string> nodes = ship.route.nodes;
        
        string in_node = nodes.front();
        for (auto iter = nodes.begin()+1; iter != nodes.end(); ++iter) {
            string out_node = *iter;
            
            Point out_point = Point(0, (int) out_node[0] -65, stoi(out_node.substr(1)));
            Point in_point = Point(0, (int) in_node[0] -65, stoi(in_node.substr(1)));

            sea_arc_pairs.emplace_back(networks->get_node_idx(out_point), networks->get_node_idx(in_point));
            
            in_node = out_node;
        }
    }*/
    
    SeaNetwork *seaNetwork = networks->getSea_network();
    for(int i = 65 ; i < 65+num_nodes ;i++){
        for(int t = 0; t < TOTAL_TIME_SLOT; t++){
            for(const auto& arc : seaNetwork->nodes[(char) i][t]->out_arcs) {
                string out_node = arc->start_node->getName();
                string in_node = arc->end_node->getName();
                Point out_point = Point(0, (int) out_node[0] -65, stoi(out_node.substr(1)));
                Point in_point = Point(0, (int) in_node[0] -65, stoi(in_node.substr(1)));

                sea_arc_pairs.emplace_back(networks->get_node_idx(out_point), networks->get_node_idx(in_point));
//                cout << out_point << " " << in_point << " " << networks.get_node_idx(out_point) << " " << networks.get_node_idx(in_point) << networks.idx_to_point(networks.get_node_idx(out_point)) << " "<< networks.idx_to_point(networks.get_node_idx(in_point)) << " " << endl;
            }
        }
    }
}

void CargoRoute::find_air_arcs() {
    /*AirNetwork *airNetwork = networks->getAir_network();
    for (Flight flight: airNetwork->getDesignedFlights()) {
        for (Route route: flight.routes) {
            vector<string> nodes = route.nodes;
            
            string in_node = nodes.front();
            for (auto iter = nodes.begin()+1; iter != nodes.end(); ++iter) {
                string out_node = *iter;
                
                Point out_point = Point(0, (int) out_node[0] -65, stoi(out_node.substr(1)));
                Point in_point = Point(0, (int) in_node[0] -65, stoi(in_node.substr(1)));

                air_arc_pairs.emplace_back(networks->get_node_idx(out_point), networks->get_node_idx(in_point));
                
                in_node = out_node;
            }
        }
    }*/
    
    AirNetwork *airNetwork = networks->getAir_network();
    for(int i = 65 ; i < 65+num_nodes ;i++){
        for(int t = 0; t < TOTAL_TIME_SLOT; t++){
            for(const auto& arc : airNetwork->nodes[(char) i][t]->out_arcs){
                string out_node = arc->start_node->getName();
                string in_node = arc->end_node->getName();
                Point out_point = Point(1, (int) out_node[0] -65, stoi(out_node.substr(1)));
                Point in_point = Point(1, (int) in_node[0] -65, stoi(in_node.substr(1)));

                air_arc_pairs.emplace_back(networks->get_node_idx(out_point), networks->get_node_idx(in_point));
//                cout << out_point << " " << in_point << " " << networks.get_node_idx(out_point) << " " << networks.get_node_idx(in_point) << " " << networks.idx_to_point(networks.get_node_idx(out_point)) << " "<< networks.idx_to_point(networks.get_node_idx(in_point)) << " " << endl;
            }
        }
    }
}

void CargoRoute::arcs_to_file(string data) {
	ofstream sea_file;
	sea_file.open(data + "_sea_arcs.csv");
	for(const auto &arc: sea_arc_pairs){

        Point first = networks->idx_to_point(arc.first);
        Point second = networks->idx_to_point(arc.second);
		sea_file << to_string(first.layer) + (char) (first.node +65) + to_string(first.time) << ",";
		sea_file << to_string(second.layer) + (char) (second.node +65) + to_string(second.time);
		cout << to_string(first.layer) + (char) (first.node +65) + to_string(first.time) << " " << to_string(second.layer) + (char) (second.node +65) + to_string(second.time) << endl;

		sea_file << "\n";
	}
	sea_file.close();

	ofstream air_file;
	air_file.open(data + "_air_arcs.csv");
	for(const auto &arc: air_arc_pairs){

        Point first = networks->idx_to_point(arc.first);
        Point second = networks->idx_to_point(arc.second);
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
        val[i] = get_sea_complicate_constr_val(sea_arc_pairs[i].first, sea_arc_pairs[i].second, networks->getSea_network()->getShips()[0].volume_ub);
    }

    return val;
}

double* CargoRoute::cal_constr2_val() {
    double* val = new double[air_arc_pairs.size()];
    for(int i = 0; i < air_arc_pairs.size(); i++){
        val[i] = get_air_complicate_constr_val_volume(air_arc_pairs[i].first, air_arc_pairs[i].second, networks->getAir_network()->getFlights()[0].volume_ub);
    }
    return val;
}

double* CargoRoute::cal_constr3_val() {
    double* val = new double[air_arc_pairs.size()];
    for(int i = 0; i < air_arc_pairs.size(); i++){
        val[i] = get_air_complicate_constr_val_weight(air_arc_pairs[i].first, air_arc_pairs[i].second, networks->getAir_network()->getFlights()[0].weight_ub);
    }
    return val;
}

double CargoRoute::get_sea_complicate_constr_val(int start_idx, int end_idx, int ub) {
    double val = 0;
    // left hand side

    for(int k = 0; k < cargos.size(); k++){
        for (int p = 0; p < target_path[k].size(); p++) {
            Path* path = target_path[k][p];
            double z_value = model_solution.target_z[k][*path];
            for(int i = 0; i < path->points().size()-1; i++){
                int out_point_idx = networks->get_node_idx(path->points()[i]);
                int in_point_idx = networks->get_node_idx(path->points()[i+1]);
                if(out_point_idx == start_idx && in_point_idx == end_idx) {
                    val += cargos[k]->volume * z_value;
                }
            }
        }
    }
    
    //right hand side
    Route route = networks->getSea_network()->getShips()[0].route;
    for(int i = 0; i < route.nodes.size()-1; i++){
        Point out_point = Point(0, (int) route.nodes[i][0] - 65 , stoi(route.nodes[i].substr(1)));
        Point in_point = Point(0, (int) route.nodes[i+1][0] - 65 , stoi(route.nodes[i+1].substr(1)));
        int out_point_idx = networks->get_node_idx(out_point);
        int in_point_idx = networks->get_node_idx(in_point);
        if(out_point_idx == start_idx && in_point_idx == end_idx) {
            val -= ub;
        }
    }

    return val;
}

double CargoRoute::get_air_complicate_constr_val_volume(int start_idx, int end_idx, int ub) {
    double val = 0;
    // left hand side

    for(int k = 0; k < cargos.size(); k++){
        for (int p = 0; p < target_path[k].size(); p++) {
            Path* path = target_path[k][p];
            double z_value = model_solution.target_z[k][*path];
            for(int i = 0; i < path->points().size()-1; i++){
                int out_point_idx = networks->get_node_idx(path->points()[i]);
                int in_point_idx = networks->get_node_idx(path->points()[i+1]);
                if(out_point_idx == start_idx && in_point_idx == end_idx){
                    val += cargos[k]->volume * z_value;
                }
            }
        }
    }
    //right hand side
    for(int week = 0; week < TIME_PERIOD / 7; week++) {
        for(const auto &route : networks->getAir_network()->getFlights()[0].routes) {
            for (int i = 0; i < route.nodes.size() - 1; i++) {
                Point out_point = Point(1, (int) route.nodes[i][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i].substr(1)));
                Point in_point = Point(1, (int) route.nodes[i+1][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i + 1].substr(1)));
                int out_point_idx = networks->get_node_idx(out_point);
                int in_point_idx = networks->get_node_idx(in_point);
                if (out_point_idx == start_idx && in_point_idx == end_idx) {
                    val -= ub;
                }
            }
        }
    }

    return val;
}

double CargoRoute::get_air_complicate_constr_val_weight(int start_idx, int end_idx, int ub) {
    double val = 0;
    // left hand side

    for(int k = 0; k < cargos.size(); k++){
        for (int p = 0; p < target_path[k].size(); p++) {
            Path* path = target_path[k][p];
            double z_value = model_solution.target_z[k][*path];
            for(int i = 0; i < path->points().size()-1; i++){
                int out_point_idx = networks->get_node_idx(path->points()[i]);
                int in_point_idx = networks->get_node_idx(path->points()[i+1]);
                if(out_point_idx == start_idx && in_point_idx == end_idx){
                    val += cargos[k]->weight * z_value;
                }
            }
        }
    }
    //right hand side
    for(int week = 0; week < TIME_PERIOD / 7; week++) {
        for(const auto &route : networks->getAir_network()->getFlights()[0].routes) {
            for (int i = 0; i < route.nodes.size() - 1; i++) {
                Point out_point = Point(1, (int) route.nodes[i][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i].substr(1)));
                Point in_point = Point(1, (int) route.nodes[i+1][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i + 1].substr(1)));
                int out_point_idx = networks->get_node_idx(out_point);
                int in_point_idx = networks->get_node_idx(in_point);
                if (out_point_idx == start_idx && in_point_idx == end_idx) {
                    val -= ub;
                }
            }
        }
    }

    return val;
}

GRBLinExpr CargoRoute::complicate_constr_lhs_volume(int start_idx, int end_idx){
    GRBLinExpr cons = 0;
    for(int k = 0; k < cargos.size(); k++){
        for (int p = 0; p < target_path[k].size(); p++) {
            Path* path = target_path[k][p];
            
            for(int i = 0; i < path->points().size()-1; i++){
                int out_point_idx = networks->get_node_idx(path->points()[i]);
                int in_point_idx = networks->get_node_idx(path->points()[i+1]);
                if(out_point_idx == start_idx && in_point_idx == end_idx){
                    cons += cargos[k]->volume * z[k][*path];
                }
            }
        }
    }
    return cons;
}

GRBLinExpr CargoRoute::complicate_constr_lhs_weight(int start_idx, int end_idx){
    GRBLinExpr cons = 0;
    for(int k = 0; k < cargos.size(); k++){
        for (int p = 0; p < target_path[k].size(); p++) {
            Path* path = target_path[k][p];
            for(int i = 0; i < path->points().size()-1; i++){
                int out_point_idx = networks->get_node_idx(path->points()[i]);
                int in_point_idx = networks->get_node_idx(path->points()[i+1]);
                if(out_point_idx == start_idx && in_point_idx == end_idx){
                    cons += cargos[k]->weight * z[k][*path];
                }
            }
        }
    }
    return cons;
}

double CargoRoute::complicate_sea_rhs(int start_idx, int end_idx, int ub) {
    double val = 0;
    Route route = networks->getSea_network()->getShips()[0].route;
    for(int w = 0; w < TOTAL_WEEK ; w++) {
        for (int i = 0; i < route.nodes.size() - 1; i++) {
            Point out_point = Point(0, (int) route.nodes[i][0] - 65, stoi(route.nodes[i].substr(1)) + w * 7 * TIME_SLOT_A_DAY);
            Point in_point = Point(0, (int) route.nodes[i + 1][0] - 65, stoi(route.nodes[i + 1].substr(1))+ w * 7 * TIME_SLOT_A_DAY);
            int out_point_idx = networks->get_node_idx(out_point);
            int in_point_idx = networks->get_node_idx(in_point);
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
        for(const auto &route : networks->getAir_network()->getFlights()[0].routes) {
            for (int i = 0; i < route.nodes.size() - 1; i++) {
                Point out_point = Point(1, (int) route.nodes[i][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i].substr(1)));
                Point in_point = Point(1, (int) route.nodes[i+1][0] - 65, week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i + 1].substr(1)));
                int out_point_idx = networks->get_node_idx(out_point);
                int in_point_idx = networks->get_node_idx(in_point);
                if (out_point_idx == start_idx && in_point_idx == end_idx) {
                    val += ub;
                    return val;
                }
            }
        }
    }
    return val;
}

// TODO: Should move to ModelSolution?
void CargoRoute::show_model_result() {
    cout << "Obj : " << model_solution.objValue << "\tIncumbent : " << incumbent << endl;
    cout << "==============LP relaxation===============" << endl;
}

unordered_set<pair<int, int>, pair_hash> CargoRoute::get_arc_set(Path *path) {
    unordered_set<pair<int, int>, pair_hash> arc_set{};
    for(int i = 0; i < path->points().size()-1; i++){
        int out_point = networks->get_node_idx(path->points()[i]);
        int in_point = networks->get_node_idx(path->points()[i+1]);
        pair<int, int> arc_pair = pair<int, int>(out_point, in_point);
        arc_set.insert(arc_pair);
    }
    return arc_set;
}

double CargoRoute::getObjVal() const {
    return model_solution.objValue;
}

double CargoRoute::get_P_value(){
    double P_val = 0;
    if(is_designed_route_added == true) {
        if(networks->get_cur_flights().size() <= 2) {
            //first subproblem
            double sea_cost = networks->getSea_network()->getShips()[0].route.getCost(networks->getSea_network());
            P_val -= sea_cost;
            
            #ifdef DEBUG_SUBPROBLEMS_ONE
            TLMLOG("Subproblem 2", networks->getSea_network()->getShips()[0].route << "Cost: " << sea_cost);
            #endif
            
            //second subproblem
            vector<Route> routes = networks->getAir_network()->getFlights()[0].routes;
            double air_cost = routes[0].getCost(networks->getAir_network());
            P_val -= air_cost * networks->getAir_network()->getFlights()[0].freq * TOTAL_WEEK;
            
            #ifdef DEBUG_SUBPROBLEMS_TWO
            TLMLOG("Subproblem 2", routes[0] << "/" << networks->getAir_network()->getFlights()[0].freq
                   << "/" << TOTAL_WEEK << "\n" << "Cost: " << air_cost);
            #endif
        }
    }
    double total_profit = 0.0;
    int cargo_size = (int)cargos.size();
    for (int i = 0; i < cargo_size; ++i) {
        for (auto path: target_path[i]) {
            
            Cargo *cargo = cargos[i];
            
            double profit = model_solution.target_z[i][*path]*cargo->volume*path->path_profit;
            
            total_profit += profit;

            
            #ifdef DEBUG_SUBPROBLEMS_PROFIT
            TLMLOG("Profit calculations", *path << profit);
            #endif
        }
    }
    
    #ifdef DEBUG_SUBPROBLEMS_PROFIT
    TLMLOG("Profit calculations", "Total profit: " << total_profit);
    #endif
    
    P_val += total_profit;
    // What is this?
    /*if(networks->get_cur_flights().size() <= 2)
        P_val += 50000 + 487.0 * cargos.size() / 2 * num_nodes;
    else
        P_val = (P_val + 10000) * 9.2;*/
    return P_val;
}



double CargoRoute::get_original_profit(){
    /*double profit = 0;
    if(is_designed_route_added == true) {
        if(networks->get_cur_flights().size() <= 2) {
            //first subproblem
            P_val -= networks->getSea_network()->getShips()[0].route.cost;
            //    //second subproblem
            vector<Route> routes = networks->getAir_network()->getFlights()[0].routes;
            P_val -= routes[0].cost * networks->getAir_network()->getFlights()[0].freq * TOTAL_WEEK;
        }
    }
    P_val += objVal;
    if(networks->get_cur_flights().size() <= 2)
        P_val += 50000 + 487.0 * cargos.size() / 2 * num_nodes;
    else
        P_val = (P_val + 10000) * 9.2;
    return P_val;*/
    
    return 0;
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

unordered_map<pair<int, int>, bool, pair_hash> CargoRoute::get_arc_usage_in_design() {
    auto is_arc_used_in_design = unordered_map<pair<int, int>, bool, pair_hash>();
    for (const auto &arc: sea_arc_pairs) {
        is_arc_used_in_design[arc] = false;
    }
    
    for (const auto &arc: air_arc_pairs) {
        is_arc_used_in_design[arc] = false;
    }
    
    Route route = getNetworks()->getSea_network()->getShips()[0].route;
    string sea_prev = route.nodes[0];
    for (int i = 1; i < route.nodes.size(); ++i) {
        string sea_current = route.nodes[i];
        
        Point prev_point = Point(0, (int) sea_prev[0] -65, stoi(sea_prev.substr(1)));
        Point current_point = Point(0, (int) sea_current[0] -65, stoi(sea_current.substr(1)));
        
        int prev_index = getNetworks()->get_node_idx(prev_point);
        int current_index = getNetworks()->get_node_idx(current_point);
        
        is_arc_used_in_design[pair<int, int>(prev_index, current_index)] = true;
        
        sea_prev = sea_current;
    }
    
    vector<Route> routes = getNetworks()->getAir_network()->getFlights()[0].routes;
    for (const auto &route: routes) {
        string air_prev = route.nodes[0];
        for (int i = 1; i < route.nodes.size(); ++i) {
            string air_current = route.nodes[i];
            
            Point prev_point = Point(1, (int) air_prev[0] -65, stoi(air_prev.substr(1)));
            Point current_point = Point(1, (int) air_current[0] -65, stoi(air_current.substr(1)));
            
            int prev_index = getNetworks()->get_node_idx(prev_point);
            int current_index = getNetworks()->get_node_idx(current_point);
            
            is_arc_used_in_design[pair<int, int>(prev_index, current_index)] = true;
            
            air_prev = air_current;
        }
    }
    
    return is_arc_used_in_design;
}

const vector<pair<int, int>> &CargoRoute::getSea_arc_pairs() const {
    return sea_arc_pairs;
}

const vector<pair<int, int>> &CargoRoute::getAir_arc_pairs() const {
    return air_arc_pairs;
}

EntireNetwork* CargoRoute::getNetworks(){
    return networks;
}

Solution* CargoRoute::run_bp() {
    Solution* sol = branch_and_price();
    reset_bp();
    return sol;
}

void CargoRoute::rebuild_entire_network() {
    networks->rebuild_networks();
    
    auto& arcs = networks->getArcs();

    for (const auto &arc_column: arcs) {
        for (const auto &arc: arc_column.second) {
            if (arc.second->fixed_profits.size() == 0) {
                arc.second->fixed_profits = vector<double>(cargos.size(), 0.0);
            }
        }
    }
}

void CargoRoute::reset_bp() {
    for(const auto &path: all_paths){
        path->reduced_cost = 0;
        const Point *cur, *next;
        for(int p = 0; p < path->points().size()-1; p++){
            cur = &path->points()[p];
            next = &path->points()[p+1];
            
            Arc* arc = networks->getArc(*cur, *next);
            arc->fixed_profits = vector<double>(cargos.size(), 0.0);
            arc->fixed_cost = 0;
        }
    }
    
    incumbent = 0;
    integer_set.clear();
    all_paths.clear();
    
    delete[] rival_path;
    delete not_use_count;
    
    chosen_paths.clear();
    
    cons1.clear();
    cons2.clear();
    cons3.clear();
    cons4.clear();
    cons5.clear();
    cons6.clear();
    cons7.clear();
}

// Not running.
/*Solution* CargoRoute::Run_full_model() {
    path_categories = networks->getPaths_categories();
    get_available_path(path_categories, all_paths);
    arcs = networks->getArcs();
    for (const auto &arc_column: arcs) {
        for (const auto &arc: arc_column.second) {
            if (arc.second->fixed_profits.size() == 0) {
                arc.second->fixed_profits = vector<double>(cargos.size(), 0.0);
            }
        }
    }
    
    //cal_paths_profit();
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
            if(is_path_feasible_for_cargo(path, cargos[k])) {
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
        Solution *sol = new Solution(cargos.size(),
                                     target_path,
                                     rival_path,
                                     z_value,
                                     get_P_value(),
                                     get_r_column(),
                                     get_arc_usage_in_design(),
                                     networks->getSea_Air_Route());
        
        return sol;

    } catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }
    return 0;
}*/

const vector<Path *>& CargoRoute::find_all_paths() {
    // TODO: Check wheather this is a valid operation.
    all_paths.clear();
    
    path_categories = networks->getPaths_categories();
    get_available_path(path_categories, all_paths);
    
    return all_paths;
}

void CargoRoute::out_put_v_value(ostream& os) {
    
    // Print out v[k][p]
    os << "======v[k][p] BEGIN======" << endl;
    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_path[k].size(); p++){
            Path* path = target_path[k][p];
            
            os << "v[" << k << "][" << p << "]: " << v[k][*path] << endl;
            os << *path << endl;
        }
    }
    os << "======v[k][p] END======" << endl;
    os << endl;
    os << "======v_[k][n] BEGIN======" << endl;
    for(int k = 0; k < cargos.size(); k++){
        for(int n = 0; n < rival_path[k].size(); n++) {
            Path* path = rival_path[k][n];
            
            os << "v_[" << k << "][" << n << "]: " << v_[k][*path] << endl;
            os << *path << endl;
        }
    }
    os << "======v_[k][n] END======" << endl;
    os << endl;
}

void CargoRoute::out_put_v_value_with_path(ostream& os,
                                           vector<Path*> *target_path,
                                           vector<Path*> *rival_path) {
    // Print out v[k][p]
    os << "======v[k][p] BEGIN======" << endl;
    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_path[k].size(); p++){
            Path* path = target_path[k][p];
            
            os << "v[" << k << "][" << p << "]: " << v[k][*path] << endl;
            os << *path << endl;
        }
    }
    os << "======v[k][p] END======" << endl;
    os << endl;
    os << "======v_[k][n] BEGIN======" << endl;
    for(int k = 0; k < cargos.size(); k++){
        for(int n = 0; n < rival_path[k].size(); n++){
            Path* path = rival_path[k][n];
            
            os << "v_[" << k << "][" << n << "]: " << v_[k][*path] << endl;
            os << *rival_path[k][n] << endl;
        }
    }
    os << "======v_[k][n] END======" << endl;
    os << endl;
}
