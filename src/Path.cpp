//
// Created by Ashee on 2019/6/16.
//

#include <ostream>
#include <iostream>
#include "Point.cpp"
#include "vector"

class Path {
public:
    // Type of path
    enum PathType {
        Sea,
        Air,
        SeaAir
    };
    
private:
    vector<Point> _points {};

public:
    int stay_at_virtual = 0;
    int virtual_entry_time = -1;
    bool enter_virtual_twice = false;
    int index;
    // TODO: Rename this.
    double original_path_cost;
    double path_cost;
    int last_time;
    double path_profit;
    double pi;
    double reduced_cost;
    bool only_rival;
    PathType type;
    
    Path(const Path& path) {
        _points.assign(path.points().begin(), path.points().end());
        this->stay_at_virtual = path.stay_at_virtual;
        this->virtual_entry_time = path.virtual_entry_time;
    }

    Path(Point start_point){
        _points.push_back(start_point);
    }

    Path() = default;

    bool operator==(const Path &rhs) const {
        return points() == rhs.points();
    }

    void setIndex(int index) {
        Path::index = index;
    }

    friend std::ostream &operator<<(std::ostream &os, const Path &path) {
        os << "Path :" ;
        for(auto point : path.points()){
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

        if (!enter_virtual_twice &&
            virtual_entry_time == -1 &&
            (!this->points().empty() && point.layer == 2 && points().back().layer != 2)
        ) {
            virtual_entry_time = point.time;
            
        } else if(virtual_entry_time > -1 &&
                  (length() != 0 && point.layer == 2 && points().back().layer != 2)) {
            
            enter_virtual_twice = true;
        }


        _points.push_back(point);
//        cout << enter_virtual_twice << " " << virtual_entry_time << " " << *this;
    }

    void pop_point(){
        if(this->points().back().layer == 2)
            this->stay_at_virtual--;

        if(this->points().back().layer == 2) {
            if (enter_virtual_twice)
                enter_virtual_twice = false;
            else if (!enter_virtual_twice && this->points().back().time == virtual_entry_time)
                virtual_entry_time = -1;
        }
        _points.pop_back();

    }

    bool is_feasible() {
        if(this->stay_at_virtual > 6)
            return false;
        if(this->length() == 2 && this->points().back().layer == 2)
            return false;
        if(this->length() >= 15)
            return false;
        if(enter_virtual_twice == true) {
            return false;
        }
        return true;
    }
    
    const vector<Point>& points() const {
        return _points;
    }

    size_t length() const{
        return _points.size();
    }

    double net_profit(){
        return path_profit - path_cost;
    }
    
    int get_start_time(){
        return _points.front().time;
    }
    
    int get_end_time(){
        return _points.back().time;
    }
    
    double fixed_profit(){
        return path_profit + pi;
    }
};
