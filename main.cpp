// timed_mutex::try_lock_until example
#include <cstdlib>
#include <iostream>       // std::cout
#include <chrono>         // std::chrono::system_clock
#include <thread>         // std::thread
#include <mutex>          // std::timed_mutex
#include <ctime>          // std::time_t, std::tm, std::localtime, std::mktime
#include <unordered_map>
#include <ctime>

using std::cout;
using std::endl;
using std::string;
using std::unordered_map;
using namespace std::chrono;
using std::chrono::system_clock;
using namespace std::this_thread;

#include <Sequence.cpp>

extern unordered_map<string, Sequence> counters;
unordered_map<string, Sequence> counters;

int main ()
{
    srand(time(NULL));

    cout << "Success\n";

    return 0;
}