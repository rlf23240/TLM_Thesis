//
// Created by Ashee on 2019/7/5.
//

#ifndef TLM_THESIS_BB_NODE_H
#define TLM_THESIS_BB_NODE_H

#include "param.h"
#include "gurobi_c++.h"

class BB_node {
private:
    double obj;
    vector<Path*>* target_path;
    vector<Path*>* rival_path;

    unordered_set<int>* chosen_paths;
    unordered_map<int, unordered_map <int, bool>> integer_set;
public:

    BB_node(double obj,
            vector<Path *> *target_path,
            vector<Path *> *rival_path,
            unordered_set<int> *chosen_paths,
            unordered_map<int,unordered_map<int, bool>> integer_set);

    bool operator<(const BB_node &rhs) const;

    bool operator>(const BB_node &rhs) const;

    bool operator<=(const BB_node &rhs) const;

    bool operator>=(const BB_node &rhs) const;

    vector<Path *> *getTargetPath() const;

    vector<Path *> *getRivalPath() const;

    unordered_set<int> *getChosenPaths() const;

    const unordered_map<int, unordered_map<int, bool>> &getIntegerSet() const;

    double getObj() const;

    static int cargo_size;

};


#endif //TLM_THESIS_BB_NODE_H
