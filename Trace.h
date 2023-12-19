#pragma once

#include <cstdint>
#include <vector>
#include <string>

const char READ = 0;
const char WRITE = 1;
const char DELETE = 2;

struct Action {
    char act;
    int64_t addr;
};


using std::vector;
using std::string;

vector<vector<Action>*> getActSeqs(string fileN);