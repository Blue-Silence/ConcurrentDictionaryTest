#include "Trace.h"

#include <vector>
#include <map>
#include <string>

using std::map;
using std::string;
using std::vector;


#include <iostream>
#include <string>
#include <fstream>

using std::fstream;
using std::cin;
using std::cout;

#include <cstdio>


vector<vector<Action>*> getActSeqs(string fileN)
{
    fstream myFile;
    myFile.open(fileN, std::ios::out | std::ios::in);

    auto re = vector<vector<Action>*>{};

    for(;;)
    {
        if(myFile.eof())
            break;
        //myFile.ignore(65536,'\n');
        auto cV = new vector<Action>{};
        for(;;)
        {
            if(myFile.eof())
                break;
            string s;
            int64_t t;
            myFile >> s >> t;
            if(s=="Thread" || !myFile.good())
            {
                myFile.clear();
                myFile.ignore(65536,'\n');
                break;
            }
            cV->push_back({s=="READ"?READ : s=="WRITE"?WRITE :DELETE,t});
        }
        re.push_back(cV);
    }
    re.erase(re.begin());
    return re;

}

