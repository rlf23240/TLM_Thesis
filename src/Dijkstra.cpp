//
//  Dijkstra.cpp
//  TLM_Thesis
//
//  Created by ian wang on 2019/10/18.
//  Copyright © 2019 ian wang. All rights reserved.
//

#include <vector>
#include "Dijkstra.h"
#include <queue>

using namespace std;

namespace Graph {
    vector<int> dijkstra(int num_nodes, int start_node, vector<vector<int>> distance) {
        vector<int> shortest = vector<int>(num_nodes, 99999);
        vector<bool> visited = vector<bool>(num_nodes, false);
        
        // Not useful for our situation.
        //vector<int> prev = vector<int>(num_nodes, -1);
        
        shortest[start_node] = 0;
        
        while (true) {
            int min_node = -1;
            int min_dist = 99999;
            
            for (int i = 0; i < num_nodes; ++i) {
                if (visited[i] == false && shortest[i] <= min_dist) {
                    min_node = i;
                    min_dist = shortest[i];
                }
            }

            if (min_node == -1) {
                break;
            } else {
                visited[min_node] = true;
                
                for (int i = 0; i < num_nodes; ++i) {
                    int new_dist = shortest[min_node] + distance[min_node][i];
                    if (new_dist < shortest[i]) {
                        shortest[i] = new_dist;
                        // prev[i] = min_node;
                    }
                }
            }
        }
        
        return shortest;
    }
}


