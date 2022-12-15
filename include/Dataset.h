//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#ifndef PANTHER_DATASET_H
#define PANTHER_DATASET_H

#include <vector>
#include <iostream>

namespace Panther {

    template <class T>
    class Dataset {
    public:
        Dataset(int, int, T);
        Dataset(int, int);

        int rows() { return _rows; }
        int columns() { return _columns; }

        T get(int, int);
        void assign(int, int, T);
        void print();
        void swtichLastTwoRows();

    private:
        int _rows;
        int _columns;
        std::vector<std::vector<T>> dataset;
    };

}

#endif //PANTHER_DATASET_H
