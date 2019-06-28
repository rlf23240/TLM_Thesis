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
    networks = EntireNetwork(data);
    read_cargo_file(data);
    get_available_path();
//    for(auto cargo : cargos){
//        cout << *cargo ;
//    }
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

void CargoRoute::run_model() {
    try{
        GRBEnv env = GRBEnv();
        GRBModel model = GRBModel(env);



        unsigned int num_cargo = cargos.size();


    } catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }
}

void CargoRoute::get_available_path() {
    vector<Path*>** path_categories = networks.getPaths_categories();
    unordered_set<pair<char, char>, pair_hash> used_OD;

    for(auto cargo : cargos){
        pair<char, char> OD(cargo->departure, cargo->destination);

        if(used_OD.find(OD) != used_OD.end()){ //OD in the set
            vector<Path*>  od_available_path = path_categories[(int) cargo->departure - 65][(int) cargo->destination - 65];
//            cout << od_available_path.size() << endl;
            paths.insert(paths.end(), od_available_path.begin(), od_available_path.end()); //append available path to paths
        }
        used_OD.insert(OD);
    }
//    cout << paths.size() << endl;
}
