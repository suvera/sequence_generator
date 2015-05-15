#include <Sequence.h>

/**
 * try to lock for a few seconds
 */
bool Sequence::tryLock() const {
    //return mtx.try_lock_until(system_clock::from_time_t(time(NULL) + MUTEX_TTL_MAX));
    return true;
}

/**
 * return Next value in this sequence
 * 0 means failed
 */
uHugeInt Sequence::nextVal() {
    int sl = 0, slc = 0;

    while (sl < MAX_WAIT_TIME) {

        if (this->tryLock()) {

            return _nextVal();

        } else {

            slc = rand() % 100 + 1;
            sl += slc;
            sleep_for(milliseconds(slc));
        }
    }

    return 0;
}

/**
 * return Next value in this sequence
 */
uHugeInt Sequence::_nextVal() {
    ++value;

    return value;
}


string Sequence::getId() const {
    return id;
}
