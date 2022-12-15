//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#ifndef PANTHER_SHARD_H
#define PANTHER_SHARD_H

#include <helib/helib.h>
#include "Encryptor.h"

#define CHUNKSIZE 100

namespace Panther {

    template <class T>
    class Shard {
    public:
        void setPart1(const std::vector<T> &);
        std::vector<T> getPart1Ptxt();
        void addToPart1(T);
        void printPart1();

        void setPart2(const std::vector<T> &);
        std::vector<T> getPart2Ptxt();
        void addToPart2(T);
        void printPart2();

        void setPart1(helib::Ctxt);
        helib::Ctxt *getPart1Ctxt();
        void setPart2(helib::Ctxt);
        helib::Ctxt *getPart2Ctxt();


        void encrypt(Panther::Encryptor &);

    private:
        std::vector<T> part1_ptxt;
        std::vector<T> part2_ptxt;
        helib::Ctxt *part1_ctxt;
        helib::Ctxt *part2_ctxt;

        void fill_plaintext(helib::Ptxt<helib::BGV> &, const std::vector<T> &);
        void encode(helib::Ptxt<helib::BGV> &, helib::Ptxt<helib::BGV> &);
    };

}

#endif //PANTHER_SHARD_H
