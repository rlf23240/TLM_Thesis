#include <ostream>

//
// Created by Ashee on 2019/6/18.
//
enum cargo_type {
    AirOnly,
    SeaOnly,
    AirBoth,
    SeaBoth
};

struct Cargo{
    Cargo(char departure,
          char destination,
          int start_time,
          int arrive_time,
          int volume,
          int weight,
          cargo_type type):
    departure(departure),
    destination(destination),
    start_time(start_time),
    arrive_time(arrive_time),
    volume(volume),
    weight(weight),
    type(type) {}

    friend std::ostream &operator<<(std::ostream &os, const Cargo &cargo) {
        os << "departure: " << cargo.departure << " destination: " << cargo.destination << " start_time: "
           << cargo.start_time << " arrive_time: " << cargo.arrive_time << " volume: " << cargo.volume << " weight: "
           << cargo.weight << " type: " << cargo.type << std::endl;
        return os;
    }

    void setAlpha(double alpha) {
        Cargo::alpha = alpha;
    }

    void setBeta(double beta) {
        Cargo::beta = beta;
    }

    char departure;
    char destination;
    int start_time;
    int arrive_time;
    int volume;
    int weight;
    float alpha;
    float beta;
    cargo_type type;
};
