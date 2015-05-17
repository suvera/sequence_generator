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

typedef std::atomic<uHugeInt> uHugeIntAtomic;

// Sequence
class Sequence {
protected:

    string id;

    // Methods
    uHugeIntAtomic _nextVal();

public:
    uHugeIntAtomic value;
        
    // Constructors
    Sequence();
    
    Sequence(uHugeInt val);
    Sequence(const Sequence&);
    
    ~Sequence();

    // Methods
    string getId() const;
};

void addNewSequence(const string &key, uHugeInt val);

void getUUID(char *uuid);

#endif