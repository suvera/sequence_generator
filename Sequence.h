#ifndef _SEQUENCE_HPP
#define _SEQUENCE_HPP


typedef unsigned long long uHugeInt;

typedef std::mutex Mutex;
typedef std::timed_mutex TtlMutex;

//in seconds
#define MUTEX_TTL_MAX 2
// in milli seconds
#define MAX_WAIT_TIME 1100


// Sequence
class Sequence {
protected:

    string id;

    uHugeInt value = 0;

    TtlMutex mtx;

    // Methods
    uHugeInt _nextVal();

public:

    // Methods
    uHugeInt nextVal();

    bool tryLock() const;

    string getId() const;
};

#endif