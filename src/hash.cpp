//
//  hash.cpp
//  TLM_Thesis
//
//  Created by Ian Wang on 2020/4/25.
//  Copyright © 2020 ian wang. All rights reserved.
//

#include "hash.hpp"

using namespace std;

size_t hash_combine(size_t h1, size_t h2) {
    // Copy from hash_combine from boost library...
    // TODO: Do we need include boost library?
    return h1 ^ (h1 + 0x9e3779b9 + (h1 << 6) + (h2 >> 2));
}
