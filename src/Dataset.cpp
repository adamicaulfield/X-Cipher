//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#include "Dataset.h"
#include <iostream>
#include "util.h"

template <class T>
Panther::Dataset<T>::Dataset(int rows, int cols, T val) : _rows(rows), _columns(cols) {
    for (int i = 0; i < rows; ++i) {
        std::vector<T> data(cols, val);
        dataset.push_back(data);
    }
}

template <class T>
Panther::Dataset<T>::Dataset(int rows, int cols) : _rows(rows), _columns(cols) {
    for (int i = 0; i < rows; ++i) {
        std::vector<T> data(cols);
        dataset.push_back(data);
    }
}

template <class T>
T
Panther::Dataset<T>::get(int rowID, int colID) {
    return dataset[rowID][colID];
}

template <class T>
void
Panther::Dataset<T>::assign(int rowID, int colID, T val) {
    dataset[rowID][colID] = val;
}

template <class T>
void
Panther::Dataset<T>::print() {
    for (int r = 0; r < _rows; ++r) {
        for (int c = 0; c < _columns; ++c) {
            std::cout << dataset[r][c] << " ";
        }
        std::cout << std::endl;
    }
}

template <class T>
void
Panther::Dataset<T>::swtichLastTwoRows() {
    std::vector<T> tmp(dataset[_rows-2]);
    dataset[_rows-2]=dataset[_rows-1];
    dataset[_rows-1]=tmp;
}


template class Panther::Dataset<bool>;
template class Panther::Dataset<int>;