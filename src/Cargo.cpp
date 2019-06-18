//
// Created by Ashee on 2019/6/18.
//
enum cargo_type{only_air, only_sea, air_both, sea_both};

struct Cargo{
    int departure;
    int destination;
    int volume;
    int weight;
    int start_time;
    int arrive_time;
    cargo_type type;
};
