//
// Created by Ashee on 2019/5/22.
//

#include "Network.h"
#include "Node.h"
#include "param.h"
#include <algorithm>

using namespace std;

/*-----------------Route struct------------------------------*/
Route::Route(const vector<string> &nodes, double updated_cost) : nodes(nodes), updated_cost(updated_cost) {}

Route::Route() = default;

ostream &operator<<(ostream &os, const Route &route) {

    if(!route.nodes.empty()) {
        os << "Route : ";
        for (const string &node: route.nodes) {
            os << node << "->";
        }
        os << "<\t\tupdated cost : ";
        os << route.updated_cost << endl;
    }
    else {
        os << "Empty route" << endl;
    }

    return os;
}

Route::Route(Route route, int gap) {
    for(const string &node: route.nodes) {
        char n = node[0];
        int time = stoi(node.substr(1)) + gap;

        this->nodes.push_back(n + to_string(time));
        this->updated_cost = route.updated_cost;
    }

}

double Route::getCost(Network *network) const {
    double result = 0.0;
    
    string prev_node = "";
    for(const string &node: nodes) {
        double stop_cost = network->getStop_cost()[(int)(node[0]) - 65];
        double arc_cost = 0.0;
        if(prev_node.empty() == false) {
            arc_cost = network->getArc_cost()[(int)prev_node[0] - 65][(int)node[0] - 65];
        }
        result += (stop_cost + arc_cost);
        
        prev_node = node;
    }
    
    return result;
}

/*-----------------------Network class------------------------*/
void Network::read_data(std::string data_path) {
    read_node(data_path + "_arccost.txt");
    read_stop_cost(data_path + "_stopcost.txt");
    read_time_cost(data_path + "_timecost.txt");
    add_nodes();
    add_edges();
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

// TODO: Change the responsibility of freeing memory to caller.
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
//                    << "   Cost :" <<arc_cost[i][out]<<endl;
                    add_edge(nodes[(char) 'A'+i][t], nodes[(char) 'A'+out][t + time_cost[i][out]], arc_cost[i][out]);
                            // start                  end                                            cost
                }
            }
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
        
Network::~Network() {
    delete[] stop_cost;
    
    for (int i = 0; i < num_nodes; ++i) {
        delete[] arc_cost[i];
    }
    delete[] arc_cost;
    
    for (int i = 0; i < num_nodes; ++i) {
        delete[] time_cost[i];
    }
    delete[] time_cost;
    
    for (auto& node_set: nodes) {
        for (auto& node: node_set.second) {
            delete node;
        }
    }
}
