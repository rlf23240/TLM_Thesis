//
// Created by Ashee on 2019/6/16.
//

#include <ostream>
#include <iostream>
#include "Point.cpp"
#include "vector"


struct Path{
    Path(Path& path) {
        this->points.assign(path.points.begin(), path.points.end());
        this->stay_at_virtual = path.stay_at_virtual;
        this->stay_at_same_node = path.stay_at_same_node;
    }

    Path() = default;

    friend std::ostream &operator<<(std::ostream &os, const Path &path) {
        os << "Path :\t:" ;
        for(auto point : path.points){
            os << point << "->";
        }
        os << "<" << std::endl;
        return os;
    }

    void push_point(Point point){
        //if stay in virtual network
        if(point.layer == 2)
            this->stay_at_virtual++;
        else
            this->stay_at_virtual = 0;

        this->points.push_back(point);
    }

    void pop_point(){
        if(this->points.end()->layer == 2)
            this->stay_at_virtual--;


        this->points.pop_back();
    }

    bool is_feasible(){
        if(this->stay_at_virtual > 5)
            return false;
        if(this->size() > 20)
            return false;
        if(this->size() == 2 && this->points.end()->layer == 2)
            return false;
        return true;
    }

    unsigned int size(){
        return this->points.size();
    }
    std::vector<Point> points{};
    int stay_at_virtual = 0;
    int stay_at_same_node = 0;

};
