#ifndef __PROGTEST__

#include <algorithm>
#include <assert.h>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#endif /* __PROGTEST__ */
using namespace std;
class CSpread {
private:
    std::unordered_map<std::string, std::set<std::string>> friendships;
public:
    // Default constructor
    CSpread() = default;

    // Add friendship between two people
    CSpread &addfriend(const std::string &p1, const std::string &p2) {
        friendships[p1].insert(p2);
        friendships[p2].insert(p1);
        return *this;
    }

    // For compatibility with test cases
    CSpread &addFriend(const std::string &p1, const std::string &p2) {
        return addfriend(p1, p2);
    }

    CSpread &optimize() {
        return *this;
    }

    std::map<std::string, int> simulate(const std::map<std::string, std::string> &spreaders) {
        map<string, pair<string, int>> cost; // person -> {conspiracy, distance}
        set<string> processed; // people who have been completely processed
        unordered_set<string> immune; // people who are immune
        map<string, int> result; // conspiracy -> count of believers
        queue<string> q;

        // Initialize with the initial spreaders
        for (const auto &[person, conspiracy] : spreaders) {
            cost[person] = {conspiracy, 0};
            result[conspiracy]++;
            q.push(person);
        }

        // BFS to spread conspiracies
        while (!q.empty()) {
            string current = q.front();
            q.pop();

            // Mark as processed
            processed.insert(current);

            const string &currentConspiracy = cost[current].first;
            int currentDistance = cost[current].second;

            // Spread to all friends
            for (const auto &friend_name : friendships[current]) {
                // Skip immune people
                if (immune.count(friend_name)) {
                    continue;
                }

                int newDistance = currentDistance + 1;

                // Case 1: Friend hasn't heard any conspiracy OR would hear this one sooner
                if (!cost.count(friend_name) || cost[friend_name].second > newDistance) {
                    // If they already had a conspiracy, decrement that counter
                    if (cost.count(friend_name)) {
                        result[cost[friend_name].first]--;
                    }

                    cost[friend_name] = {currentConspiracy, newDistance};
                    result[currentConspiracy]++;

                    // Only add to the queue if not fully processed yet
                    if (!processed.count(friend_name)) {
                        q.push(friend_name);
                    }

                    // No longer immune
                    immune.erase(friend_name);
                }
                    // Case 2: Heard a different conspiracy at the same distance, becomes immune
                else if (cost[friend_name].second == newDistance &&
                         cost[friend_name].first != currentConspiracy) {

                    // Only become immune if not already processed
                    if (!processed.count(friend_name)) {
                        // Remove from believers
                        result[cost[friend_name].first]--;

                        // Mark as immune
                        immune.insert(friend_name);
                    }
                }
                // Case 3: Already heard a conspiracy earlier - no effect
            }
        }

        return result;
    }

    // For compatibility with the first implementation
    std::map<std::string, int> findCoverage(const std::map<std::string, std::string> &spreaders) {
        return simulate(spreaders);
    }
};



#ifndef __PROGTEST__

int main() {
    CSpread x0;
    x0.addfriend("Dave", "Eve");
    x0.addfriend("Dave", "John");
    x0.addfriend("Eve", "Peter");
    x0.addfriend("John", "Peter");
    x0.addfriend("Peter", "Rob");
    x0.addfriend("Peter", "Mark");
    x0.addfriend("Rob", "Lily");
    x0.addfriend("Rob", "Dave");
    x0.addfriend("Rob", "Eve");
    x0.addfriend("Mark", "Lily");
    x0.optimize();

    assert(x0.simulate({{"Dave", "chemtrails"}}) == (std::map<std::string, int>{{"chemtrails", 7}}));

    assert(x0.simulate({{"Dave", "chemtrails"},
                        {"Eve",  "chemtrails"}}) ==
           (std::map<std::string, int>{{"chemtrails", 7}}));

    assert(x0.simulate({{"Lily", "chemtrails"}}) == (std::map<std::string, int>{{"chemtrails", 7}}));

    assert(x0.simulate({{"Dave", "chemtrails"},
                        {"John", "moon-landing"}}) == (std::map<std::string, int>{{"chemtrails",   4},
                                                                                  {"moon-landing", 3}}));
    // Dave - chemtrails, John - moon, Eve - chemtrails, Peter - moon, Rob - chemtrails, Mark - moon, Lily - chemtrails


    CSpread x1;
    x1.addfriend("Dave", "Eve");
    x1.addfriend("Dave", "John");
    x1.addfriend("Eve", "Peter");
    x1.addfriend("Eve", "John");
    x1.addfriend("Peter", "Rob");
    x1.addfriend("Peter", "Mark");
    x1.addfriend("Rob", "Lily");
    x1.addfriend("Rob", "Dave");
    x1.addfriend("Rob", "Eve");
    x1.addfriend("Mark", "Lily");
    x1.optimize();

    assert(x1.simulate({{"Dave", "chemtrails"},
                        {"Eve",  "moon-landing"}}) == (std::map<std::string, int>{{"chemtrails",   1},
                                                                                  {"moon-landing", 4}}));
    // Dave - chemtrails, Eve - moon,  Peter - moon, Mark - moon, Lily - moon, Rob - immune, John - immune


}

#endif /* __PROGTEST__ */

