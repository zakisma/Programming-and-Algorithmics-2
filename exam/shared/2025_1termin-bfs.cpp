#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>

using namespace std;

class CTransport {
public:
    // konstruktor
    // destruktor

    CTransport &addLine(std::istream &is) {
        string line;
        vector<string> vec;
        while (getline(is, line)) {
            vec.emplace_back(line);
        }
        for (size_t i = 0; i < vec.size(); i++) {
            if ((i + 1) != vec.size()) {
                connections[vec[i]].insert(vec[i + 1]);
                connections[vec[i + 1]].insert(vec[i]);
            }
            allStations.insert(vec[i]);
        }
        return *this;
    }

    std::set<std::string> meetingPoints(
            const std::vector<std::string> &fromList, int &sumCost
    ) const {
        set<string> res, tempRes;
        int sumCostCpy = sumCost;
        unordered_map<string, size_t> fromMap;
        for (const auto &x: fromList) {
            fromMap[x]++;
        }
        for (const auto &start: allStations) {
            set<string> visited;
            queue<pair<string, int>> que;
            size_t desSize = 0;
            int tempSumCost = 0;

            que.push({start, 0});
            visited.insert(start);

            while (!que.empty()) {
                auto curStantion = que.front().first;
                auto curCost = que.front().second;
                que.pop();

                if (fromMap.contains(curStantion)) {
                    desSize += fromMap.at(curStantion);
                    tempSumCost += curCost * fromMap.at(curStantion);
                    if (desSize == fromList.size()) {
                        if (tempRes.empty() || (!tempRes.empty() && tempSumCost <= sumCostCpy)) {
                            if (sumCostCpy == tempSumCost) {
                                tempRes.insert(start);
                                sumCostCpy = tempSumCost;
                            } else {
                                tempRes = {};
                                tempRes.insert(start);
                                sumCostCpy = tempSumCost;
                            }
                        }
                        break;
                    }
                }
                if (!connections.contains(curStantion)) {
                    continue;
                }
                for (const auto &neigh: connections.at(curStantion)) {
                    if (!visited.contains(neigh)) {
                        visited.insert(neigh);
                        que.push({neigh, curCost + 1});
                    }
                }
            }

        }
        if (!tempRes.empty()) {
            sumCost = sumCostCpy;
        } else if (fromMap.size() == 1 && !connections.contains(fromList[0])) {
            sumCost = 0;
            tempRes.insert(fromList[0]);
        }
        return tempRes;
//        return res;
    }

private:
    unordered_map<string, set<string>> connections;
    set<string> allStations;
};

int main() {
    CTransport t;
    std::istringstream iss;
    iss.clear();
    iss.str(
            "Newton\n"
            "Black Hill\n"
            "Wood Side\n"
            "Green Hill\n"
            "Lakeside\n"
            "Newton\n"
    );
    t.addLine(iss);
    iss.clear();
    iss.str(
            "Wood Side\n"
            "Green Hill\n"
            "Little Burnside\n"
            "Great Newton\n"
            "Wood Side\n"
    );
    t.addLine(iss);
    iss.clear();
    iss.str(
            "Little Newton\n"
            "Little Burnside\n"
            "Castle Hill\n"
            "Newton Crossroad\n"
            "Lakeside Central\n"
            "Little Newton\n"
    );
    t.addLine(iss);
    iss.clear();
    iss.str(
            "Lakeside Central\n"
            "Little Waterton\n"
    );
    t.addLine(iss);
    iss.clear();
    iss.str(
            "Little Waterton\n"
            "Waterton West\n"
            "Waterton Central\n"
            "Waterton East\n"
            "Waterton Woods\n"
            "Waterton North\n"
            "Waterton East\n"
            "Little Waterton\n"
    );
    t.addLine(iss);
    iss.clear();
    iss.str(
            "Tidmouth\n"
            "Gordon's Hill\n"
            "Suderry\n"
            "Knapford\n"
            "Great Waterton\n"
            "Brendam Docks\n"
            "Tidmouth\n"
    );
    t.addLine(iss);
    iss.clear();
    iss.str(
            "Sodor Ironworks\n"
            "Sodor Steamwork\n"
            "Knapford\n"
            "Maron\n"
            "Gordon's Hill\n"
            "Sodor Ironworks\n"
    );
    t.addLine(iss);

    int cost;
    assert(
            (t.meetingPoints({"Newton"}, cost) ==
             std::set<std::string>{"Newton"}) &&
            (cost == 0)
    );

    assert(
            (t.meetingPoints({"Newton", "Newton"}, cost) ==
             std::set<std::string>{"Newton"}) &&
            (cost == 0)
    );

    assert(
            (t.meetingPoints({"Brno"}, cost) == std::set<std::string>{"Brno"}
            ) &&
            (cost == 0)
    );

    assert(
            (t.meetingPoints({"Brno", "Brno"}, cost) ==
             std::set<std::string>{"Brno"}) &&
            (cost == 0)
    );

    assert(
            (t.meetingPoints({"Newton", "Brno"}, cost) ==
             std::set<std::string>{})
    );

    assert(
            (t.meetingPoints({"Newton", "Black Hill"}, cost) ==
             std::set<std::string>{"Newton", "Black Hill"}) &&
            cost == 1
    );

    assert(
            (t.meetingPoints({"Newton", "Newton", "Black Hill"}, cost) ==
             std::set<std::string>{"Newton"}) &&
            cost == 1
    );

    assert(
            (t.meetingPoints({"Newton", "Black Hill", "Wood Side"}, cost) ==
             std::set<std::string>{"Black Hill"}) &&
            cost == 2
    );

    assert(
            (t.meetingPoints({"Newton Crossroad", "Little Waterton"}, cost) ==
             std::set<std::string>{
                     "Newton Crossroad", "Little Waterton", "Lakeside Central"
             }) &&
            cost == 2
    );

    assert(
            (t.meetingPoints({"Suddery", "Little Waterton"}, cost) ==
             std::set<std::string>{})
    );

    assert(
            (t.meetingPoints({"Tidmouth", "Newton Crossroad"}, cost) ==
             std::set<std::string>{})
    );

    iss.clear();
    iss.str(
            "Newton\n"
            "Newton Crossroad\n"
            "Tidmouth\n"
            "Newton\n"
    );
    t.addLine(iss);

    assert(
            (t.meetingPoints({"Tidmouth", "Newton Crossroad"}, cost) ==
             std::set<std::string>{"Tidmouth", "Newton Crossroad"}) &&
            cost == 1
    );


    CTransport t2;
    std::istringstream iss2;
    iss2.clear();
    iss2.str(
            "A\n"
            "B\n"
    );
    t2.addLine(iss2);
    iss2.clear();
    iss2.str(
            "C\n"
            "D\n"
    );
    t2.addLine(iss2);

    assert(t2.meetingPoints({"A", "C"}, cost) == set<string>{});
}

