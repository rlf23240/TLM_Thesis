//
// Created by Ashee on 2019/6/16.
//
#include <iostream>
#include <ostream>
#include <string>

using namespace std;

struct Point{
    Point(int layer, int node, int time) : layer(layer), node(node), time(time) {}

    bool operator==(const Point &rhs) const {
        return layer == rhs.layer &&
               node == rhs.node &&
               time == rhs.time;
    }

    bool operator!=(const Point &rhs) const {
        return !(rhs == *this);
    }

    Point(const string &name) {

        this->layer = (int) name[0] - 48; //ascii 48 = 0
        this->node = name[1] - 48; //char to int (A to 0)
        this->time = stoi(name.substr(2));
    }

	friend std::ostream &operator<<(std::ostream &os, const Point &point) {
		os  <<  point.layer << num_to_excel_like_alpha(point.node) <<  point.time;
		return os;
	}

	static string num_to_excel_like_alpha(int num){
    	string s;
    	if(num < 26){
    		s.push_back((char) (num + 'A'));
    	}else{
    		int first = (int) num / 26;
    		int second = num % 26;
    		s.push_back((char) (first + 'A'));
    		s.push_back((char) (second + 'A'));
    	}
    	return s;
    }

	int layer;
	int node;
	int time;
};

