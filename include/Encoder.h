//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#ifndef PANTHER_ENCODER_H
#define PANTHER_ENCODER_H

#include <vector>
#include <iostream>
#include "Dataset.h"

#define mod(a, b) (a % b + b) % b

namespace Panther {

    class Encoder {
    public:
        Encoder();

        Panther::Dataset<bool> encodeImage(std::string, int);
        void decodeImage(std::string, Panther::Dataset<bool>);

        Panther::Dataset<bool> encodeChar(char, int);
        void decodeChar(Panther::Dataset<bool>);

        void encodeImageToInt(std::string);

        Panther::Dataset<bool> compressDataset(Panther::Dataset<bool>);
        Panther::Dataset<bool> expandDataset(Panther::Dataset<bool>);
        Panther::Dataset<int> compressDataset(Panther::Dataset<int>);
        Panther::Dataset<int> expandDataset(Panther::Dataset<int>);

        std::vector<int> getPSIParams(int, int);
        Panther::Dataset<int> encodePSI(std::vector<int>, std::vector<int>, int, int, int, int, int);
        Panther::Dataset<int> getCombinations(int, int, int);
        int n_chose_k(int, int);
        std::vector<int> getSenderOptCoeffs(int, std::vector<int>, int);
        int modPow(int , int , int);

        Panther::Dataset<int> encodeMatrix(Panther::Dataset<int>);
    private:
    	int originalSize;
        int height;
        int length;
        std::string header;
    };
}

#endif //PANTHER_ENCODER_H
