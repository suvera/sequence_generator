/* ---------------------------------------------------------------------------
** see LICENSE.md
**
** Sequence.h
**  Sequence class header
**
** Author: rnarmala
** -------------------------------------------------------------------------*/
#ifndef _SEQUENCE_HPP
#define _SEQUENCE_HPP

#define SEQUENCE_START_FROM 0

typedef std::mutex Mutex;
typedef std::timed_mutex TtlMutex;

extern Mutex newSeqMtx;
Mutex newSeqMtx;

extern Mutex UUIDMtx;
Mutex UUIDMtx;

// Sequence
class Sequence {
protected:

    string id;

    uHugeInt value = SEQUENCE_START_FROM;

    Mutex mtx;

    // Methods
    uHugeInt _nextVal();

public:

    // Constructors
    Sequence();
    
    Sequence(uHugeInt val);
    
    ~Sequence();

    // Methods
    uHugeInt nextVal();

    string getId() const;
    
    uHugeInt getVal() const;

};

void addNewSequence(const string &key, uHugeInt val);

void getUUID(char *uuid);

#endif