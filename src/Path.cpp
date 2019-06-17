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
        this->virtual_entry_time = path.virtual_entry_time;
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

        if(!enter_virtual_twice && (point.layer == 2 && this->points.end()->layer != 2))
            virtual_entry_time = point.time;
        else if(virtual_entry_time > -1 && (point.layer == 2 && this->points.end()->layer != 2))
            enter_virtual_twice = true;

        this->points.push_back(point);
    }

    void pop_point(){
        if(this->points.end()->layer == 2)
            this->stay_at_virtual--;

        if(enter_virtual_twice && this->points.end()->layer == 2)
            enter_virtual_twice = false;
        else if(!enter_virtual_twice && this->points.end()->time == virtual_entry_time)
            virtual_entry_time = -1;

        this->points.pop_back();
    }

    bool is_feasible(){
        if(this->stay_at_virtual > 10)
            return false;
        if(this->size() > 25)
            return false;
        if(this->size() == 2 && this->points.end()->layer == 2)
            return false;
        if(this->points.begin()->layer == 2)
            return false;
        if(enter_virtual_twice == true)
            return false;
        return true;
    }

    unsigned int size(){
        return this->points.size();
    }
    std::vector<Point> points{};
    int stay_at_virtual = 0;
    int virtual_entry_time = -1;
    bool enter_virtual_twice = false;

};
