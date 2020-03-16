//
// Created by Ashee on 2019/7/5.
//

#ifndef TLM_THESIS_BB_NODE_H
#define TLM_THESIS_BB_NODE_H

#include "param.h"

class BB_node {
private:
    double obj;
    vector<Path*>* target_path;
    vector<Path*>* rival_path;
    
#pragma mark Column Deletion
    vector<unordered_map<Path*, int>>* not_use_count = NULL;
    
    unordered_set<int>* chosen_paths;
    unordered_map<int, unordered_map <int, bool>> integer_set;
public:
    BB_node(double obj,
            vector<Path *> *target_path,
            vector<Path *> *rival_path,
            unordered_set<int> *chosen_paths,
            vector<unordered_map<Path*, int>>* not_use_count,
            unordered_map<int,unordered_map<int, bool>> integer_set);
    
    ~BB_node();

    bool operator<(const BB_node &rhs) const;

    bool operator>(const BB_node &rhs) const;

    bool operator<=(const BB_node &rhs) const;

    bool operator>=(const BB_node &rhs) const;

    vector<Path *> *getTargetPath() const;

    vector<Path *> *getRivalPath() const;
    
    vector<unordered_map<Path*, int>>* getNotUseCount() const;

    unordered_set<int> *getChosenPaths() const;

    const unordered_map<int, unordered_map<int, bool>> &getIntegerSet() const;

    double getObj() const;

    static int cargo_size;

};


#endif //TLM_THESIS_BB_NODE_H
