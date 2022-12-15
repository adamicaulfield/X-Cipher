//
// Copyright SpiRITlab - The Panther project.
// https://github.com/SpiRITlab/Panther
//

#include "Cache.h"

/*
 * CacheEntry
 */

Panther::CacheEntry::CacheEntry(int row, int col) : rowID(row), columnID(col) {
}

Panther::CacheEntry::CacheEntry(int row, int col, helib::Ctxt *ctxt) : rowID(row), columnID(col), ctxt(ctxt) {
}

int
Panther::CacheEntry::getRowID() {
    return this->rowID;
}

int
Panther::CacheEntry::getColumnID() {
    return this->columnID;
}

helib::Ctxt *
Panther::CacheEntry::getCtxt() {
    return this->ctxt;
}

void
Panther::CacheEntry::addCtxt(helib::Ctxt *ctxt) {
    this->ctxt = ctxt;
}


/*
 * Cache
 */
void
Panther::Cache::add(std::string label, CacheEntry *entry) {
    if (cache.find(label) == cache.end()) {
        cache[label] = entry;
    }
}

void
Panther::Cache::add(std::string label, int row, int column, helib::Ctxt *ctxt) {
    if (cache.find(label) == cache.end()) {
        cache[label] = new Panther::CacheEntry(row, column, ctxt);
    }
}

Panther::CacheEntry *
Panther::Cache::get(std::string label) {
    return cache.find(label)->second;
}

void
Panther::Cache::remove(std::string label) {
    if (cache.find(label) != cache.end()) {
        cache.erase(label);
    }
}

bool
Panther::Cache::contain(std::string label) {
    if (cache.find(label) == cache.end())
        return false;
    else
        return true;
}

void
Panther::Cache::print() {
    for (auto& x: cache) {
        std::cout << x.first << ": (" << x.second->getRowID() << ", " << x.second->getColumnID() << ", " << x.second->getCtxt() << ")" << std::endl;
    }
}