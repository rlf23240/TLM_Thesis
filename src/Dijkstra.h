//
//  Dijkstra.hpp
//  TLM_Thesis
//
//  Created by ian wang on 2019/10/18.
//  Copyright © 2019 ian wang. All rights reserved.
//

#ifndef Dijkstra_h
#define Dijkstra_h

#include <stdio.h>
#include <vector>

using namespace std;

namespace Graph {
    vector<int> dijkstra(int num_nodes, int start, vector<vector<int>> distance);
}

#endif /* Dijkstra_hpp */
