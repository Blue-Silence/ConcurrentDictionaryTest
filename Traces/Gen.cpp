#include <iostream>
#include <string>
#include <fstream>

#include <cstdlib>

#include <cstddef>
#include <cstdint>

#include <string>
#include <vector>
#include <map>
#include <set>

#include <filesystem>

using std::map;
using std::set;
using std::string;
using std::vector;

using std::cin;
using std::cout;
using std::fstream;

#define THREAD_HEAR_TAG(X) (string{"Thread "} + std::to_string(X) + string{" trace:"})
#define ACTION(ACT, ADD) (ACT + string{" "} + std::to_string(ADD))

/*
enum
{
    RE,
    WR,
    DEL
};

string toACT(int t)
{
    switch (t)
    {
    case RE:
        return "READ";
    case WR:
        return "WRITE";
    case DEL:
        return "DELETE";
    }
}
*/

string actionGen(unsigned readR, unsigned writeR, unsigned delR)
{ // Choose an random action according to the ratio
    auto p = rand() % (readR + writeR + delR);
    if (p < readR)
        return "READ";
    else if (p < readR + writeR)
        return "WRITE";
    else
        return "DELETE";
}

int64_t rand64()
{ // Generate int64 random number;
    return (int64_t)(((uint64_t)rand() << 32) | (uint64_t)rand());
}

vector<int64_t>* addressGen(size_t num)
{ // Generate target addresses to operate on according to the giving number
    std::set<int64_t> s;
    auto re = new vector<int64_t>{};
    while (s.size() < num)
    {
        auto r = rand64();
        if (s.count(r) == 0)
        {
            s.insert(r);
            re->push_back(r);
        }
    }
    return re;
}

void symAccess(unsigned readR, unsigned writeR, unsigned delR, size_t opPerThread, int threadNum, size_t addressNum)
{
    auto addrs = addressGen(addressNum);
    // Thread access in a sym way. Every thread can read and write random location.
    string fileN = string{ "symAccess" } + '-' + std::to_string(readR) + '-' + std::to_string(writeR) + '-' + std::to_string(delR) + '-' + std::to_string(opPerThread) + '-' + std::to_string(threadNum) + '-' + std::to_string(addressNum);
    fstream myFile;
    myFile.open(fileN, std::ios::out | std::ios::in | std::ios::trunc);

    for (size_t i = 0; i < threadNum; i++)
    {
        myFile << "\n\n\n";
        myFile << THREAD_HEAR_TAG(i) << "\n";
        for (size_t j = 0; j < opPerThread; j++)
            myFile << ACTION(actionGen(readR, writeR, delR), (*addrs)[(rand64() % addrs->size())]) << "\n";
    }

}

                                                                                                                                                      


int main() {
    symAccess(1, 1, 1, 10000, 128, 10000);
}
