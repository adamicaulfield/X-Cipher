//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#include "Shard.h"
#include "util.h"
#include<vector>
#include<algorithm>
#include<iterator>

template <class T>
void
Panther::Shard<T>::setPart1(const std::vector<T> &values) {
    copy(values.begin(), values.end(), back_inserter(part1_ptxt));
}

template <class T>
std::vector<T>
Panther::Shard<T>::getPart1Ptxt() {
    return part1_ptxt;
}

template <class T>
void
Panther::Shard<T>::addToPart1(T value) {
    part1_ptxt.push_back(value);
}

template <class T>
void
Panther::Shard<T>::printPart1() {
    for (int i = 0; i < this->part1_ptxt.size(); ++i) {
        std::cout << this->part1_ptxt[i] << " ";
    }
    std::cout << std::endl;
}

template <class T>
void
Panther::Shard<T>::setPart2(const std::vector<T> & values) {
    copy(values.begin(), values.end(), back_inserter(part2_ptxt));
}

template <class T>
std::vector<T>
Panther::Shard<T>::getPart2Ptxt() {
    return this->part2_ptxt;
}

template <class T>
void
Panther::Shard<T>::addToPart2(T value) {
    part2_ptxt.push_back(value);
}

template <class T>
void
Panther::Shard<T>::printPart2() {
    for (int i = 0; i < this->part2_ptxt.size(); ++i) {
        std::cout << this->part2_ptxt[i] << " ";
    }
    std::cout << std::endl;
}

template <class T>
void
Panther::Shard<T>::setPart1(helib::Ctxt ctxt) {
    *(this->part1_ctxt) = ctxt;
}

template <class T>
helib::Ctxt *
Panther::Shard<T>::getPart1Ctxt() {
    return this->part1_ctxt;
}

template <class T>
void
Panther::Shard<T>::setPart2(helib::Ctxt ctxt) {
    *(this->part2_ctxt) = ctxt;
}

template <class T>
helib::Ctxt *
Panther::Shard<T>::getPart2Ctxt() {
    return this->part2_ctxt;
}

template <class T>
void
Panther::Shard<T>::fill_plaintext(helib::Ptxt<helib::BGV> &ptxt, const std::vector<T> &val) {
    int i;
#pragma omp parallel for shared(ptxt, val) private(i) schedule(static, CHUNKSIZE)
    for(i=0; i<val.size(); i++)
        ptxt[i] = val[i];
}

template <class T>
void
Panther::Shard<T>::encode(helib::Ptxt<helib::BGV> &part1_encoded_ptxt, helib::Ptxt<helib::BGV> &part2_encoded_ptxt) {
    fill_plaintext(part1_encoded_ptxt, this->part1_ptxt);
    fill_plaintext(part2_encoded_ptxt, this->part2_ptxt);
}

template <class T>
void
Panther::Shard<T>::encrypt(Panther::Encryptor &encryptor) {
    util::info("Encrypting a shard...");
    // pack and encrypt into one ciphertext
    helib::Ptxt<helib::BGV> part1_encoded_ptxt(*(encryptor.getContext()));
    helib::Ptxt<helib::BGV> part2_encoded_ptxt(*(encryptor.getContext()));

    encode(part1_encoded_ptxt, part2_encoded_ptxt);

    part1_ctxt = new helib::Ctxt(*(encryptor.getPublicKey()));
    part2_ctxt = new helib::Ctxt(*(encryptor.getPublicKey()));
    encryptor.getPublicKey()->Encrypt(*part1_ctxt, part1_encoded_ptxt);
    encryptor.getPublicKey()->Encrypt(*part2_ctxt, part2_encoded_ptxt);
}

template class Panther::Shard<bool>;
template class Panther::Shard<int>;