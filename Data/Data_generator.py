import random
import numpy
from math import floor

sea_time_cost = None
air_time_cost = None
ship_stop_day = 1
air_weight_lb = 15
air_weight_ub = 20 # * 100
air_volume_lb = 15
air_volume_ub = 20
sea_volume_lb = 30
sea_volume_ub = 50


def data_generator(name = "A", n = 10, num_ships = 20, num_flights = 20, num_cargos = 100, total_time_slot = 84):
    # random.seed(5)
    sea_data_generator(name, n, num_ships)
    air_data_generator(name, n, num_flights)
    # virtual_data_generator(name, n)
    # cargo_data_generator(name, n, num_cargos)
    # param(name, n, num_ships, num_flights, total_time_slot)
    # unload_cost(name, n)
    unit_profit(name, n)



def sea_data_generator(name, n, num_ships):
    sea_arc_cost = [[0 for _ in range(n)] for _ in range(n)]
    sea_time_cost = [[0 for _ in range(n)] for _ in range(n)]
    stop_cost = [random.randint(15,30) * 10 for _ in range(n)]
    def sea_arc_time_cost():
        arc_file = open("%s_sea_arccost.txt" % name,'w')
        time_file = open("%s_sea_timecost.txt" % name,'w')
        arc_file.write(str(n) + '\n')
        time_file.write(str(n) + '\n')

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

        for i in range(n) :
            stop_file.write(str(stop_cost[i]))
            if i != n - 1 :
                stop_file.write('\t')
        stop_file.close()
    def sea_ships_param(num_ships) :
        param_file = open("%s_sea_ships_param.txt" % name,'w')
        param_file.write(str(1) + '\n')

        node = chr(65 + random.randint(0,n-1))
        starting_time = random.randint(0,10)
        freq = 1
        cycle_time = round(numpy.random.randint(40,50))
        volume_ub = (sea_volume_lb + random.randint(0,sea_volume_ub - sea_volume_lb)) * 100 #weight upper bound
        param_file.write(node + '\t' + str(starting_time) + '\t' + str(freq) + '\t' + str(cycle_time) + '\t' + str(volume_ub)+ '\n')

        param_file.close()
    def sea_route_generator(num_ships, file_prefix, unlimit_ub):
        file = open("%s_routes.csv" % file_prefix, 'w')
        file.write(str(num_ships) + "\n")
        for i in range(num_ships) :
            start_node = random.randint(0, n-1)
            start_time = random.randint(0, 9)
            cur_node = start_node
            cur_time = start_time

            node_list = []
            node_list.append(chr(start_node+65)+str(start_time))

            while cur_time - start_time < 30 :
                while True :
                    next_node = random.randint(0, n-1)
                    if cur_node != next_node :
                        break
                next_time = cur_time + sea_time_cost[cur_node][next_node]
                node_list.append(chr(65 + next_node) + str(next_time))
                node_list.append(chr(65 + next_node) + str(next_time+ ship_stop_day))

                cur_node = next_node
                cur_time = next_time
            if cur_node != start_node :
                next_node = start_node
                next_time = cur_time + sea_time_cost[cur_node][next_node]
                node_list.append(chr(65+next_node) + str(next_time))

            if not unlimit_ub :
                volume_ub = (sea_volume_lb + random.randint(0,sea_volume_ub - sea_volume_lb)) * 100
            else :
                volume_ub = 99999
            file.write(str(volume_ub) + ",")
            for node in node_list :
                file.write(node)
                if node != node_list[-1] :
                    file.write(",")
            file.write("\n")
        file.close()
    def sea_unit_cost(name, n) :
        sea_cost_file = open("%s_sea_trans_cost.txt"% name, 'w')

        for i in range(n) :
            for j in range(n) :
                sea_cost = random.random() * sea_time_cost[i][j]
                sea_cost_file.write(str(round(sea_cost,2)))
                if j != n-1 :
                    sea_cost_file.write("\t")
            if i != n-1 :
                sea_cost_file.write("\n")
        sea_cost_file.close()

    # sea_arc_time_cost()
    # sea_stop_cost()
    # sea_ships_param(num_ships)
    # sea_route_generator(num_ships, name + "_sea_target", False)
    # sea_route_generator(num_ships, name + "_sea_rival", True)
    sea_unit_cost(name, n)

def air_data_generator(name, n, num_flights):
    air_arc_cost = [[0 for _ in range(n)] for _ in range(n)]
    air_time_cost = [[0 for _ in range(n)] for _ in range(n)]
    stop_cost = [random.randint(15,30) * 10 for _ in range(n)]
    def air_arc_time_cost():
        arc_file = open("%s_air_arccost.txt" % name,'w')
        time_file = open("%s_air_timecost.txt" % name,'w')
        arc_file.write(str(n) + '\n')
        time_file.write(str(n) + '\n')



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

        for i in range(n) :
            stop_file.write(str(stop_cost[i]))
            if i != n - 1 :
                stop_file.write('\t')
        stop_file.close()
    def air_flights_param(num_flights) :
        param_file = open("%s_air_flights_param.txt" % name,'w')
        param_file.write(str(1) + '\n')

        for i in range(1):
            node = chr(65 + random.randint(0,n-1))
            cycle_time = random.randint(5,7)
            gap = cycle_time + 1 if random.random() < 0.8 else cycle_time + 2
            weight_ub = random.randint(air_weight_lb, air_volume_ub)*100
            volume_ub = random.randint(air_volume_lb, air_volume_ub)*100
            freq = floor(18 / gap)
            param_file.write(node + '\t' + str(gap) + '\t' + str(freq) + '\t' + str(cycle_time)+ '\t' + str(volume_ub)+ '\t' + str(weight_ub) + '\n')
        param_file.close()
    def air_route_generator(num_flights, file_prefix, unlimit_ub):
        file = open("%s_routes.csv" % file_prefix, 'w')
        file.write(str(num_flights) + "\n")
        for i in range(num_flights) :
            cycle_time = random.randint(5,7)

            start_node = random.randint(0, n-1)
            start_time = random.randint(0,4)
            cur_node = start_node
            cur_time = start_time

            node_list = []
            node_list.append(chr(65 + cur_node) + str(cur_time))
            while cur_time - start_time < cycle_time -1 :
                while True :
                    next_node = random.randint(0, n-1)
                    if next_node != cur_node :
                        break

                next_time = cur_time + air_time_cost[cur_node][next_node]

                node_list.append(chr(65 + next_node) + str(next_time))

                cur_node = next_node
                cur_time = next_time
            if cur_node != start_node :
                next_node = start_node
                next_time = cur_time + air_time_cost[cur_node][next_node]
                cur_node = next_node
                cur_time = next_time
                node_list.append(chr(65 + cur_node) + str(cur_time))

            cycle_time = cur_time - start_time
            gap = cycle_time  + 1 if random.random() < 0.5 else cycle_time + 2
            freq = floor((21 - start_time) / gap)

            if not unlimit_ub :
                weight_ub = random.randint(air_weight_lb, air_volume_ub)*100
                volume_ub = random.randint(air_volume_lb, air_volume_ub)*100
            else :
                weight_ub = 99999
                volume_ub = 99999

            file.write(str(volume_ub) + "," + str(weight_ub) + ",")
            for f in range(freq) :
                for node in node_list :
                    node_ = node[0] + str(int(node[1:]) + gap * f)
                    file.write(node_)
                    if node != node_list[-1] or f != freq-1 :
                        file.write(",")
                if f != freq-1 :
                    file.write(",")
            file.write("\n")
        file.close()
    def air_unit_cost(name, n) :
        air_cost_file = open("%s_air_trans_cost.txt"% name, 'w')
        for i in range(n) :
            for j in range(n) :
                if i != j :
                    air_cost = random.random() * air_time_cost[i][j]
                    air_cost_file.write(str(round(air_cost,2)))
                else :
                    air_cost_file.write(str(99999))
                if j != n-1 :
                    air_cost_file.write("\t")
            if i != n-1 :
                air_cost_file.write("\n")
        air_cost_file.close()



    # air_arc_time_cost()
    # air_stop_cost()
    # air_flights_param(num_flights)
    # air_route_generator(num_flights, name + "_air_target", False)
    # air_route_generator(num_flights, name + "_air_rival", True)
    air_unit_cost(name, n)

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
        departure, destination = random.sample([chr(i) for i in range(65, 65+n)], 2)

        starting_time = random.randint(1,20)
        end_time = random.randint(25,60)

        weight = random.randint(20,99) * 10
        volume = random.randint(20,99) * 10

        time_sensitivity = 'H' if end_time - starting_time <=  20 else 'L'
        product_value = 'H' if  volume <= 500 else 'L'

        # time_sensitivity = 'H' if random.random() > 0.3 else 'L'

        alpha, beta = None, None
        if time_sensitivity == "H" and product_value == "H" :
            alpha = -0.0051
            beta = -0.4339
        elif time_sensitivity == "H" and product_value == "L" :
            alpha = -0.0004
            beta = -0.0012
        elif time_sensitivity == "L" and product_value == "H" :
            alpha = -0.0052
            beta = -0.4787
        elif time_sensitivity == "L" and product_value == "L" :
            alpha = -0.002
            beta = -0.0023


        cargo_file.write("%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n" %
                         (departure, destination,str(starting_time), str(end_time), str(weight), str(weight), time_sensitivity, product_value, str(alpha), str(beta))) #weight = volume
    cargo_file.close()

def param(name, n, num_ships, num_flights, total_time_slot) :
    param_file = open("%s_param.txt"% name, 'w')
    param_file.write("%s\t%s\t%s\t%s" %(n, num_ships, num_flights, total_time_slot))
    param_file.close()

def unload_cost(name, n) :
    unload_file = open("%s_unload_cost.txt"% name, 'w')

    for i in range(n) :
        cost = random.randint(5,25) * 10
        unload_file.write(str(cost))
        if i != n-1 : unload_file.write("\n")
    unload_file.close()
def unit_profit(name, n) :
    air_profit_file = open("%s_air_profit.txt"% name, 'w')
    sea_profit_file = open("%s_sea_profit.txt"% name, 'w')

    for i in range(n) :
        for j in range(n) :
            air_cost = random.random()
            sea_cost = random.random()
            air_profit_file.write(str(round(air_cost,2)))
            sea_profit_file.write(str(round(sea_cost,2)))
            if j != n-1 :
                air_profit_file.write("\t")
                sea_profit_file.write("\t")
        if i != n-1 :
            air_profit_file.write("\n")
            sea_profit_file.write("\n")
    air_profit_file.close()
    sea_profit_file.close()




if __name__ == "__main__" :
    # data_generator(name = "A", n = 4, num_flights= 4, num_ships=4, num_cargos=40)
    # data_generator(name = "B", n = 6, num_flights= 6, num_ships=6, num_cargos=60)
    # data_generator(name = "C", n = 8, num_flights= 8, num_ships=8, num_cargos=80)
    # data_generator(name = "D", n = 10, num_flights= 10, num_ships=10, num_cargos=100)
    # data_generator(name = "E", n = 12, num_flights= 12, num_ships=12, num_cargos=120)
    # data_generator(name = "B2", n = 6, num_flights= 2, num_ships=2, num_cargos=30)
    # data_generator(name = "A1", n = 4, num_flights= 1, num_ships=1, num_cargos=5)
    # data_generator(name = "A2", n = 4, num_flights= 1, num_ships=1, num_cargos=10)
    # data_generator(name = "A3", n = 4, num_flights= 1, num_ships=1, num_cargos=15)
    # data_generator(name = "A4", n = 4, num_flights= 1, num_ships=1, num_cargos=20)
    data_generator(name = "F", n = 10, num_flights= 1, num_ships=1, num_cargos=20)
