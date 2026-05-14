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

    void optimize() {
        return;
    }

    std::map<std::string, int> simulate(const std::map<std::string, std::string> &spreaders) {
        map<string, pair<string, int>> beliefs; // person -> {theory, day}
        set<string> immune;
        map<string, int> res; // theory -> count

        // Initialize spreaders (day 0)
        for (const auto &[person, theory]: spreaders) {
            beliefs[person] = {theory, 0};
            res[theory]++;
        }

        // Start day-by-day simulation
        int day = 1;
        bool changed = true;

        while (changed) {
            changed = false;

            // Track daily exposures
            map<string, set<string>> dailyExposures;

            // Collect exposures for this day
            for (const auto &[person, belief]: beliefs) {
                // Skip people who just started believing today (they spread tomorrow)
                if (belief.second == day) {
                    continue;
                }

                // Each believer spreads to their friends
                string theory = belief.first;
                for (const auto &friend_: friendships[person]) {
                    // Skip immune people and those who already believe something
                    if (immune.contains(friend_) ||
                        (beliefs.contains(friend_) && beliefs[friend_].second < day)) {
                        continue;
                    }

                    // Record exposure
                    dailyExposures[friend_].insert(theory);
                }
            }

            // Process daily exposures
            for (const auto &[person, theories]: dailyExposures) {
                // Multiple theories on same day -> immune
                if (theories.size() > 1) {
                    immune.insert(person);
                    // Remove existing belief if they were exposed earlier today
                    if (beliefs.contains(person) && beliefs[person].second == day) {
                        res[beliefs[person].first]--;
                        beliefs.erase(person);
                    }
                }
                    // Single theory and no prior belief -> adopt it
                else if (theories.size() == 1) {
                    string newTheory = *theories.begin();

                    // If they don't already have a belief from earlier days
                    if (!beliefs.contains(person) || beliefs[person].second >= day) {
                        // Set new belief
                        beliefs[person] = {newTheory, day};
                        res[newTheory]++;
                        changed = true;
                    }
                }
            }

            day++;
        }

        return res;
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

