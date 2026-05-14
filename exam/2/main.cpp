#include<iostream>
#include<cassert>
#include<vector>
#include<set>
#include<map>
#include<string>
#include<unordered_map>
#include<memory>
#include<unordered_set>
#include<queue>
#include<list>
#include<sstream>
#include<cstring>

using namespace std;

class CPkg {
public:
    //TODO constructor
    //TODO addDep(string)
    CPkg(const string &name) : pckg_name(name) {}

    CPkg &addDepend(const string &depName) {
        deps.insert(depName);
        return *this;
    }

    CPkg &addRecommend(const string &recName) {
        recs.insert(recName);
        return *this;
    }

    bool isInDeps(const string &dep) const {
        auto it = deps.find(dep);
        if (it == deps.end()) {
            return false;
        }
        return true;
    }

    bool isInRecs(const string &dep) const {
        auto it = recs.find(dep);
        if (it == recs.end()) {
            return false;
        }
        return true;
    }

    set<string> getDeps() const {
        return deps;
    }

    set<string> getRecs() const {
        return recs;
    }

    string getName() const {
        return pckg_name;
    }

private:
    string pckg_name;
    set<string> deps;
    set<string> recs;
};

class CPkgSys {
public:
    CPkgSys() = default;

    CPkgSys &addPkg(const CPkg &cpkg) {
        string name = cpkg.getName();
        set temp_deps = cpkg.getDeps();
        for (const auto &pck: temp_deps) {
            dep_pck_map[pck].insert(name);
        }
        dep_pck_map[name].insert(name);

        set temp_reps = cpkg.getRecs();
        for (const auto &pck: temp_reps) {
            rec_pck_map[pck].insert(name);
        }
        return *this;
    }

    void dfs(string pkg, set<string> &res, set<string> &visited, bool withRecs) {
        visited.insert(pkg);
        for (const auto &dep: dep_pck_map.at(pkg)) {
            if (dep_pck_map.find(dep) != dep_pck_map.end()) {
                if (visited.find(dep) == visited.end()) {
                    res.insert(dep);
                    dfs(dep, res, visited, withRecs);
                }
            } else {
                throw runtime_error("lox");
            }
        }
        if (withRecs) {
            if (rec_pck_map.find(pkg) != rec_pck_map.end()) {
                for (const auto &dep: rec_pck_map.at(pkg)) {
//                    if (rec_pck_map.find(dep) != rec_pck_map.end()) {
                    res.insert(dep);
                    dfs(dep, res, visited, withRecs);
//                    }
                }
            }
        }
    }

    set<std::string> listAffected(list<string> pkgList, bool withRecs) {
        set<string> res;
        set<string> visited;
        for (const auto &pkg: pkgList) {
            if (dep_pck_map.find(pkg) != dep_pck_map.end()) {
                res.insert(pkg);
                dfs(pkg, res, visited, withRecs);
            } else {
                throw invalid_argument("lox");
            }
        }
        return res;
    }

private:
    map<string, set<string>> dep_pck_map;
    map<string, set<string>> rec_pck_map;

};

#ifndef __PROGTEST__

int main() {
    CPkg pkg("balik");
    CPkg("ssh").addDepend("openssh").addDepend("libstdc++");

    CPkgSys t0;
    t0.addPkg(CPkg("ssh").addDepend("openssh").addDepend("libstdc++"));
    t0.addPkg(CPkg("openssh").addDepend("openssh-server").addRecommend("openssh-client"));
    t0.addPkg(
            CPkg("openssh-server").addDepend("libkrb").addRecommend("libaudit").addDepend("libssl").addDepend("libc"));
    t0.addPkg(CPkg("openssh-client").addDepend("libssl").addDepend("libc"));
    t0.addPkg(CPkg("libaudit").addDepend("libc").addDepend("libstdc++"));
    t0.addPkg(CPkg("libssl").addDepend("libc"));
    t0.addPkg(CPkg("libc"));
    t0.addPkg(CPkg("libstdc++").addDepend("libc"));
    t0.addPkg(CPkg("ntpd").addDepend("libc").addDepend("libm"));
    t0.addPkg(CPkg("libssl-dev").addDepend("libc").addDepend("libssl"));
    t0.addPkg(CPkg("libm"));

    assert(
            t0.listAffected(std::list<std::string>{"libkrb"}, false) == (std::set<std::string>{"libkrb", "openssh",
                                                                                               "openssh-server",
                                                                                               "ssh"}));
    assert(
            t0.listAffected(std::list<std::string>{"libssl"}, false) == (std::set<std::string>{"libssl", "libssl-dev",
                                                                                               "openssh",
                                                                                               "openssh-client",
                                                                                               "openssh-server",
                                                                                               "ssh"}));
    assert(
            t0.listAffected(std::list<std::string>{"libssl", "libm"}, false) == (std::set<std::string>{"libm", "libssl",
                                                                                                       "libssl-dev",
                                                                                                       "ntpd",
                                                                                                       "openssh",
                                                                                                       "openssh-client",
                                                                                                       "openssh-server",
                                                                                                       "ssh"}));
    assert(
            t0.listAffected(std::list<std::string>{"libstdc++"}, false) ==
            (std::set<std::string>{"libaudit", "libstdc++",
                                   "ssh"}));
    assert(
            t0.listAffected(std::list<std::string>{"libstdc++"}, true) ==
            (std::set<std::string>{"libaudit", "libstdc++",
                                   "openssh", "openssh-server", "ssh"}));

    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */

