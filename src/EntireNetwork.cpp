//
// Created by Ashee on 2019/6/15.
//

#include "EntireNetwork.h"

int seed;

EntireNetwork::EntireNetwork(string data) {
    seed = 0;
    data_str = data;
    read_param_data(data);
    read_unload_cost_data(data);
    read_unit_profit_data(data);
    read_unit_cost_data(data);
    cout << "===========AIR===========" << endl;
    air_network = AirNetwork("../Data/" + data + "_air", num_cur_flights, num_cur_ships);
    cout << "===========SEA===========" << endl;
    sea_network = SeaNetwork("../Data/" + data + "_sea", num_cur_ships, num_cur_ships);

    create_networks(data);
    find_all_paths();

    candidate_designed_flight_routes = air_network.find_all_routes();
    candidate_designed_ship_routes = sea_network.find_all_routes();

//    for(auto &route : candidate_designed_flight_routes){
//        cout << *route << endl;
//    }
//    cout << candidate_designed_flight_routes.size() << " " << candidate_designed_ship_routes.size() << endl;

//    for(int i = 0; i < num_nodes; i++){
//        for(int j = 0; j < num_nodes; j++) {
//            cout << paths_categories[i][j].size() << "\t";
//        }
//        cout << endl;
//    }

}

EntireNetwork::EntireNetwork() = default;

void EntireNetwork::rebuild_networks() {

    cout << air_network.getFlights()[0].routes[0];
    cout << sea_network.getShips()[0].route;

    create_networks(data_str);
    find_all_paths();

    for(int i = 0; i < num_nodes; i++){
        for(int j = 0; j < num_nodes; j++) {
            cout << paths_categories[i][j].size() << "\t";
        }
        cout << endl;
    }
}

void EntireNetwork::generate_new_routes() {
//    random_device rd;
//    unsigned seed = static_cast<unsigned int>(chrono::system_clock::now().time_since_epoch().count());
    mt19937 gen =  mt19937(seed++);
    uniform_int_distribution<int> dis(0, INT_MAX);

    int sea_route_idx = dis(gen) % candidate_designed_ship_routes.size();
    int air_route_idx = dis(gen) % candidate_designed_flight_routes.size();

    set_sea_air_route(*candidate_designed_ship_routes[sea_route_idx], *candidate_designed_flight_routes[air_route_idx]);
}

void EntireNetwork::set_sea_air_route(Route sea_route, Route air_route) {
    sea_network.set_designed_ship(sea_route);
    air_network.set_designed_flight(air_route);
}

void EntireNetwork::create_networks(string data) {
    if(is_desinged_route_added) {
        add_designed_ships(); //layer 0
        add_designed_flights(); //layer 1
    }
    add_current_ships(); //layer 3
    add_current_flights(); //layer layer4
    add_rival_ships(); //layer 5
    add_rival_flights(); //layer 6
    add_virtual_network(data); //layer 2
}

void EntireNetwork::read_param_data(string data) {
    fstream file;
    file.open("../Data/" + data + "_param.txt");
    string line;
    getline(file, line);
    istringstream iss(line);
    string token;

    getline(iss, token, '\t');
    num_nodes = stoi(token);
    getline(iss, token, '\t');
    num_cur_ships = stoi(token);
    getline(iss, token, '\t');
    num_cur_flights = stoi(token);
    getline(iss, token, '\t');

}

void EntireNetwork::read_unload_cost_data(string data) {
    fstream file;
    file.open("../Data/" + data + "_unload_cost.txt");

    string line;
    getline(file, line);


    while(getline(file, line)){
        unload_cost.push_back(stoi(line));
    }
}

void EntireNetwork::read_unit_profit_data(string data) {
    air_profit =  vector<vector<double>>{num_nodes};
    sea_profit =  vector<vector<double>>{num_nodes};

    fstream air_file,sea_file;
    air_file.open("../Data/" + data + "_air_profit.txt");

    if(air_file.is_open()) {
        string line;

        //read time cost
        for (int i = 0;i < num_nodes ; i++) { //row counter
            getline(air_file, line);
            istringstream iss(line);
            string token;

            for (int j = 0 ; j < num_nodes; j++) { //col counter
                getline(iss, token, '\t');
                air_profit[i].push_back(stod(token));
            }
        }
    }
    else {
        cout << "Can't read unit profit file !!!" << endl;
    }

    sea_file.open("../Data/" + data + "_sea_profit.txt");

    if(sea_file.is_open()) {
        string line;

        //read time cost
        for (int i = 0; i < num_nodes; i++) { //row counter
            getline(sea_file,line);
            istringstream iss(line);
            string token;

            for (int j = 0; j < num_nodes; j++) { //col counter
                getline(iss, token, '\t');
                sea_profit[i].push_back(stod(token));
            }
        }
    }
    else {
        cout << "Can't read unit profit file file !!!" << endl;
    }

//    for(int i = 0; i < num_nodes; i++){
//        for(int j = 0; j < num_nodes; j++){
//            cout << air_profit[i][j] << " ";
//        }
//        cout << endl;
//    }
}

void EntireNetwork::read_unit_cost_data(string data) {
    air_cost =  vector<vector<double>>{num_nodes};
    sea_cost =  vector<vector<double>>{num_nodes};

    fstream air_file,sea_file;
    air_file.open("../Data/" + data + "_air_trans_cost.txt");

    if(air_file.is_open()) {
        string line;

        //read time cost
        for (int i = 0;i < num_nodes ; i++) { //row counter
            getline(air_file, line);
            istringstream iss(line);
            string token;

            for (int j = 0 ; j < num_nodes; j++) { //col counter
                getline(iss, token, '\t');
                air_cost[i].push_back(stod(token));
            }
        }
    }
    else {
        cout << "Can't read unit cost file !!!" << endl;
        exit(1);
    }

    sea_file.open("../Data/" + data + "_sea_trans_cost.txt");

    if(sea_file.is_open()) {
        string line;

        //read time cost
        for (int i = 0; i < num_nodes; i++) { //row counter
            getline(sea_file,line);
            istringstream iss(line);
            string token;

            for (int j = 0; j < num_nodes; j++) { //col counter
                getline(iss, token, '\t');
                sea_cost[i].push_back(stod(token));
            }
        }
    }
    else {
        cout << "Can't read unit cost file !!!" << endl;
        exit(1);
    }

//    for(int i = 0; i < num_nodes; i++){
//        for(int j = 0; j < num_nodes; j++){
//            cout << air_cost[i][j] << " ";
//        }
//        cout << endl;
//    }
}

void EntireNetwork::add_designed_ships() {
    int layer = 0;
    nodes[layer] = vector<vector<Node*>>(num_nodes); //first time space network layer

    //add nodes
    int* node_cost = sea_network.getStop_cost();
    for(int i = 0; i < num_nodes; i++){
        nodes[layer][i] = vector<Node*>(TOTAL_TIME_SLOT);
        for(int t = 0 ; t < TOTAL_TIME_SLOT; t++){
            nodes[layer][i][t] = new Node(to_string(layer) + (char) (65 + i) + to_string(t), node_cost[i]);
        }
    }

    //add routes arc
    int** arc_cost = sea_network.getArc_cost();
    for(int w = 0; w < TOTAL_WEEK; w++) {
        for (const auto &ship : sea_network.getShips()) {
            Route route = ship.route;
            for (int i = 0; i < route.nodes.size() - 1; i++) {
                char start_node_char = route.nodes[i][0];
                int start_node_time = stoi(route.nodes[i].substr(1)) + w * 7 * TIME_SLOT_A_DAY;
                char end_node_char = route.nodes[i + 1][0];
                int end_node_time = stoi(route.nodes[i + 1].substr(1)) + w * 7 * TIME_SLOT_A_DAY;
                if(end_node_time >= TOTAL_TIME_SLOT) break;

                Node *start_node = nodes[0][(int) start_node_char - 65][start_node_time];
                Node *end_node = nodes[0][(int) end_node_char - 65][end_node_time];

                Arc *arc = new Arc(start_node, end_node, arc_cost[(int) start_node_char - 65][(int) end_node_char - 65],
                                   ship.volume_ub, sea_profit[(int) start_node_char - 65][(int) end_node_char - 65], sea_cost[(int) start_node_char - 65][(int) end_node_char - 65]);
                add_arc(start_node, end_node, arc);
            }
        }
    }
}

void EntireNetwork::add_designed_flights() {
    int layer = 1;
    nodes[layer] = vector<vector<Node*>>(num_nodes);

    //add nodes
    int* node_cost = air_network.getStop_cost();
    for(int i = 0; i < num_nodes; i++){
        nodes[layer][i] = vector<Node*>(TOTAL_TIME_SLOT);
        for(int t = 0 ; t < TOTAL_TIME_SLOT; t++){
            nodes[layer][i][t] = new Node(to_string(layer) + (char) (65 + i) + to_string(t), node_cost[i]);
        }
    }

    int** arc_cost = air_network.getArc_cost();

    for(const auto &flight : air_network.getFlights()){
        for(int week = 0 ; week < TIME_PERIOD / 7; week++) {
            for(const auto &route : flight.routes){
                for(int i = 0; i < route.nodes.size()-1; i++) {
                    char start_node_char = route.nodes[i][0];
                    int start_node_time = week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i].substr(1));
                    char end_node_char = route.nodes[i + 1][0];
                    int end_node_time = week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i + 1].substr(1));
                    if(end_node_time >= TOTAL_TIME_SLOT) continue;

                    Node *start_node = nodes[layer][(int) start_node_char - 65][start_node_time];
                    Node *end_node = nodes[layer][(int) end_node_char - 65][end_node_time];

                    Arc *arc = new Arc(start_node, end_node,
                                       arc_cost[(int) start_node_char - 65][(int) end_node_char - 65], flight.volume_ub,
                                       flight.weight_ub, air_profit[(int) start_node_char - 65][(int) end_node_char - 65], air_cost[(int) start_node_char - 65][(int) end_node_char - 65]);

                    add_arc(start_node, end_node, arc);
                }
            }
        }
    }

}

int* EntireNetwork::read_stop_cost(const string cost_data_path) {
    fstream file;
    file.open(cost_data_path);
    int *stop_cost = new int[num_nodes];

    if(file.is_open()){
        string line;
        getline(file, line);
        istringstream iss(line);
        string token;
        for(int i = 0; getline(iss, token, '\t'); i++) {
            stop_cost[i] = stoi(token);
        }
        return stop_cost;
    }
    else {
        cout << "stop_cost file cannot open !!!" << endl;
        return nullptr;
    }
}

void EntireNetwork::add_current_ships() {
    int layer = 3;
    nodes[layer] = vector<vector<Node*>>(num_nodes); //first time space network layer

    //add nodes
    int* node_cost = sea_network.getStop_cost();
    for(int i = 0; i < num_nodes; i++){
        nodes[layer][i] = vector<Node*>(TOTAL_TIME_SLOT);
        for(int t = 0 ; t < TOTAL_TIME_SLOT; t++){
            nodes[layer][i][t] = new Node(to_string(layer) + (char) (65 + i) + to_string(t), node_cost[i]);
        }
    }

    int** arc_cost = sea_network.getArc_cost();
    for(int w = 0; w < TOTAL_WEEK; w++) {
        for (const auto &ship : sea_network.getCur_ships()) {
            Route route = ship.route;
            for (int i = 0; i < route.nodes.size() - 1; i++) {
                char start_node_char = route.nodes[i][0];
                int start_node_time = stoi(route.nodes[i].substr(1)) + w * 7 * TIME_SLOT_A_DAY;
                char end_node_char = route.nodes[i + 1][0];
                int end_node_time = stoi(route.nodes[i + 1].substr(1)) + w * 7 * TIME_SLOT_A_DAY;
                if(end_node_time >= TOTAL_TIME_SLOT) break;

                Node *start_node = nodes[layer][(int) start_node_char - 65][start_node_time];
                Node *end_node = nodes[layer][(int) end_node_char - 65][end_node_time];

                Arc *arc = new Arc(start_node, end_node, arc_cost[(int) start_node_char - 65][(int) end_node_char - 65],
                                   ship.volume_ub, sea_profit[(int) start_node_char - 65][(int) end_node_char - 65], sea_cost[(int) start_node_char - 65][(int) end_node_char - 65]);
                add_arc(start_node, end_node, arc);
            }
        }
    }
}

void EntireNetwork::add_virtual_network(string data) {
    int layer = 2;
    nodes[layer] = vector<vector<Node*>>(num_nodes);

    //add nodes
    int* virtual_node_cost = read_stop_cost("../Data/" + data +"_virtual.txt");
    for(int i = 0; i < num_nodes; i++){
        nodes[layer][i] = vector<Node*>(TOTAL_TIME_SLOT);
        for(int t = 0 ; t < TOTAL_TIME_SLOT; t++){
            nodes[layer][i][t] = new Node(to_string(layer) + (char) (65 + i) + to_string(t), virtual_node_cost[i]);
        }
    }

    //add horizontal arcs
    for(int i = 0; i < num_nodes; i++){
        for(int t = 0; t < TOTAL_TIME_SLOT - 1; t++){
            Arc *arc = new Arc(nodes[layer][i][t], nodes[layer][i][t+1], FIX_COST_OF_VIRTUAL_ARC);
            add_arc(nodes[layer][i][t], nodes[layer][i][t+1], arc);
        }
    }
    Arc* arc;
    //add vertical arcs(only from ship to flight)
    for(int i = 0; i < num_nodes; i++){
        for(int t = 0; t < TOTAL_TIME_SLOT-1; t++) {
            //from design ship to virtual
            if(is_desinged_route_added) {
                if (!nodes[0][i][t]->out_arcs.empty()) {
                    //in arc
                    arc = new Arc(nodes[layer][i][t], nodes[0][i][t + 1], 0);
                    add_arc(nodes[layer][i][t], nodes[0][i][t + 1], arc);
                    //out arc
                    arc = new Arc(nodes[0][i][t], nodes[layer][i][t + 1], 0);
                    add_arc(nodes[0][i][t], nodes[layer][i][t + 1], arc);
                }
                //from virtual to design flight
                if (!nodes[1][i][t]->out_arcs.empty()) {
                    //in arc
                    arc = new Arc(nodes[layer][i][t], nodes[1][i][t], 0);
                    add_arc(nodes[layer][i][t], nodes[1][i][t], arc);
                }
            }

            //from current ship to virtual
            if (!nodes[3][i][t]->out_arcs.empty()) {
                //in arc
                arc = new Arc(nodes[layer][i][t], nodes[3][i][t+1],0);
                add_arc(nodes[layer][i][t], nodes[3][i][t+1], arc);
                //out arc
                arc = new Arc(nodes[3][i][t], nodes[layer][i][t+1],0);
                add_arc(nodes[3][i][t], nodes[layer][i][t+1], arc);
            }
            //from current flight to virtual
            if (!nodes[4][i][t]->out_arcs.empty()) {
                //in arc
                arc = new Arc(nodes[layer][i][t], nodes[4][i][t],0);
                add_arc(nodes[layer][i][t], nodes[4][i][t], arc);
            }
            //from rival ship to virtual

            if (!nodes[5][i][t]->out_arcs.empty()) {
                //in arc
                arc = new Arc(nodes[layer][i][t], nodes[5][i][t+1], 0);
                add_arc(nodes[layer][i][t], nodes[5][i][t+1], arc);
                //out arc
                arc = new Arc(nodes[5][i][t], nodes[layer][i][t+1], 0);
                add_arc(nodes[5][i][t], nodes[layer][i][t+1], arc);
            }
            //from rival flight to virtual
            if (!nodes[6][i][t]->out_arcs.empty()) {
                //in arc
                arc = new Arc(nodes[layer][i][t], nodes[6][i][t], 0);
                add_arc(nodes[layer][i][t], nodes[6][i][t], arc);
            }
        }
    }
}

void EntireNetwork::add_arc(Node *out, Node *in, Arc *arc) {
    int out_node_idx = get_node_idx(out->getLayer(), out->getNode(), out->getTime());
    int in_node_idx = get_node_idx(in->getLayer(), in->getNode(), in->getTime());
//    cout << out->getName() << " " << out_node_idx << " " << in->getName() << " " << in_node_idx << endl;
    if(arcs[out_node_idx][in_node_idx]){  // if two arcs are in the same place
        arcs[out_node_idx][in_node_idx]->weight_ub += arc->weight_ub;
        arcs[out_node_idx][in_node_idx]->volume_ub += arc->volume_ub;
    }else {
        arcs[out_node_idx][in_node_idx] = arc;
    }
    out->out_arcs.push_back(arc);
    in->in_arcs.push_back(arc);
}

void EntireNetwork::add_current_flights() {
    int layer = 4;
    nodes[layer] = vector<vector<Node*>>(num_nodes);

    //add nodes
    int* node_cost = air_network.getStop_cost();
    for(int i = 0; i < num_nodes; i++){
        nodes[layer][i] = vector<Node*>(TOTAL_TIME_SLOT);
        for(int t = 0 ; t < TOTAL_TIME_SLOT; t++){
            nodes[layer][i][t] = new Node(to_string(layer) + (char) (65 + i) + to_string(t), node_cost[i]);
        }
    }

    //add arcs
    int** arc_cost = air_network.getArc_cost();

    for(const auto &flight : air_network.getCur_flights()){
        for(int week = 0 ; week < TIME_PERIOD / 7; week++) {
            for(const auto &route : flight.routes){
                for(int i = 0; i < route.nodes.size()-1; i++) {
                    char start_node_char = route.nodes[i][0];
                    int start_node_time = week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i].substr(1));
                    char end_node_char = route.nodes[i + 1][0];
                    int end_node_time = week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i + 1].substr(1));

                    Node *start_node = nodes[layer][(int) start_node_char - 65][start_node_time];
                    Node *end_node = nodes[layer][(int) end_node_char - 65][end_node_time];

                    Arc *arc = new Arc(start_node, end_node,
                                       arc_cost[(int) start_node_char - 65][(int) end_node_char - 65], flight.volume_ub,
                                       flight.weight_ub, air_profit[(int) start_node_char - 65][(int) end_node_char - 65], air_cost[(int) start_node_char - 65][(int) end_node_char - 65]);

                    add_arc(start_node, end_node, arc);
                }
            }
        }
    }
}

void EntireNetwork::add_rival_ships() {
    int layer = 5;
    nodes[layer] = vector<vector<Node*>>(num_nodes); //first time space network layer

    //add nodes
    int* node_cost = sea_network.getStop_cost();
    for(int i = 0; i < num_nodes; i++){
        nodes[layer][i] = vector<Node*>(TOTAL_TIME_SLOT);
        for(int t = 0 ; t < TOTAL_TIME_SLOT; t++){
            nodes[layer][i][t] = new Node(to_string(layer) + (char) (65 + i) + to_string(t), node_cost[i]);
        }
    }

    int** arc_cost = sea_network.getArc_cost();
    for(int w = 0; w < TOTAL_WEEK; w++) {
        for (const auto &ship : sea_network.getRival_ships()) {
            Route route = ship.route;
            for (int i = 0; i < route.nodes.size() - 1; i++) {
                char start_node_char = route.nodes[i][0];
                int start_node_time = stoi(route.nodes[i].substr(1)) + w * 7 * TIME_SLOT_A_DAY;
                char end_node_char = route.nodes[i + 1][0];
                int end_node_time = stoi(route.nodes[i + 1].substr(1))+ w * 7 * TIME_SLOT_A_DAY;
                if(end_node_time >= TOTAL_TIME_SLOT) break;

                Node *start_node = nodes[layer][(int) start_node_char - 65][start_node_time];
                Node *end_node = nodes[layer][(int) end_node_char - 65][end_node_time];

                Arc *arc = new Arc(start_node, end_node, arc_cost[(int) start_node_char - 65][(int) end_node_char - 65],
                                   ship.volume_ub, sea_profit[(int) start_node_char - 65][(int) end_node_char - 65], sea_cost[(int) start_node_char - 65][(int) end_node_char - 65]);

                add_arc(start_node, end_node, arc);
            }
        }
    }

}

void EntireNetwork::add_rival_flights() {
    int layer = 6;
    nodes[layer] = vector<vector<Node*>>(num_nodes);

    //add nodes
    int* node_cost = air_network.getStop_cost();
    for(int i = 0; i < num_nodes; i++){
        nodes[layer][i] = vector<Node*>(TOTAL_TIME_SLOT);
        for(int t = 0 ; t < TOTAL_TIME_SLOT; t++){
            nodes[layer][i][t] = new Node(to_string(layer) + (char) (65 + i) + to_string(t), node_cost[i]);
        }
    }

    //add arcs
    int** arc_cost = air_network.getArc_cost();

    for(const auto &flight : air_network.getRival_flights()){
        for(int week = 0 ; week < TIME_PERIOD / 7; week++) {
            for(const auto &route : flight.routes){
                for(int i = 0; i < route.nodes.size()-1; i++) {
                    char start_node_char = route.nodes[i][0];
                    int start_node_time = week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i].substr(1));
                    char end_node_char = route.nodes[i + 1][0];
                    int end_node_time = week * 7 * TIME_SLOT_A_DAY + stoi(route.nodes[i + 1].substr(1));

                    Node *start_node = nodes[layer][(int) start_node_char - 65][start_node_time];
                    Node *end_node = nodes[layer][(int) end_node_char - 65][end_node_time];

                    Arc *arc = new Arc(start_node, end_node,
                                       arc_cost[(int) start_node_char - 65][(int) end_node_char - 65], flight.volume_ub,
                                       flight.weight_ub, air_profit[(int) start_node_char - 65][(int) end_node_char - 65], air_cost[(int) start_node_char - 65][(int) end_node_char - 65]);

                    add_arc(start_node, end_node, arc);
                }
            }
        }
    }
}

void EntireNetwork::find_all_paths() {
    paths_categories = new vector<Path*>*[num_nodes];
    for(int i = 0; i < num_nodes; i++)
        paths_categories[i] = new vector<Path*>[num_nodes];

    for(int l = 0; l < 1; l++) {
        for (int n = 0; n < num_nodes; n++) {
            for (int t = 0; t < TOTAL_TIME_SLOT; t++) {
                Point point = Point{2, n, t};
                int ***visited = create_3d_array(num_layers, num_nodes, TOTAL_TIME_SLOT);
                Path path = Path(point);
                find_paths_from_single_node(path, point, visited);
            }
        }
    }
}

void EntireNetwork::find_paths_from_single_node(Path path, Point point, int*** visited) {

    Node* cur_node = nodes[point.layer][point.node][point.time];
    visited[point.layer][point.node][point.time] = 1;

    add_path(new Path(path));
    for(auto* out_arc : cur_node->out_arcs){
        auto next_point = Point(out_arc->end_node->getName());
        path.push_point(next_point);
        if (path.is_feasible() && visited[next_point.layer][next_point.node][next_point.time] == 0)
            find_paths_from_single_node(path, next_point, visited);
        path.pop_point();
    }
}

void EntireNetwork::add_path(Path *path) {
    Point front = path->points.front();
    Point back = path->points.back();

    if(front.node == back.node || back.layer == 2)
        return;

//    cout << front.node << back.node << endl;

    if(check_path_feasibility(path)) {
        all_paths.push_back(path);
        paths_categories[front.node][back.node].push_back(path);
    }
}

bool EntireNetwork::check_path_feasibility(Path *path) {
    Point front = path->points.front();
    Point back = path->points.back();
    if(front.node == back.node || back.layer == 2)
        return false;

    unordered_set<int> visited_nodes;

    int previous_node = -1;
    int previous_virtual_node = -1;
    int previous_layer = -1;

    if(path->points.back().node == (path->points.end()-2)->node)
        return false;

    if((path->points[1].layer == 0 || path->points[1].layer == 3 || path->points[1].layer == 5) && (path->points.back().layer == 0 || path->points.back().layer == 3 || path->points.back().layer == 5)){
        path->type = onlySea;
    }
    else if((path->points[1].layer == 1 || path->points[1].layer == 4 || path->points[1].layer == 6) && (path->points.back().layer == 1 || path->points.back().layer == 4 || path->points.back().layer == 6)) {
        path->type = onlyAir;
    }
    else{
        path->type = seaAir;
    }
    
    if(path->points[1].layer != path->points.back().layer && (path->points.back().layer == 0 || path->points.back().layer == 3 || path->points.back().layer == 5))
        return false;


    for(const auto& point : path->points){
        // check if this node is visited
        if(visited_nodes.find(point.node) == visited_nodes.end()) {
            visited_nodes.insert(point.node);
        }else if(previous_node != point.node){
            return false;
        }

        //check if the cargo is moved to another node
        if(point.layer == 2){
            if(previous_virtual_node == point.node && previous_layer != 2){
                return false;
            }else{
                previous_virtual_node = point.node;
            }
        }

        previous_node = point.node;
        previous_layer = point.layer;
    }

    return true;
}

int ***EntireNetwork::create_3d_array(int x, int y, int z) {
    int ***arr = new int**[x];
    for (int i = 0; i < x; ++i) {
        arr[i] = new int*[y];
        for (int j = 0; j < y; ++j)
            arr[i][j] = new int[z];
    }
    for(int i = 0; i < x; i++){
        for(int j = 0; j < y ; j++){
            for(int k = 0; k < z; k++){
                arr[i][j][k] = 0;
            }
        }
    }
    return arr;
}

vector<Path *> **EntireNetwork::getPaths_categories() const {
    return paths_categories;
}

unsigned int EntireNetwork::getNumNodes() const {
    return num_nodes;
}

int EntireNetwork::get_node_idx(int layer, int node, int time) {
    return layer *(num_nodes * TOTAL_TIME_SLOT) + node * TOTAL_TIME_SLOT + time;
}

int EntireNetwork::get_node_idx(Point point) {
    return point.layer * (num_nodes * TOTAL_TIME_SLOT) + point.node * TOTAL_TIME_SLOT + point.time;;
}

Point EntireNetwork::idx_to_point(int idx) {
    int time  = idx % TOTAL_TIME_SLOT;
    int node = (idx - time) % (TOTAL_TIME_SLOT * num_nodes) / TOTAL_TIME_SLOT;
    int layer = (idx - time - node * TOTAL_TIME_SLOT) / (TOTAL_TIME_SLOT * num_nodes);
    return Point(layer, node, time);
}

const unordered_map<int, unordered_map<int, Arc *>> &EntireNetwork::getArcs() const {
    return arcs;
}

vector<Flight> EntireNetwork::get_cur_flights() {
    return this->air_network.getCur_flights();
}

vector<Ship> EntireNetwork::get_cur_ships() {
    return this->sea_network.getCur_ships();
}

AirNetwork &EntireNetwork::getAir_network() {
    return air_network;
}

SeaNetwork &EntireNetwork::getSea_network(){
    return sea_network;
}

vector<Route> EntireNetwork::getSea_Air_Route() {
    vector<Route> sea_air_routes;
    sea_air_routes.push_back(sea_network.getDesignedShips()[0].route);
    sea_air_routes.insert(sea_air_routes.end(), air_network.getDesignedFlights()[0].routes.begin(), air_network.getDesignedFlights()[0].routes.end());

    return sea_air_routes;
}

void EntireNetwork::setAir_network(const AirNetwork &air_network) {
    EntireNetwork::air_network = air_network;
}

void EntireNetwork::setSea_network(const SeaNetwork &sea_network) {
    EntireNetwork::sea_network = sea_network;
}







