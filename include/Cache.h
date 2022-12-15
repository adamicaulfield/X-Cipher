//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#ifndef PANTHER_CACHE_H
#define PANTHER_CACHE_H

#include <helib/helib.h>
#include <unordered_map>
#include <string>

namespace Panther {

    class CacheEntry {
    public:
        CacheEntry(int, int);
        CacheEntry(int, int, helib::Ctxt *);

        int getRowID();
        int getColumnID();
        helib::Ctxt *getCtxt();
        void addCtxt(helib::Ctxt *);
    private:
        int rowID;
        int columnID;
        helib::Ctxt *ctxt;
    };

    class Cache {
    public:
        void add(std::string, CacheEntry *);
        void add(std::string, int, int, helib::Ctxt *);
        CacheEntry *get(std::string);
        void remove(std::string);
        bool contain(std::string);
        void print();
    private:
        std::unordered_map<std::string, CacheEntry *> cache;
        std::unordered_map<std::string, CacheEntry *>::const_iterator found;
    };

}


#endif //PANTHER_CACHE_H
