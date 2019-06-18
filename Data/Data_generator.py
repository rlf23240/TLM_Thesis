import random
import numpy
from math import floor


sea_time_cost = None
air_time_cost = None

def data_generator(name = "A", n = 10, num_ships = 20, num_flights = 20, num_cargos = 100):
    random.seed(10)
    sea_data_generator(name, n, num_ships)
    air_data_generator(name, n, num_flights)
    virtual_data_generator(name, n)
    cargo_data_generator(name, n, num_cargos)


def sea_data_generator(name, n, num_ships):
    def sea_arc_time_cost():
        arc_file = open("%s_sea_arccost.txt" % name,'w')
        time_file = open("%s_sea_timecost.txt" % name,'w')
        arc_file.write(str(n) + '\n')
        time_file.write(str(n) + '\n')

        sea_arc_cost = [[0 for _ in range(n)] for _ in range(n)]
        sea_time_cost = [[0 for _ in range(n)] for _ in range(n)]

        for i in range(n) :
            sea_arc_cost[i][i] = 1000
            sea_time_cost[i][i] = 1

        for i in range(n):
            for j in range(i+1,n):
                cost = random.randint(3,20)
                sea_arc_cost[i][j] = cost * 10
                sea_arc_cost[j][i] = cost * 10
                sea_time_cost[i][j] = cost
                sea_time_cost[j][i] = cost

        for i in range(n) :
            for j in range(n) :
                arc_file.write(str(sea_arc_cost[i][j]))
                time_file.write(str(sea_time_cost[i][j]))
                if j != n-1 :
                    arc_file.write('\t')
                    time_file.write('\t')
            arc_file.write('\n')
            time_file.write('\n')
        arc_file.close()
        time_file.close()
    def sea_stop_cost() :
        stop_file = open("%s_sea_stopcost.txt" % name,'w')
        stop_cost = [random.randint(15,30) * 10 for _ in range(n)]
        for i in range(n) :
            stop_file.write(str(stop_cost[i]))
            if i != n - 1 :
                stop_file.write('\t')
        stop_file.close()
    def sea_ships_param(num_ships) :
        param_file = open("%s_sea_ships_param.txt" % name,'w')
        param_file.write(str(num_ships) + '\n')

        for i in range(num_ships):
            node = chr(65 + random.randint(0,n-1))
            starting_time = random.randint(0,20)
            freq = 1
            cycle_time = round(numpy.random.normal(42,5))
            weight_ub = random.randint(30,50) * 100 #weight upper bound
            param_file.write(node + '\t' + str(starting_time) + '\t' + str(freq) + '\t' + str(cycle_time) + '\t' + str(weight_ub)+ '\n')
        param_file.close()

    sea_arc_time_cost()
    sea_stop_cost()
    sea_ships_param(num_ships)

def air_data_generator(name, n, num_flights):
    def air_arc_time_cost():
        arc_file = open("%s_air_arccost.txt" % name,'w')
        time_file = open("%s_air_timecost.txt" % name,'w')
        arc_file.write(str(n) + '\n')
        time_file.write(str(n) + '\n')

        air_arc_cost = [[0 for _ in range(n)] for _ in range(n)]
        air_time_cost = [[0 for _ in range(n)] for _ in range(n)]

        for i in range(n) :
            air_arc_cost[i][i] = 'M'
            air_time_cost[i][i] = 'M'

        for i in range(n):
            for j in range(i+1,n):
                cost = random.randint(1,2)
                air_arc_cost[i][j] = cost * 80
                air_arc_cost[j][i] = cost * 80
                air_time_cost[i][j] = cost
                air_time_cost[j][i] = cost

        for i in range(n) :
            for j in range(n) :
                arc_file.write(str(air_arc_cost[i][j]))
                time_file.write(str(air_time_cost[i][j]))
                if j != n-1 :
                    arc_file.write('\t')
                    time_file.write('\t')
            arc_file.write('\n')
            time_file.write('\n')
        arc_file.close()
        time_file.close()
    def air_stop_cost() :
        stop_file = open("%s_air_stopcost.txt" % name,'w')
        stop_cost = [random.randint(15,30) * 10 for _ in range(n)]
        for i in range(n) :
            stop_file.write(str(stop_cost[i]))
            if i != n - 1 :
                stop_file.write('\t')
        stop_file.close()
    def air_ships_param(num_flights) :
        param_file = open("%s_air_flights_param.txt" % name,'w')
        param_file.write(str(num_flights) + '\n')

        for i in range(num_flights):
            node = chr(65 + random.randint(0,n-1))
            cycle_time = random.randint(5,8)
            gap = cycle_time + numpy.random.poisson(0.2) + 1
            weight_ub = random.randint(10,20)*100
            volume_ub = random.randint(10,20)*100
            freq = floor(20 / gap)
            param_file.write(node + '\t' + str(gap) + '\t' + str(freq) + '\t' + str(cycle_time)+ '\t' + str(weight_ub)+ '\t' + str(volume_ub) + '\n')
        param_file.close()

    air_arc_time_cost()
    air_stop_cost()
    air_ships_param(num_flights)

def virtual_data_generator(name,n) :
    virtual_file = open("%s_virtual.txt"% name, 'w')
    for i in range(n) :
        transfer_cost = random.randint(5,15) * 10
        virtual_file.write(str(transfer_cost))
        if i < n-1 :
            virtual_file.write('\t')
    virtual_file.close()

def cargo_data_generator(name, n,num_cargos):
    cargo_file = open("%s_cargo.txt"% name, 'w')
    cargo_file.write(str(num_cargos) + '\n')

    for _ in range(num_cargos) :
        start, end = random.sample([chr(i) for i in range(65, 65+n)], 2)
        weight = random.randint(1,10) * 100
        volume = random.randint(1,15)
        time_sensitivity = random.choice(['H','L'])
        sensitivity = random.choice(['H','L'])

        cargo_file.write("%s\t%s\t%s\t%s\t%s\t%s\n" % (start, end, str(weight), str(volume), time_sensitivity, sensitivity))
    cargo_file.close()

if __name__ == "__main__" :
    # data_generator(name = "A", n = 10, num_flights= 10, num_ships=10, num_cargos=100)
    # data_generator(name = "B", n = 4, num_flights= 2, num_ships=2, num_cargos=20)
    data_generator(name = "C", n = 20, num_flights= 15, num_ships=15, num_cargos=200)