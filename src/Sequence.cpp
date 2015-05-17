/* ---------------------------------------------------------------------------
** see LICENSE.md
**
** Sequence.cpp
**  Sequence implementation
**
** Author: rnarmala
** -------------------------------------------------------------------------*/
#include <Sequence.h>


string Sequence::getId() const {
    return id;
}

Sequence::Sequence() {
    value = SEQUENCE_START_FROM;
}

Sequence::Sequence(uHugeInt val) {
    value = val;
}

Sequence::~Sequence() {
}

Sequence::Sequence(const Sequence&) {
}


// add a new Sequence
void addNewSequence(const string &key, uHugeInt val) {
    std::lock_guard<Mutex> lk(newSeqMtx);
    
    counters[key] = new Sequence(val);
}

// UUIDMtx
void getUUID(char *uuid) {
    std::lock_guard<Mutex> lk(UUIDMtx);
    
    uuid_t newUUID;
    uuid_generate(newUUID);
    
    uuid_unparse_lower(newUUID, uuid);
}

