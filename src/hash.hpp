//
//  hash.hpp
//  TLM_Thesis
//
//  Created by Ian Wang on 2020/4/25.
//  Copyright © 2020 ian wang. All rights reserved.
//

#ifndef hash_hpp
#define hash_hpp

#include <iostream>

// TODO: Do we need include boost library?
std::size_t hash_combine(std::size_t h1, std::size_t h2);

//Can't use pair as element in set without this struct
struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (std::pair<T1, T2> const &pair) const {
        std::size_t h1 = std::hash<T1>()(pair.first);
        std::size_t h2 = std::hash<T2>()(pair.second);
        
        // Copy from hash_combine from boost library...
        // TODO: Do we need include boost library?
        return hash_combine(h1, h2);
    }
};

#endif /* hash_hpp */
