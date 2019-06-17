//
// Created by Ashee on 2019/6/15.
//

#include "EntireNetwork.h"

EntireNetwork::EntireNetwork(string data) {
    cout << "===========AIR===========" << endl;
    air_network = AirNetwork("../Data/" + data + "_air");
    cout << "===========SEA===========" << endl;
    sea_network = SeaNetwork("../Data/" + data + "_sea");
    read_data(data);

//    for(int l = 0; l < 5; l++){
//        for(int n = 0; n < num_nodes; n++){
//            for(int t = 0; t < TOTAL_TIME_SLOT; t++){
//                cout <<l<<" "<<n<<" "<<t <<":" << nodes[l][n][t]->out_arcs.size()<<endl;
//            }
//        }
//    }
//
    find_all_paths();
//    print_all_arcs();
}

void EntireNetwork::read_data(string data) {
    // 5 layers of time space network
    add_designed_ships(data);
    add_designed_flights(data);
    add_current_ships(data);
    add_current_flights(data);
    add_virtual_network(data);

}

void EntireNetwork::add_designed_ships(string data) {
    num_nodes = sea_network.getNum_nodes();
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

            Arc* arc = new Arc(start_node, end_node, arc_cost[(int) end_node_char -65][(int) end_node_char -65], ship.weight_ub);

            arcs.push_back(arc);
            start_node->out_arcs.push_back(arc);
            end_node->in_arcs.push_back(arc);
        }
    }


}

void EntireNetwork::add_designed_flights(string data) {
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
                                       arc_cost[(int) end_node_char - 65][(int) end_node_char - 65], flight.weight_ub,
                                       flight.volume_ub);

                    arcs.push_back(arc);
                    start_node->out_arcs.push_back(arc);
                    end_node->in_arcs.push_back(arc);
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

void EntireNetwork::add_current_ships(string data) {
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

            Arc* arc = new Arc(start_node, end_node, arc_cost[(int) end_node_char -65][(int) end_node_char -65], ship.weight_ub);

            arcs.push_back(arc);
            start_node->out_arcs.push_back(arc);
            end_node->in_arcs.push_back(arc);
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

            arcs.push_back(arc);
            nodes[layer][i][t]->out_arcs.push_back(arc);
            nodes[layer][i][t+1]->in_arcs.push_back(arc);
        }
    }
    Arc* arc;
    //add vertical arcs(only from ship to flight)
    for(int i = 0; i < num_nodes; i++){
        for(int t = 0; t < TOTAL_TIME_SLOT-1; t++) {
            //from design ship to virtual
            if (!nodes[0][i][t]->out_arcs.empty()) {
                //out arc
                add_arc(nodes[0][i][t], nodes[layer][i][t+1], 0);
            }
            //from virtual to design flight
            if (!nodes[1][i][t]->out_arcs.empty()) {
                //in arc
                add_arc(nodes[layer][i][t], nodes[1][i][t], 0);
            }
            //from current ship to virtual
            if (!nodes[3][i][t]->out_arcs.empty()) {
                //out arc
                add_arc(nodes[3][i][t], nodes[layer][i][t+1], 0);
            }
            //from current flight to virtual
            if (!nodes[4][i][t]->out_arcs.empty()) {
                //in arc
                add_arc(nodes[layer][i][t], nodes[4][i][t], 0);
            }
        }
    }
}

void EntireNetwork::add_arc(Node *out, Node *in, int cost) {
    Arc* arc = new Arc(out, in, cost);
    arcs.push_back(arc);
    out->out_arcs.push_back(arc);
    in->in_arcs.push_back(arc);
}

void EntireNetwork::add_current_flights(string data) {
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
                                       arc_cost[(int) end_node_char - 65][(int) end_node_char - 65], flight.weight_ub,
                                       flight.volume_ub);

                    arcs.push_back(arc);
                    start_node->out_arcs.push_back(arc);
                    end_node->in_arcs.push_back(arc);
                }
            }
        }

    }
}

void EntireNetwork::find_all_paths() {
    Point point = Point{3,5,0};
    int*** color = create_3d_array(5, num_nodes, 84);
    Path path = Path();
    path.push_point(point);
    find_paths_from_single_node(path, point, color);
}


void EntireNetwork::find_paths_from_single_node(Path path, Point point, int*** color) {

    Node* cur_node = nodes[point.layer][point.node][point.time];
    color[point.layer][point.node][point.time] = 1;
    if(path.is_feasible()){
        cout << path;
        for(auto* out_arc : cur_node->out_arcs){
            auto next_point = Point(out_arc->end_node->getName());
            path.push_point(next_point);
            if (color[next_point.layer][next_point.node][next_point.time] == 0)
                find_paths_from_single_node(path, next_point, color);
            path.pop_point();
        }
    }
    else{
        return ;
    }
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

void EntireNetwork::print_all_arcs() {
    for(auto* arc : arcs){
        cout << arc->start_node->getName() << "->" << arc->end_node->getName() << endl;
    }
}




