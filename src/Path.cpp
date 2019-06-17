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
        this->points.push_back(point);
    }

    void pop_point(){
        this->points.pop_back();
    }

    std::vector<Point> points{};
    int stay_at_virtual = 0;

};
