#include <folly/concurrency/ConcurrentHashMap.h>
#include <folly/ConcurrentSkipList.h>
#include <iostream>
#include <string>
#include <map>
#include <shared_mutex>
#include <mutex>
#include <memory>

#include <thread>

#include "Trace.h"

#include <chrono>
using namespace std::chrono;

#include <fstream>
using std::fstream;

using std::string;

class CM
{
public:
    virtual void read(int64_t) = 0;
    virtual void write(int64_t) = 0;
    virtual void del(int64_t) = 0;
};

class RBM : public CM
{
    std::map<int64_t, bool> m;
    std::shared_mutex l;

public:
    virtual void read(int64_t t)
    {
        l.lock_shared();
        m[t];
        l.unlock_shared();
    }
    virtual void write(int64_t t)
    {
        l.lock();
        m.insert_or_assign(t, true);
        l.unlock();
    }
    virtual void del(int64_t t)
    {
        l.lock();
        m.erase(t);
        l.unlock();
    }
};

class HashM : public CM
{
    folly::ConcurrentHashMap<int64_t, bool> m;

public:
    virtual void read(int64_t t) { m[t]; }
    virtual void write(int64_t t) { m.insert_or_assign(t, true); }
    virtual void del(int64_t t) { m.erase(t); }
};

class SkipL : public CM
{
    typedef folly::ConcurrentSkipList<int64_t> SkipListT;
    std::shared_ptr<SkipListT> sl = (SkipListT::createInstance(6));
    // folly::ConcurrentSkipList<int64_t, bool> m;

public:
    virtual void read(int64_t t)
    {
        SkipListT::Accessor accessor(sl);
        accessor.find(t);
    }
    virtual void write(int64_t t)
    {
        SkipListT::Accessor accessor(sl);
        accessor.insert(t);
    }
    virtual void del(int64_t t)
    {
        SkipListT::Accessor accessor(sl);
        accessor.erase(t);
    }
};

void testT(vector<Action> *seq, CM *m)
{
    for (const auto &a : *seq)
    {
        switch (a.act)
        {
        case READ:
            m->read(a.addr);
            break;
        case WRITE:
            m->write(a.addr);
            break;
        case DELETE:
            m->del(a.addr);
            break;
        }
    }
}

struct LogOut
{
    fstream logFile;
    std::mutex mu;
};

void testThreads(const vector<vector<Action> *> &actsSeqs, CM *m, size_t num, LogOut &log)
{
    vector<std::thread*> ts;
    for (size_t i = 0; i < num; i++)
    {
        auto as = actsSeqs[i];
        auto p = new std::thread{[as, m, i, &log]()
                             {
                                 
                                 auto startT = high_resolution_clock::now();
                                 testT(as, m);
                                 auto endT = high_resolution_clock::now();
                                 log.mu.lock();
                                 log.logFile << "Thread " << i << "  " << duration_cast<milliseconds>(endT - startT).count() << "\n";
                                 log.logFile.flush();
                                 log.mu.unlock();
                                 std::cout << "Fuck!\n";
                             }};
        
        //p.detach();
        ts.push_back(p);
    }

    for(auto p:ts)
    {
        p->join();
        delete p;
    }

}

using namespace std;

int main()
{
    string traceF = "symAccess-10-10-0-10000-128-10000";
    auto r = getActSeqs(traceF);
    auto log = new LogOut{};
    fstream foo;
    foo.open("Hell no", std::ios::out | std::ios::in | std::ios::trunc);
    log->logFile.open(string{"TestResult-"} + traceF, std::ios::out | std::ios::in | std::ios::trunc);
    log->logFile << (string{"TestResult-"} + traceF) << "\n";
    log->logFile.flush();
    foo << (string{"TestResult-"} + traceF) << "\n";
    auto m = new SkipL{};
    testThreads(r, m, 128, *log);

    return 0;
}
