//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#ifndef PANTHER_CODES_H
#define PANTHER_CODES_H

#include <vector>
#include "Dataset.h"
#include "Shard.h"

namespace Panther {
    template <class T>
    class Codes {
    public:
        virtual void encode(std::vector<Panther::Shard<T>> &, Panther::Dataset<T> &)=0;
    };
}


#endif //PANTHER_CODES_H
