//
// Created by Ashee on 2019/6/16.
//
#include <iostream>
#include <ostream>
#include <string>

using namespace std;

struct Point{
    Point(int layer, int node, int time) : layer(layer), node(node), time(time) {}

    Point(const string &name) {

        this->layer = (int) name[0] - 48; //ascii 48 = 0
        this->node = name[1] - 65; //char to int (A to 0)
        this->time = stoi(name.substr(2));
    }

    friend std::ostream &operator<<(std::ostream &os, const Point &point) {
        os  <<  point.layer << (char) (65 + point.node) <<  point.time;
        return os;
    }

    int layer;
    int node;
    int time;
};

