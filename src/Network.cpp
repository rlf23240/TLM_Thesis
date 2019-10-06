//
// Created by Ashee on 2019/5/22.
//

#include "Network.h"
#include "Node.h"
#include "param.h"

using namespace std;




/*-----------------Route struct------------------------------*/
Route::Route(const vector<string> &nodes, double cost) : nodes(nodes), cost(cost) {}

Route::Route() = default;

ostream &operator<<(ostream &os, const Route &route) {

    if(!route.nodes.empty()) {
        os << "Route : ";
        for (const string &node : route.nodes) {
            os << node << "->";
        }
        os << "<\t\tcost : ";
        os << route.cost << endl;
    }
    else {
        os << "Empty route" << endl;
    }

    return os;
}

Route::Route(Route route, int gap) {
    for(const string &node : route.nodes){
        char n = node[0];
        int time = stoi(node.substr(1)) + gap;

        this->nodes.push_back(n + to_string(time));
        this->cost = route.cost;
    }

}

/*-----------------------Network class------------------------*/
void Network::read_data(std::string data_path) {
    read_node(data_path + "_arccost.txt");
    read_stop_cost(data_path + "_stopcost.txt");
    read_time_cost(data_path + "_timecost.txt");
    add_nodes();
    add_edges();
}

static int excel_alpha_to_num(string str){
	if(str.size() <= 1)
		return (int) str[0] - 65;
	else{
		return ((int) str[0] - 65) * 26 + ((int) str[1] - 65);
	}
}
void Network::read_node(std::string node_data_path) {

    fstream file;
    file.open(node_data_path);

    if(file.is_open()) {
        string line;
        getline(file,line);
        num_nodes = stoi(line);
        arc_cost = new int*[num_nodes];
        for(int i = 0 ; i < num_nodes ; i++)
            arc_cost[i] = new int[num_nodes];
        //read arc cost
        for (int i = 0; getline(file, line); i++) { //row counter

            istringstream iss(line);
            string token;
            for (int j = 0 ;getline(iss, token, '\t') ; j++) { //col counter
                if( token[0] != 'M') {
                    arc_cost[i][j] = stoi(token);
                }
                else{
                    arc_cost[i][j] = INT_MAX;
                }
            }
        }
    }
    else {
        cout << "Can't read node file !!!" << endl;
    }
}

void Network::read_stop_cost(std::string cost_data_path) {
    fstream file;
    file.open(cost_data_path);
    stop_cost = new int[num_nodes];

    if(file.is_open()){
        string line;
        getline(file, line);
        istringstream iss(line);
        string token;
        for(int i = 0; getline(iss, token, '\t'); i++) {
            stop_cost[i] = stoi(token);
        }
    }
    else {
        cout << "stop_cost file cannot open !!!" << endl;
    }
}

void Network::read_time_cost(std::string time_data_path) {
    fstream file;
    file.open(time_data_path);

    if(file.is_open()) {
        string line;
        getline(file,line);
        time_cost = new int*[num_nodes];
        for(int i = 0 ; i < num_nodes ; i++)
            time_cost[i] = new int[num_nodes];

        //read time cost
        for (int i = 0; getline(file, line); i++) { //row counter
            istringstream iss(line);
            string token;

            for (int j = 0 ;getline(iss, token, '\t'); j++) { //col counter
//                cout << token;
                if( token[0] != 'M') {
                    time_cost[i][j] = stoi(token);
                }
                else{
                    time_cost[i][j] = INT_MAX;
                }
            }
        }
    }
    else {
        cout << "Can't read node file !!!" << endl;
    }

}

void Network::add_nodes() {
    // add time space network nodes
    for(int i = 0; i < num_nodes; i++){
        char letter = (char) (65+i);
        nodes[letter] = vector<Node*>();

        for(int node = 0; node < TOTAL_TIME_SLOT; node++){
            string name = letter+to_string(node); //node name
            Node* newNode = new Node(name, stop_cost[i]);
            nodes[letter].push_back(newNode);
        }
    }
}

bool Network::add_edge(Node* start, Node* end, int cost) {
    Arc* new_arc = new Arc(start, end, cost);
    start->out_arcs.push_back(new_arc);
    end->in_arcs.push_back(new_arc);

    return true;
}

void Network::add_edges() {

    for(int t = 0; t < TOTAL_TIME_SLOT; t++){
        for(int i = 0; i < num_nodes; i++){
            for(int out = 0; out < num_nodes; out++){
                if(arc_cost[i][out] < INT_MAX && t+time_cost[i][out] < TOTAL_TIME_SLOT){
//                    cout << (char) (65+i) <<t<< " To " << (char) (65+out)<<t+time_cost[i][out]
//                    << "  Arc Cost :" <<arc_cost[i][out]<<endl;
                    add_edge(nodes[(char) 'A'+i][t], nodes[(char) 'A'+out][t + time_cost[i][out]], arc_cost[i][out]);
                            // start                  end                                            cost
                }
            }
        }
    }
}

Route Network::DP_shortest_path(char start_node, int start_time, char end_node, int end_time) {
    Route **dp = new Route *[num_nodes];
    for (int i = 0; i < num_nodes; i++)
        dp[i] = new Route[TOTAL_TIME_SLOT];


    int start_node_idx = (int) start_node - 'A';
    int end_node_idx = (int) end_node - 'A';

    vector<string> init_node = vector<string>();
    init_node.push_back(start_node + to_string(start_time));

    dp[start_node_idx][start_time] = Route(init_node, stop_cost[start_node_idx]);
    forward_update(dp, start_node_idx, start_time);

    for (int t = start_time + 1; t < end_time; t++) {
        for (int node = 0; node < num_nodes; node++) {
            if (dp[node][t].nodes.empty() == 0)
                forward_update(dp, node, t);
        }
    }
    return dp[end_node_idx][end_time];
}

void Network::forward_update(Route** dp, int node, int time) {
    char node_char = (char) ('A' + node) ;
    Node* cur_node = nodes[node_char][time];

    for (auto& arc : cur_node->out_arcs){
        Node* end_node = arc->end_node;
        char end_node_char = end_node->getName()[0];
        int end_node_idx = (int) end_node_char - 65;
        int end_time = stoi(end_node->getName().substr(1));

        //Calculate cost if append end node to current route
        Route cur_route = dp[node][time];
        Route end_route = dp[end_node_idx][end_time];
        double new_cost = cur_route.cost + arc->cost + arc->fixed_cost + end_node->getCost();
        new_cost = MAX(0, new_cost);

        // if yes, replace old route.
        if (new_cost < end_route.cost){
            vector<string> new_nodes;
            new_nodes.assign(cur_route.nodes.begin(), cur_route.nodes.end());
            new_nodes.push_back(end_node_char + to_string(end_time));
            //cout << end_node_idx << " " << end_time <<endl;

            dp[end_node_idx][end_time] = Route(new_nodes, new_cost);
        }
    }
}

unsigned int Network::getNum_nodes() const {
    return num_nodes;
}

int *Network::getStop_cost() const {
    return stop_cost;
}

int **Network::getArc_cost() const {
    return arc_cost;
}


