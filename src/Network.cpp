//
// Created by Ashee on 2019/5/22.
//

#include "Network.h"
#include "Arc.h"
#include "Node.h"
#include "param.h"

using namespace std;

Network::Network() {
}

bool Network::add_node(std::string name, int cost) {
    Node* new_node = new Node(name);
    new_node->setCost(cost);
    if(nodes[name] != nullptr){
        return false; //arc exist
    }
    nodes[name] = new_node;
    return true;
}

bool Network::add_edge(std::string start, std::string end, int cost) {
    Node* start_node = nodes[start];
    Node* end_node = nodes[end];
    Arc* new_arc = new Arc(start, end, cost);
    if(start_node->out_arc[end] != nullptr || end_node->out_arc[start] != nullptr)
        return false; //arc exist
    start_node->out_arc[end] = new_arc;
    end_node->in_arc[start] = new_arc;
    return true;
}

void Network::read_data(std::string data_path) {
    read_node(data_path + "_arccost.txt");
    read_stop_cost(data_path + "stopcost.txt");
    read_flights_param(data_path + "baseairport.txt");
}

void Network::read_node(std::string node_data_path) {

    fstream file;
    file.open(node_data_path);
    string line;
    getline(file,line);
    setN_nodes(stoi(line));
    arc_cost = new int*[getN_nodes()];
    for(int i = 0 ; i < getN_nodes() ; i++)
        arc_cost[i] = new int[getN_nodes()];


    if(file.is_open()) {
        //read arc cost
        for (int i = 0; getline(file, line); i++) { //row counter
            istringstream iss(line);
            string token;
            for (int j = 0 ;getline(iss, token, '\t') ; j++) { //col counter
                if( token[0] != 'M') {
                    arc_cost[i][j] = stoi(token);
                    cout << arc_cost[i][j] << " ";
                }
                else{
                    arc_cost[i][j] = INT_MAX;
                }
            }

        }
        //add nodes
        for(int i = 0; i < getN_nodes(); i++){
            for(int j = 0; j < TOTAL_TIME_SLOT; j++){
                add_node()
            }
        }



    }
    else {
        cout << "Can't read node file !!!" << endl;
    }


}

void Network::read_stop_cost(std::string cost_data_path) {

}

void Network::read_flights_param(std::string flights_data) {

}

int Network::getN_nodes() const {
    return N_nodes;
}

void Network::setN_nodes(int N_nodes) {
    Network::N_nodes = N_nodes;
}





