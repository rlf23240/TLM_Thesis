//
// Created by Ashee on 2019/7/5.
//

#include "BB_node.h"

BB_node::BB_node(double obj,
                 vector<Path *> *target_path,
                 vector<Path *> *rival_path,
                 unordered_set<int> *chosen_paths,
                 vector<unordered_map<Path, int>>* not_use_count,
                 unordered_map<int, unordered_map<Path, bool>> integer_set){
    this->obj = obj;
//    this->integer_set = integer_set;

    this->target_path = new vector<Path*>[cargo_size];
    this->rival_path = new vector<Path*>[cargo_size];
    this->chosen_paths = new unordered_set<int>[cargo_size];
    this->not_use_count = new vector<unordered_map<Path, int>>(cargo_size, unordered_map<Path, int>());
    
    for(int k = 0; k < cargo_size; k++){
        // TODO: Check if this need deep copy or not!
        this->target_path[k].assign(target_path[k].begin(), target_path[k].end()); //copy vector
        this->rival_path[k].assign(rival_path[k].begin(), rival_path[k].end()); //copy vector
        this->chosen_paths[k] = chosen_paths[k];
        
        // Copy map
        for(auto key_value_pair: (*not_use_count)[k]) {
            this->not_use_count->at(k)[key_value_pair.first] = key_value_pair.second;
        }
    }
    
    this->integer_set = integer_set;
    /*for(auto &k : integer_set){
        for(auto &p : integer_set[k.first]){
            this->integer_set[k.first][p.first] = p.second;
        }
    }*/

}

bool BB_node::operator<(const BB_node &rhs) const {
    return obj < rhs.obj;
}

bool BB_node::operator>(const BB_node &rhs) const {
    return rhs < *this;
}

bool BB_node::operator<=(const BB_node &rhs) const {
    return !(rhs < *this);
}

bool BB_node::operator>=(const BB_node &rhs) const {
    return !(*this < rhs);
}

int BB_node::cargo_size = 0;

vector<Path *> *BB_node::getTargetPath() const {
    return target_path;
}

vector<Path *> *BB_node::getRivalPath() const {
    return rival_path;
}

unordered_set<int> *BB_node::getChosenPaths() const {
    return chosen_paths;
}

const unordered_map<int, unordered_map<Path, bool>> &BB_node::getIntegerSet() const {
    return integer_set;
}

double BB_node::getObj() const {
    return obj;
}
    
vector<unordered_map<Path, int>>* BB_node::getNotUseCount() const {
    return not_use_count;
}

BB_node::~BB_node() {
    /*delete[] target_path;
    delete[] rival_path;
    delete[] chosen_paths;
    delete not_use_count;*/
}
