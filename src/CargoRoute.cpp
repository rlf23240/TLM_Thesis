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
    target_networks = EntireNetwork(data, true);
    target_path_categories = target_networks.getPaths_categories();
    get_available_path(target_path_categories, target_paths);


    rival_networks = EntireNetwork(data, false);
    rival_path_categories = rival_networks.getPaths_categories();
    get_available_path(rival_path_categories, rival_paths);

    combine_paths(target_paths, rival_paths);
    combine_path_categories(target_path_categories, rival_path_categories);
    find_cargo_available_paths();

    for(int i = 0; i < num_nodes; i++){
        for(int j = 0; j < num_nodes; j++) {
            cout << target_path_categories[i][j].size() << "\t";
        }
        cout << endl;
    }
    cout << endl;
    for(int i = 0; i < num_nodes; i++){
        for(int j = 0; j < num_nodes; j++) {
            cout << rival_path_categories[i][j].size() << "\t";
        }
        cout << endl;
    }

    run_model();
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

            cargo_type type;
            if(time_sensitivity == 'H' && product_value == 'H') {
                type = only_air;
            }else if(time_sensitivity == 'H' && product_value == 'L')
            {
                type = air_both;
            }else if(time_sensitivity == 'L' && product_value == 'H'){
                type = sea_both;
            }else{
                type = only_sea;
            }

            auto * new_cargo = new Cargo(departure, destination, starting_time, end_time, volume, weight, type);
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
}

void CargoRoute::combine_paths(vector<Path *> target_paths, vector<Path *> rival_paths) {
    all_paths.insert(all_paths.end(), target_paths.begin(), target_paths.end());
    all_paths.insert(all_paths.end(), rival_paths.begin(), rival_paths.end());
    for(int i = 0; i < all_paths.size(); i++){
        all_paths[i]->setIndex(i);
    }
}

void CargoRoute::combine_path_categories(vector<Path *> **target_path_categories, vector<Path *> **rival_path_categories) {
    num_nodes = target_networks.getNumNodes();
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



    } catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }
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

void CargoRoute::set_constr3(GRBModel &model) {

}

void CargoRoute::set_constr4(GRBModel &model) {

}

void CargoRoute::set_constr5(GRBModel &model) {

}

void CargoRoute::set_constr6(GRBModel &model) {

}

void CargoRoute::set_constr7(GRBModel &model) {

}




