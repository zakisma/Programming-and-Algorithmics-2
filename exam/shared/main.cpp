#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <list>
#include <deque>
#include <queue>
#include <stack>
#include <cassert>

using namespace std;
class CMHD
{
public:
    void Add(istringstream &is);
    set<string> Dest(const string &from, int maxCost) const;

private:
    map<string, set<string>> stations;
};

void CMHD::Add(istringstream &is)
{
    vector<string> stations_on_line;
    string station;
    while (getline(is, station))
        stations_on_line.push_back(station);
    for (auto &it1 : stations_on_line)
        for (auto &it2 : stations_on_line)
            stations[it1].insert(it2);
}

set<string> CMHD::Dest(const string &from, int maxCost) const
{
    set<string> res;
    if (stations.count(from) == 0)
        return res = {from};
    queue<pair<string, int>> next_visit;
    next_visit.push({from, 0});
    while (!next_visit.empty())
    {
        string station = next_visit.front().first;
        int cost = next_visit.front().second;
        next_visit.pop();
        if (res.count(station) == 1)
            continue;
        res.insert(station);
        if (cost > maxCost)
            continue;
        for (const auto &it : stations.at(station))
            next_visit.push({it, cost + 1});
    }
    return res;
}


int main(void) {
    CMHD city;
    istringstream iss;
    iss.clear();

    iss.str("A\nB\nC\nD\nE\n");
    city.Add(iss);
    iss.clear();

    iss.str("B\nC\nF\nH\n");
    city.Add(iss);
    iss.clear();

    iss.str("F\nG\nI\nJ\nK\nN\n");
    city.Add(iss);
    iss.clear();

    iss.str("H\nL\n");
    city.Add(iss);
    iss.clear();

    iss.str("L\nM\nN\nO\n");
    city.Add(iss);
    iss.clear();

    iss.str("P\nQ\nR\nN\nS");
    city.Add(iss);
    iss.clear();

    assert (city.Dest("S", 0) == set<string>({"S", "N", "R", "Q", "P"}));

    assert (city.Dest("N", 0) == set<string>({"S", "N", "R", "Q", "P",
                                              "O", "M", "L",
                                              "K", "J", "I", "G", "F"}));

    assert (city.Dest("S", 1) == set<string>({"S", "N", "R", "Q", "P",
                                              "O", "M", "L",
                                              "K", "J", "I", "G", "F"}));

    assert (city.Dest("N", 1) == set<string>({"S", "N", "R", "Q", "P",
                                              "O", "M", "L",
                                              "K", "J", "I", "G", "F",
                                              "H", "F", "C", "B"}));

    assert (city.Dest("N", 2) == set<string>({"S", "N", "R", "Q", "P",
                                              "O", "M", "L",
                                              "K", "J", "I", "G", "F",
                                              "H", "F", "C", "B",
                                              "A", "D", "E"}));

    assert (city.Dest("unknown", 0) == set<string>({"unknown"}));
    assert (city.Dest("unknown", 1) == set<string>({"unknown"}));
    assert (city.Dest("unknown", 2) == set<string>({"unknown"}));

    // Speed test
    CMHD circleCity;
    iss.clear();
    iss.str("A\nB\nC\n");
    circleCity.Add(iss);

    iss.clear();
    iss.str("C\nD\nA\n");
    circleCity.Add(iss);

    assert(circleCity.Dest("A", 1000) == set<string>({"A", "B", "C", "D"}));

    cout << "All asserts passed." << endl;
    return 0;
}