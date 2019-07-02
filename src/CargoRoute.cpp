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

    cout  << all_paths.size() << endl;

    cal_paths_profit(all_paths);

//    combine_path_categories(target_path_categories, rival_path_categories);
//    find_cargo_available_paths();
//
//    cout << endl;
//    for(int i = 0; i < num_nodes; i++){
//        for(int j = 0; j < num_nodes; j++) {
//            cout << rival_path_categories[i][j].size() << "\t";
//        }
//        cout << endl;
//    }
//
//    run_model();
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

void CargoRoute::cal_paths_profit(vector<Path*> all_paths){
    for(auto &path : all_paths){
        cal_profit(path);

    }
}
void CargoRoute::cal_profit(Path* path){
    for(int p = 0; p < path->points.size()-1; p++){
        Point cur_node = path->points[p];
        Point next_node = path->points[p+1];
    }

}
void CargoRoute::combine_path_categories(vector<Path *> **target_path_categories, vector<Path *> **rival_path_categories) {
    num_nodes = networks.getNumNodes();
    all_path_categories = new vector<Path*>*[num_nodes];
    for(int i = 0; i < num_nodes; i++)
        all_path_categories[i] = new vector<Path*>[num_nodes];

    for(int i = 0; i < num_nodes; i++){
        for(int j = 0; j < num_nodes; j++){
            all_path_categories[i][j].insert(all_path_categories[i][j].end(), target_path_categories[i][j].begin(), target_path_categories[i][j].end());
            all_path_categories[i][j].insert(all_path_categories[i][j].end(), rival_path_categories[i][j].begin(), rival_path_categories[i][j].end());
        }
    }
}

void CargoRoute::find_cargo_available_paths() {
    cargo_available_paths = new vector<Path*>[cargos.size()];
    target_cargo_available_paths = new vector<Path*>[cargos.size()];
    rival_cargo_available_paths = new vector<Path*>[cargos.size()];
    int cargo_count = 0;
    for(const auto &cargo : cargos){
        int departure_node = (int) cargo->departure - 65;
        int destination_node = (int) cargo->destination - 65;
        int start_time = cargo->start_time;
        int arrive_time = cargo->arrive_time;

        for(const auto &path : all_path_categories[departure_node][destination_node]){
            int path_start_time = path->points.front().time;
            int path_end_time = path->points.back().time;

            if(start_time <= path_start_time && arrive_time >= path_end_time){
                if(path->index < target_paths.size()) {
                    target_cargo_available_paths[cargo_count].push_back(path);
                }
                else{
                    rival_cargo_available_paths[cargo_count].push_back(path);
                }
                cargo_available_paths[cargo_count].push_back(path);
            }

        }
        cargo_count++;
    }

//    for(const auto &path : all_path_categories[0][3]){
//        cout << *path;
//    }
//
//    for(int i = 0; i< cargos.size(); i++){
//        cout << cargo_available_paths[i].size() << endl;
//    }
}

void CargoRoute::run_model() {
    try{
        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);

        unsigned int num_cargo = cargos.size();
        unsigned int num_target_paths = target_paths.size();
        unsigned int num_rival_paths = rival_paths.size();

        cout << "Number of Cargo : " << num_cargo << endl;
        cout << "Number of target paths : " << num_target_paths << endl;
        cout << "Number of rival paths : " << num_rival_paths << endl;

        cal_target_v();
        cal_rival_v();
        cal_e();

        /*Set variables*/
        //set u
        GRBVar** u = new GRBVar*[cargos.size()];
        for(int i = 0; i < cargos.size(); i++){
            u[i] = model.addVars(cargo_available_paths[i].size(), GRB_BINARY);
        }
        //set target z
        GRBVar** z = new GRBVar*[cargos.size()];
        for(int i = 0; i < cargos.size(); i++){
            z[i] = model.addVars(target_cargo_available_paths[i].size(), GRB_CONTINUOUS);
        }
        //set rival z
        GRBVar** z_ = new GRBVar*[cargos.size()];
        for(int i = 0; i < cargos.size(); i++){
            z_[i] = model.addVars(rival_cargo_available_paths[i].size(), GRB_CONTINUOUS);
        }

        set_constr1(model, z, z_);
        set_constr2(model, z, u);
        set_constr3(model, z, z_, u);
        set_constr4(model, z, z_, u);



    } catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }
}

void CargoRoute::cal_target_v() {
    v = new double*[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        v[k] = new double[target_cargo_available_paths[k].size()];
    }

    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_cargo_available_paths[k].size(); p++){
            cal_path_cost(networks, target_cargo_available_paths[k][p]);
            v[k][p] = cargos[k]->alpha * target_cargo_available_paths[k][p]->cost +
                      cargos[k]->beta * target_cargo_available_paths[k][p]->last_time;
//            cout << v[k][p] << " " << cargos[k]->alpha << " " << cargos[k]->beta << endl;
        }
    }

}

void CargoRoute::cal_rival_v() {
    v_ = new double*[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        v_[k] = new double[rival_cargo_available_paths[k].size()];
    }

    for(int k = 0; k < cargos.size(); k++){
        for(int n = 0; n < rival_cargo_available_paths[k].size(); n++){
            cal_path_cost(networks, rival_cargo_available_paths[k][n]);
            v_[k][n] = cargos[k]->alpha * rival_cargo_available_paths[k][n]->cost +
                      cargos[k]->beta * rival_cargo_available_paths[k][n]->last_time;
        }
    }
}

void CargoRoute::cal_path_cost(EntireNetwork& network, Path *path) {

    Node* pre_node = nullptr;
    Node* cur_node;
    int cost = 0;
    for(const auto &point : path->points){
        cur_node = network.getNode(point.layer, point.node, point.time);
        cost += cur_node->getCost();
        for(const auto &arc : cur_node->in_arcs){
            if(arc->start_node == pre_node){
//                cout << arc->cost << endl;
                cost += arc->cost;
            }
        }
        pre_node = cur_node;
    }
    path->cost = cost;
    path->last_time = path->points.back().time - path->points.front().time;
//    cout << *path;
//    cout << path->cost << " " << path->last_time << endl;
}

void CargoRoute::cal_e() {
    e = new double*[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        e[k] = new double[target_cargo_available_paths[k].size()];
    }

    e_ = new double*[cargos.size()];
    for(int k = 0; k < cargos.size(); k++){
        e_[k] = new double[rival_cargo_available_paths[k].size()];
    }

    double denominator_sum = 0;
    for(int k = 0; k < cargos.size(); k++){
        for(int n = 0; n < rival_cargo_available_paths[k].size(); n++ ){
            denominator_sum += exp(v_[k][n]);
        }
    }
    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_cargo_available_paths[k].size(); p++ ){
            denominator_sum += exp(v[k][p]);
        }
    }

    for(int k = 0; k < cargos.size(); k++){
        for(int n = 0; n < rival_cargo_available_paths[k].size(); n++ ){
            e_[k][n] = exp(v_[k][n]) / denominator_sum;
            cout << e_[k][n] << " ";
        }
        cout << endl;
    }

    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_cargo_available_paths[k].size(); p++ ){
            e[k][p] = exp(v[k][p]) / denominator_sum;
        }

    }
}

void CargoRoute::set_obj(GRBModel &model, GRBVar **z) {

}

void CargoRoute::set_constr1(GRBModel &model, GRBVar** z, GRBVar** z_) {
    for(int k = 0; k < cargos.size(); k++){
        GRBLinExpr z_sum = 0;
        for(int n = 0; n < rival_cargo_available_paths[k].size(); n++){
            z_sum += z_[k][n];
        }
        for(int p = 0; p < target_cargo_available_paths[k].size(); p++){
            z_sum += z[k][p];
        }
        model.addConstr(z_sum <= 1, "cons1");
    }
}

void CargoRoute::set_constr2(GRBModel& model, GRBVar** z, GRBVar** u) {
    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_cargo_available_paths[k].size(); p++){
            model.addConstr(z[k][p] <= u[k][p]);
        }
    }
}

void CargoRoute::set_constr3(GRBModel& model, GRBVar** z, GRBVar** z_, GRBVar** u) {
    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_cargo_available_paths[k].size(); p++){
            GRBLinExpr left = 0;
            GRBLinExpr right = 0;
            for(int n = 0; n < rival_cargo_available_paths[k].size(); n++){
                left += e_[k][n] * z[k][p];
                right += z_[k][n];
            }
            model.addConstr(left <=  e[k][p] * right);
        }
    }
}

void CargoRoute::set_constr4(GRBModel &model, GRBVar** z, GRBVar** z_, GRBVar** u) {
    for(int k = 0; k < cargos.size(); k++){
        for(int p = 0; p < target_cargo_available_paths[k].size(); p++){
            GRBLinExpr left = 0;
            GRBLinExpr right = 0;
            for(int n = 0; n < rival_cargo_available_paths[k].size(); n++){
                left += e_[k][n] * z[k][p];
                right += z_[k][n];
            }
            model.addConstr(left >=  e[k][p] * right + u[k][p] - 1);
        }
    }
}

void CargoRoute::set_constr5(GRBModel &model) {

}

void CargoRoute::set_constr6(GRBModel &model) {

}

void CargoRoute::set_constr7(GRBModel &model) {

}










