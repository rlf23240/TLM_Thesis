//
// Created by Ashee on 2019/7/5.
//

#include "BB_node.h"

BB_node::BB_node(double obj, vector<Path *> *target_path, vector<Path *> *rival_path, unordered_set<int> *chosen_paths){
//    cout << cargo_size << endl;
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

