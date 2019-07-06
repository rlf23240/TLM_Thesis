//
// Created by Ashee on 2019/7/5.
//

#include "BB_node.h"

BB_node::BB_node(double obj,
        vector<Path *> *target_path,
        vector<Path *> *rival_path,
        unordered_set<int> *chosen_paths,
        unordered_map<int, unordered_map <int, bool>> integer_set){
    this->obj = obj;
    this->integer_set = integer_set;

    this->target_path = new vector<Path*>[cargo_size];
    this->rival_path = new vector<Path*>[cargo_size];
    this->chosen_paths = new unordered_set<int>[cargo_size];
    for(int k = 0; k < cargo_size; k++){
        this->target_path[k].assign(target_path[k].begin(), target_path[k].end()); //copy vector
        this->rival_path[k].assign(rival_path[k].begin(), rival_path[k].end()); //copy vector
        this->chosen_paths[k] = chosen_paths[k];
    }

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

const unordered_map<int, unordered_map<int, bool>> &BB_node::getIntegerSet() const {
    return integer_set;
}

