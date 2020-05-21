//
// Created by Ashee on 2019/6/16.
//
#include <iostream>
#include <ostream>
#include <string>

using namespace std;

struct Point{
    int layer = 0;
    int node = 0;
    int time = 0;
    
    Point(int layer, int node, int time): layer(layer), node(node), time(time) {}

    bool operator == (const Point &rhs) const {
        return layer == rhs.layer && node == rhs.node && time == rhs.time;
    }

    bool operator != (const Point &rhs) const {
        return !(rhs == *this);
    }

    Point(const string &name) {
        this->layer = (int) name[0] - 48; //ascii 48 = 0
        this->node = name[1] - 65; //char to int (A to 0)
        this->time = stoi(name.substr(2));
    }

    friend std::ostream &operator<<(std::ostream &os, const Point &point) {
        os  <<  point.layer << (char) (65 + point.node) <<  point.time;
        return os;
    }
};

