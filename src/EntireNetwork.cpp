//
// Created by Ashee on 2019/6/15.
//

#include "EntireNetwork.h"

EntireNetwork::EntireNetwork(string data) {
    data_str = data;
    read_param_data(data);
    cout << "===========AIR===========" << endl;
    air_network = AirNetwork("../Data/" + data + "_air", num_cur_flights, num_cur_ships);
    cout << "===========SEA===========" << endl;
    sea_network = SeaNetwork("../Data/" + data + "_sea", num_cur_ships, num_cur_ships);

    create_networks(data);
    find_all_paths();

    for(int i = 0; i < num_nodes; i++){
        for(int j = 0; j < num_nodes; j++) {
            cout << paths_categories[i][j].size() << "\t";
        }
        cout << endl;
    }

//
//    cout << sum << " " << all_paths.size() << endl;
//    for(auto path : all_paths){
//        cout << *path ;
//    }
}

EntireNetwork::EntireNetwork() = default;

void EntireNetwork::rebuild_networks() {

//    air_network.generate_designed_flight();
//    sea_network.generate_designed_ship();
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

void EntireNetwork::create_networks(string data) {

    add_designed_ships(); //layer 0
    add_designed_flights(); //layer 1
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
    for(const auto &ship : sea_network.getShips()) {
        Route route = ship.route;
        for(int i = 0 ; i < route.nodes.size() -1; i++){
            char start_node_char = route.nodes[i][0];
            int start_node_time = stoi(route.nodes[i].substr(1));
            char end_node_char = route.nodes[i+1][0];
            int end_node_time = stoi(route.nodes[i+1].substr(1));

            Node* start_node = nodes[0][(int) start_node_char -65][start_node_time];
            Node* end_node = nodes[0][(int) end_node_char -65][end_node_time];

            Arc* arc = new Arc(start_node, end_node, arc_cost[(int) start_node_char -65][(int) end_node_char -65], ship.volume_ub);
            add_arc(start_node, end_node, arc);
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

                    Node *start_node = nodes[layer][(int) start_node_char - 65][start_node_time];
                    Node *end_node = nodes[layer][(int) end_node_char - 65][end_node_time];

                    Arc *arc = new Arc(start_node, end_node,
                                       arc_cost[(int) start_node_char - 65][(int) end_node_char - 65], flight.volume_ub,
                                       flight.weight_ub);

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
    for(const auto &ship : sea_network.getCur_ships()) {
        Route route = ship.route;
        for(int i = 0 ; i < route.nodes.size() -1; i++){
            char start_node_char = route.nodes[i][0];
            int start_node_time = stoi(route.nodes[i].substr(1));
            char end_node_char = route.nodes[i+1][0];
            int end_node_time = stoi(route.nodes[i+1].substr(1));

            Node* start_node = nodes[layer][(int) start_node_char -65][start_node_time];
            Node* end_node = nodes[layer][(int) end_node_char -65][end_node_time];

            Arc* arc = new Arc(start_node, end_node, arc_cost[(int) start_node_char -65][(int) end_node_char -65], ship.volume_ub);
            add_arc(start_node, end_node, arc);
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
            if (!nodes[0][i][t]->out_arcs.empty()) {
                //in arc
                arc = new Arc(nodes[layer][i][t], nodes[0][i][t], 0);
                add_arc(nodes[layer][i][t], nodes[0][i][t], arc);
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

            //from current ship to virtual
            if (!nodes[3][i][t]->out_arcs.empty()) {
                //in arc
                arc = new Arc(nodes[layer][i][t], nodes[3][i][t], 0);
                add_arc(nodes[layer][i][t], nodes[3][i][t], arc);
                //out arc
                arc = new Arc(nodes[3][i][t], nodes[layer][i][t+1], 0);
                add_arc(nodes[3][i][t], nodes[layer][i][t+1], arc);
            }
            //from current flight to virtual
            if (!nodes[4][i][t]->out_arcs.empty()) {
                //in arc
                arc = new Arc(nodes[layer][i][t], nodes[4][i][t], 0);
                add_arc(nodes[layer][i][t], nodes[4][i][t], arc);
            }

            if (!nodes[5][i][t]->out_arcs.empty()) {
                //in arc
                arc = new Arc(nodes[layer][i][t], nodes[5][i][t], 0);
                add_arc(nodes[layer][i][t], nodes[5][i][t], arc);
                //out arc
                arc = new Arc(nodes[5][i][t], nodes[layer][i][t+1], 0);
                add_arc(nodes[5][i][t], nodes[layer][i][t+1], arc);
            }
            //from current flight to virtual
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
                                       flight.weight_ub);

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
    for(const auto &ship : sea_network.getRival_ships()) {
        Route route = ship.route;
        for(int i = 0 ; i < route.nodes.size() -1; i++){
            char start_node_char = route.nodes[i][0];
            int start_node_time = stoi(route.nodes[i].substr(1));
            char end_node_char = route.nodes[i+1][0];
            int end_node_time = stoi(route.nodes[i+1].substr(1));

            Node* start_node = nodes[layer][(int) start_node_char -65][start_node_time];
            Node* end_node = nodes[layer][(int) end_node_char -65][end_node_time];

            Arc* arc = new Arc(start_node, end_node, arc_cost[(int) start_node_char -65][(int) end_node_char -65], ship.volume_ub);

            add_arc(start_node, end_node, arc);
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
                                       flight.weight_ub);

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

    all_paths.push_back(path);
    paths_categories[front.node][back.node].push_back(path);
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

Node *EntireNetwork::getNode(int layer, int node, int time) {
    return nodes[layer][node][time];
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

const AirNetwork &EntireNetwork::getAir_network() const {
    return air_network;
}

const SeaNetwork &EntireNetwork::getSea_network() const {
    return sea_network;
}




