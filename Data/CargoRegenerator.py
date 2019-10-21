#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import random


# Classify all node and return list of available airports and harbors.
def read_node_type(sea_stop_cost_file_path, air_stop_cost_file_path):
    airports = []
    harbors = []

    with open(air_stop_cost_file_path) as air_stop_cost_file:
        raw = air_stop_cost_file.read()
        for idx, cost_str in enumerate(raw.split('\t')):
            if int(cost_str) < 99999:
                airports.append(idx)

    with open(sea_stop_cost_file_path) as sea_stop_cost_file:
        raw = sea_stop_cost_file.read()
        for idx, cost_str in enumerate(raw.split('\t')):
            if int(cost_str) < 99999:
                harbors.append(idx)

    return airports, harbors


# Generate cargo file.
def cargo_data_generator(name, n, num_cargoes, cargo_type_prob, total_time_slot, airports, harbors):
    cargo_file = open("%s_cargo.txt" % name, 'w')
    cargo_file.write(str(num_cargoes) + '\n')

    for _ in range(num_cargoes):
        # Now we need to consider airports or harbors which accept cargoes.
        #departure, destination = random.sample([chr(i) for i in range(65, 65+n)], 2)

        starting_time = random.randint(1, total_time_slot // 1.3)
        end_time = random.randint(starting_time + 6, min(starting_time + 63, total_time_slot-1))

        weight = random.randint(20, 99) * 100
        volume = random.randint(20, 99) * 1

        # time_sensitivity = 'H' if end_time - starting_time <=  20 else 'L'
        # product_value = 'H' if  volume <= 500 else 'L'

        # time_sensitivity = 'H' if random.random() > 0.3 else 'L'

        cargo_type = random.choices(population=[0, 1, 2, 3], weights=cargo_type_prob, k=1)[0]

        if cargo_type == 0:
            alpha = -0.0051
            beta = -0.4339
            time_sensitivity = 'H'
            product_value = 'H'

            # From air to air
            departure, destination = random.sample(airports, 2)

        elif cargo_type == 1:
            alpha = -0.0004
            beta = -0.0012
            time_sensitivity = 'H'
            product_value = 'L'

            # From sea to sea or air
            departure = random.choice(harbors)
            harbors_without_departure_node = list(harbors)
            harbors_without_departure_node.remove(departure)

            destination = random.choice(airports+harbors_without_departure_node)
        elif cargo_type == 2:
            alpha = -0.0052
            beta = -0.4787
            time_sensitivity = 'L'
            product_value = 'H'

            # From air or sea to air
            destination = random.choice(airports)
            airports_without_destination_node = list(airports)
            airports_without_destination_node.remove(destination)

            departure = random.choice(airports_without_destination_node + harbors)
        elif cargo_type == 3:
            alpha = -0.0002
            beta = -0.0023
            time_sensitivity = 'L'
            product_value = 'L'

            # From sea to sea
            departure, destination = random.sample(harbors, 2)

        cargo_file.write(
            "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" % (
                chr(departure+65),
                chr(destination+65),
                str(starting_time),
                str(end_time),
                str(weight),
                str(volume),
                time_sensitivity,
                product_value,
                str(alpha),
                str(beta)
            )
        ) #weight = volume

    cargo_file.close()


if __name__ == "__main__":
    name = 'X62'
    n = 62
    num_cargoes = 600
    total_time_slot = 315

    cargo_type_prob = [0.25, 0.25, 0.25, 0.25]  # HH, HL, LH, LL

    air_stop_cost_file_path = "Z62_air_stopcost.txt"
    sea_stop_cost_file_path = "Z62_sea_stopcost.txt"

    airports, harbors = read_node_type(sea_stop_cost_file_path, air_stop_cost_file_path)

    cargo_data_generator(name, n, num_cargoes, cargo_type_prob, total_time_slot, airports, harbors)
