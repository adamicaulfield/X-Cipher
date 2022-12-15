//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#ifndef PANTHER_UTIL_H
#define PANTHER_UTIL_H

#include "panther.h"
#include <bitset>
#include <iostream>
#include <string>
#include <vector>

namespace util {
    std::string to_upper(const std::string &s);
    std::string to_lower(const std::string &s);

    template<size_t N>
    std::vector<bool> bitset_to_bitvec(const std::bitset<N> &bits) {
        std::vector<bool> res(N);
        for (size_t i = 0; i < N; i++) res[i] = bits[i];

        return res;
    }

    std::string bitvec_to_string(const std::vector<bool> &val);
    std::vector<bool> make_bitvec(int size, int pos);

    void debug(const std::string &s);
    void info(const std::string &s);
    void error(const std::string &s);
}

#endif /* PANTHER_UTIL_H */
