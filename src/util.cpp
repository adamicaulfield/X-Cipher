//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#include "util.h"
#include <iomanip>
#include <iostream>
#include <iterator>

std::string
util::to_upper(const std::string &s) {
    std::string t;
    for (auto it = s.cbegin(); it != s.cend(); it++) t += toupper(*it);

    return t;
}

std::string
util::to_lower(const std::string &s) {
    std::string t;
    for (auto it = s.cbegin(); it != s.cend(); it++) t += tolower(*it);

    return t;
}

std::string
util::bitvec_to_string(const std::vector<bool> &val) {
    std::string s;
    for (auto bit : val) s += bit ? '1' : '0';

    return s;
}

std::vector<bool>
util::make_bitvec(int size, int pos) {
    std::vector<bool> res(size, 0);
    res[pos] = 1;
    return res;
}

void
util::debug(const std::string &s) {
#ifdef DEBUG
    std::cout << "[DEBUG] " << s << std::endl;
#endif
}

void
util::info(const std::string &s) {
#ifdef INFO
    std::cout << "[INFO] " << s << std::endl;
#endif
}

void
util::error(const std::string &s) {
    std::cerr << "[ERROR] " << s << std::endl;
}
