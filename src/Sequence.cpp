/* ---------------------------------------------------------------------------
** see LICENSE.md
**
** Sequence.cpp
**  Sequence implementation
**
** Author: rnarmala
** -------------------------------------------------------------------------*/

string Sequence::getId() const {
    return id;
}

Sequence::Sequence() {
    value = SEQUENCE_START_FROM;
    dirty = true;
    offset = 0;
}

Sequence::Sequence(uHugeInt val) {
    value = val;
    dirty = true;
    offset = 0;
}

Sequence::~Sequence() {
}

Sequence::Sequence(const Sequence&) {
}


// add a new Sequence
void addNewSequence(const string &key, uHugeInt val) {
    std::lock_guard<Mutex> lk(newSeqMtx);
    
    counters[key] = new Sequence(val);
    
    cout << "Created new, size now: " << counters.size() << "\n";
}

// UUIDMtx
void getUUID(char *uuid) {
    std::lock_guard<Mutex> lk(UUIDMtx);
    
    uuid_t newUUID;
    uuid_generate(newUUID);
    
    uuid_unparse_lower(newUUID, uuid);
}

