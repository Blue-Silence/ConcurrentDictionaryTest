#include <folly/concurrency/ConcurrentHashMap.h>
#include <folly/ConcurrentSkipList.h>
#include <iostream>
#include <string>
#include <map>
#include <shared_mutex>
#include <mutex>
#include <memory>

#include <thread>
#include <filesystem>

#include <algorithm>

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
        m.count(t);
        l.unlock_shared();
    }
    virtual void write(int64_t t)
    {
        l.lock();
        m[t]=true;
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
    vector<int> infos;
    std::mutex mu;
};

void testThreads(vector<vector<Action> *> &actsSeqs, CM *m, size_t num, LogOut &log)
{
    vector<std::thread *> ts;
    if (actsSeqs.size() < num)
        for (auto roiS = actsSeqs.size(), s = actsSeqs.size(); s < num; s++)
        {
            auto ns = new vector<Action>{};
            *ns = *(actsSeqs[s % roiS]);
            actsSeqs.push_back(ns);
        }
    log.infos = vector<int>(num, 0);

    size_t opNumAll = 0;
    for (auto p : actsSeqs)
        opNumAll += p->size();

    for (size_t i = 0; i < num; i++)
    {
        auto as = actsSeqs[i];
        auto p = new std::thread{[as, m, i, &log]()
                                 {
                                     auto startT = high_resolution_clock::now();
                                     testT(as, m);
                                     auto endT = high_resolution_clock::now();
                                     log.mu.lock();
                                     auto t = duration_cast<milliseconds>(endT - startT).count();
                                     //log.logFile << "Thread " << i << "  " << t << "\n";
                                     log.infos[i] = (int)t;
                                     //log.logFile.flush();
                                     log.mu.unlock();
                                 }};

        // p.detach();
        ts.push_back(p);
    }

    for (auto p : ts)
    {
        p->join();
        delete p;
    }

    size_t totalTime = 0;
    for (auto n : log.infos)
        totalTime += (size_t)n;

    log.logFile << "Total ops:" << opNumAll << " ";
    log.logFile << "Total ms:" << totalTime << " ";
    log.logFile << "Ops per ms:" << ((double)opNumAll / ((double)totalTime)) << " ";

    std::sort(log.infos.begin(), log.infos.end());
    log.logFile << "Shortest exec ms:" << log.infos[0] << " ";
    log.logFile << "Longest exec ms:" << log.infos[log.infos.size() - 1] << " ";
    log.logFile << "Overall ops per ms:" << ((double)opNumAll / ((double)log.infos[log.infos.size() - 1])) << "\n";

    log.logFile.flush();
    std::cout << "OKOK\n";
}

using namespace std;

int main(int argc, char **argv)
{

    if (argc != 4)
    {
        std::cout << "Usage: program MapType[RBM|HashM|SkipL] ThreadNumber TraceFileName  \n";
        return 0;
    }

    string mtype = string{argv[1]};
    CM *m = (mtype == "RBM" ? (CM *)new RBM{} : mtype == "HashM" ? (CM *)new HashM{}
                                                                 : (CM *)new SkipL{});

    string traceF = string{argv[3]}; //"symAccess-10-10-0-10000-128-10000";
    int num = std::atoi(argv[2]);
    
    auto r = getActSeqs(traceF);
    auto log = new LogOut{};
    auto fileN = string{"TestResult-MTYPE="} + mtype + "-TNUM" + std::to_string(num) + "-TRACE=" + std::filesystem::path(traceF).filename().string();
    log->logFile.open(fileN, std::ios::out | std::ios::in | std::ios::app);// | std::ios::trunc);
    //log->logFile << (fileN) << "\n";
    log->logFile.flush();

    testThreads(r, m, num, *log);

    return 0;
}
