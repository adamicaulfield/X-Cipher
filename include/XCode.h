//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#ifndef PANTHER_XCODE_H
#define PANTHER_XCODE_H

#include "Codes.h"
#include <helib/helib.h>
#include "Encryptor.h"

#define mod(a, b) (a % b + b) % b

namespace Panther {
    /*
    * Abstract class for column-major XCode tables
    */
    template <class T>
    class XCodeTableBase {
    public:
        XCodeTableBase(int, int);

        void assign(int, int, T);
        T get(int, int);
        void print();
        void swtichLastTwoRows(int, int);

        int rows() { return _rows; }
        int columns() { return _columns; }

    protected:
        std::vector<std::vector<T>> xcodeTable;

    private:
        int _rows;
        int _columns;
    };

    /*
     * XCodeLabelTable
     */
    template <class T>
    class XCodeLabelTable : public XCodeTableBase<T> {
    public:
        XCodeLabelTable(int, int);
    };
    template class Panther::XCodeLabelTable<std::string>;

    /*
     * XCodeDataTable
     */
    template <class T>
    class XCodeDataTable : public XCodeTableBase<T> {
    public:
        XCodeDataTable(int, int);
    };
    template class Panther::XCodeDataTable<bool>;

    enum Direction {forward, backward};

    /*
     * XCode
     */
    template <class T>
    class XCode : public Codes<T> {
    public:
        XCode(int, int, int);

        Panther::XCodeLabelTable<std::string> *getPart1LabelTable();
        Panther::XCodeLabelTable<std::string> *getPart2LabelTable();
        void printPart1LabelTable();
        void printPart2LabelTable();
        void encode(std::vector<Panther::Shard<T>> &, Panther::Dataset<T> &) override;
        std::vector<helib::Ctxt*> verify(Panther::Encryptor &, std::vector<Panther::Shard<T>> &, std::vector<helib::Ctxt*> &);
        void recoverFromOneColumnErasure(Panther::Encryptor &, std::vector<Panther::Shard<T>> &, int);
        void recoverFromTwoColumnErasure(Panther::Encryptor &, std::vector<Panther::Shard<T>> &, int, int);
        void rotateColumns(Panther::Encryptor &,std::vector<Panther::Shard<T>> &, Direction);
        void refreshTags(Panther::Encryptor &, std::vector<Panther::Shard<T>> &);
        std::vector<int> findLabel(std::string, int);
        helib::Ctxt integrityKeyGen(Panther::Encryptor &);
        helib::Ctxt integrityTagGen(Panther::Encryptor &, std::vector<Panther::Shard<T>> &, helib::Ctxt);
        bool verify2(Panther::Encryptor &, helib::Ctxt, helib::Ctxt);
        bool simpleVerify(Panther::Encryptor &, std::vector<Panther::Shard<T>> &);
        helib::Ctxt convolution(Panther::Encryptor &, std::vector<Panther::Shard<T>> &, int, int);
        helib::Ctxt summation(Panther::Encryptor &, std::vector<Panther::Shard<T>> &);
        helib::Ctxt dotProd(Panther::Encryptor &, helib::Ctxt, helib::Ctxt);
        std::vector<helib::Ctxt> matrixAdd(Panther::Encryptor &, std::vector<Panther::Shard<T>> &, std::vector<Panther::Shard<T>> &);
        helib::Ctxt matrixMult(Panther::Encryptor &, std::vector<Panther::Shard<T>> &, std::vector<Panther::Shard<T>> &, int);

    private:
        Panther::XCodeLabelTable<std::string> *part_1_labelTable;
        Panther::XCodeLabelTable<std::string> *part_2_labelTable;
        int n; //XCode configuration: a signle matrix is nxn
        int mults; //Number of multiples: extended version has n columns and n*mults rows
        int p; //plaintext prime modulus
        helib::Ctxt *macCtxt;
        Dataset<T> *mac_v1;
        std::vector<long> mac_v2;
    };

}


#endif //PANTHER_XCODE_H