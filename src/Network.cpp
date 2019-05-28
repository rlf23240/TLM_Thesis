//
// Created by Ashee on 2019/5/22.
//

#include "Network.h"
#include "Node.h"
#include "param.h"

using namespace std;


Flight::Flight(const string &start_node, int gap, int freq, int cycle_time) : start_node(start_node), gap(gap),
                                                                              freq(freq), cycle_time(cycle_time) {}

ostream &operator<<(ostream &os, const Flight &flight) {
    os << "start_node: " << flight.start_node << " gap: " << flight.gap << " freq: " << flight.freq << " cycle_time: "
       << flight.cycle_time;
    return os;
}

Network::Network() {
}

bool Network::add_edge(Node* start, Node* end, int cost) {
    Arc* new_arc = new Arc(start, end, cost);
    start->out_arc.push_back(new_arc);
    end->in_arc.push_back(new_arc);

    return true;
}

void Network::read_data(std::string data_path) {
    read_node(data_path + "_arccost.txt");
    read_stop_cost(data_path + "_stopcost.txt");
    read_flights_param(data_path + "_flights_gap_freq_ct.txt");
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
//            cout << line << endl;
            istringstream iss(line);
            string token;
            for (int j = 0 ;getline(iss, token, '\t') ; j++) { //col counter
                if( token[0] != 'M') {
                    arc_cost[i][j] = stoi(token);
//                    cout << arc_cost[i][j] << " ";
                }
                else{
                    arc_cost[i][j] = INT_MAX;
                }
            }
        }
//        for(int i = 0; i < 4; i++){
//            for(int j = 0; j < 4;j++){
//                cout << arc_cost[i][j] << ' ';
//            }
//            cout << endl;
//        }
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
        cout << "stop_cost file cannot open !!!";
    }
}

void Network::read_flights_param(std::string flights_data) {

    vector<Flight> flights;
    fstream file;
    file.open(flights_data);


    if(file.is_open()){
        string line;
        getline(file, line);
        int num_flights = stoi(line);

        for(int i = 0; i < num_flights ; i++){
            getline(file, line);
            istringstream iss(line);
            string token;

            getline(iss, token, ' ');
            string flight_name = token;

            getline(iss, token, ' ');
            int flight_gap = stoi(token);

            getline(iss, token, ' ');
            int flight_freq = stoi(token);

            getline(iss, token, ' ');
            int flight_cycle_time = stoi(token);

            Flight new_flight = Flight(flight_name, flight_gap, flight_freq, flight_cycle_time);
            flights.push_back(new_flight);
        }
    }
    else {
        cout << "flights data cannot open !!!";
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
//        cout << endl;
//        for(int i = 0; i < 4; i++){
//            for(int j = 0; j < 4;j++){
//                cout << time_cost[i][j] << '\t';
//            }
//            cout << endl;
//        }
    }
    else {
        cout << "Can't read node file !!!" << endl;
    }

}


void Network::add_nodes() {
    // add time space network
    for(int i = 0; i < num_nodes; i++){
        nodes[(char) 'A'+i] = vector<Node*>();

        for(int node = 1; node <= TOTAL_TIME_SLOT; node++){
            Node* newNode = new Node(stop_cost[i]);
            nodes[(char) 'A'+i].push_back(newNode);
        }
    }
}

void Network::add_edges() {

    for(int t = 1; t <= TOTAL_TIME_SLOT; t++){
        for(int i = 0; i < num_nodes; i++){
            for(int out = 0; out < num_nodes; out++){
                if(arc_cost[i][out] < INT_MAX && t+time_cost[i][out] < TOTAL_TIME_SLOT){
                    cout << (char) (65+i) <<t<< " To " << (char) (65+out)<<t+time_cost[i][out]
                    << "  Arc Cost :" <<arc_cost[i][out]<<endl;
                    add_edge(nodes[(char) 'A'+i][t], nodes[(char) 'A'+out][t + time_cost[i][out]], arc_cost[i][out]);
                            // start                  end                                            cost
                }
            }
        }
    }
}

