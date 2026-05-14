#ifndef __PROGTEST__

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <memory>
#include <compare>
#include <functional>
#include <stdexcept>
#include <algorithm>

#endif

using namespace std;

class Identity {
public:
    Identity(const string &name, const string &address, string account, int sumExpense, int sumIncome) :
            m_name(name), m_address(address), m_account(std::move(account)),
            m_sumExpense(sumExpense), m_sumIncome(sumIncome) {};

    const string &getName() const { return m_name; }

    const string &getAddress() const { return m_address; }

    const string &getAccount() const { return m_account; }

    void setSumExpense(int sumExpense) { m_sumExpense = sumExpense; }

    int getSumExpense() const { return m_sumExpense; }

    void setSumIncome(int sumIncome) { m_sumIncome = sumIncome; }

    int getSumIncome() const { return m_sumIncome; }

private:
    string m_name;
    string m_address;
    string m_account;
    int m_sumExpense;
    int m_sumIncome;
};


class CIterator {
public:
    CIterator() {};

    CIterator(std::vector<Identity>::const_iterator myIterBegin,
              std::vector<Identity>::const_iterator myIterEnd) : beginCIterator(myIterBegin),
                                                                 endCIterator(myIterEnd) {}

    bool atEnd() const {
        return beginCIterator == endCIterator;
    }

    void next() {
        if (!atEnd()) {
            beginCIterator++;
        }
    }

    const std::string &name() const { return beginCIterator->getName(); }

    const std::string &addr() const { return beginCIterator->getAddress(); }

    const std::string &account() const { return beginCIterator->getAccount(); }

private:
    vector<Identity>::const_iterator beginCIterator;
    vector<Identity>::const_iterator endCIterator;
};

class CTaxRegister {
public:
    bool birth(const std::string &name,
               const std::string &addr,
               const std::string &account) {
        auto identity = Identity(name, addr, account, 0, 0);
        auto it1 = lower_bound(reg_name_sorted.begin(), reg_name_sorted.end(), identity,
                               [](const Identity &id1, const Identity &id2) {
                                   return std::make_pair(id1.getName(), id1.getAddress()) <
                                          std::make_pair(id2.getName(), id2.getAddress());
                               });
        if (it1 != reg_name_sorted.end() && (it1->getName() == name && it1->getAddress() == addr)) {
            return false;
        }

        auto it2 = lower_bound(reg_account_sorted.begin(), reg_account_sorted.end(), identity,
                               [](const Identity &id1, const Identity &id2) {
                                   return id1.getAccount() < id2.getAccount();
                               });
        if (it2 != reg_account_sorted.end() && it2->getAccount() == account) {
            return false;
        }
        reg_name_sorted.insert(it1, identity);
        reg_account_sorted.insert(it2, identity);
        return true;
    }

    bool death(const std::string &name,
               const std::string &addr) {
        auto identity = Identity(name, addr, "", 0, 0);
        auto it1 = lower_bound(reg_name_sorted.begin(), reg_name_sorted.end(), identity,
                               [](const Identity &id1, const Identity &id2) {
                                   return std::make_pair(id1.getName(), id1.getAddress()) <
                                          std::make_pair(id2.getName(), id2.getAddress());
                               });
        if (it1 == reg_name_sorted.end() || it1->getName() != name || it1->getAddress() != addr) {
            return false;
        }

        string account = it1->getAccount();
        auto tempIdentity = Identity("", "", account, 0, 0);

        auto it2 = lower_bound(reg_account_sorted.begin(), reg_account_sorted.end(), tempIdentity,
                               [](const Identity &id1, const Identity &id2) {
                                   return id1.getAccount() < id2.getAccount();
                               });
        if (it2 == reg_account_sorted.end() || it2->getAccount() != account) {
            return false;
        }
        reg_name_sorted.erase(it1);
        reg_account_sorted.erase(it2);
        return true;
    }

    bool income(const std::string &account,
                int amount) {
        auto identity = Identity("", "", account, 0, amount);
        auto it = lower_bound(reg_account_sorted.begin(), reg_account_sorted.end(), identity,
                              [](const Identity &id1, const Identity &id2) {
                                  return id1.getAccount() < id2.getAccount();
                              });
        if (it == reg_account_sorted.end() || it->getAccount() != account) {
            return false;
        }
        it->setSumIncome(it->getSumIncome() + amount);

        const std::string &name = it->getName();
        const std::string &addr = it->getAddress();

        auto tempIdentity = Identity(name, addr, "", 0, 0);
        auto it2 = lower_bound(reg_name_sorted.begin(), reg_name_sorted.end(),
                               tempIdentity, [](const Identity &id1, const Identity &id2) {
                    return std::make_pair(id1.getName(), id1.getAddress()) <
                           std::make_pair(id2.getName(), id2.getAddress());
                });

        if (it2 == reg_name_sorted.end() || it2->getName() != name || it2->getAddress() != addr) {
            return false;
        }

        it2->setSumIncome(it2->getSumIncome() + amount);
        return true;
    }

    bool income(const std::string &name,
                const std::string &addr,
                int amount) {
        auto identity = Identity(name, addr, "", 0, amount);
        auto it = lower_bound(reg_name_sorted.begin(), reg_name_sorted.end(), identity,
                              [](const Identity &id1, const Identity &id2) {
                                  return std::make_pair(id1.getName(), id1.getAddress()) <
                                         std::make_pair(id2.getName(), id2.getAddress());
                              });
        if (it == reg_name_sorted.end() || it->getName() != name || it->getAddress() != addr) {
            return false;
        }
        it->setSumIncome(it->getSumIncome() + amount);


        std::string account = it->getAccount();

        auto tempIdentity = Identity("", "", account, 0, 0);

        auto it2 = std::lower_bound(reg_account_sorted.begin(), reg_account_sorted.end(), tempIdentity,
                                    [](const Identity &id1, const Identity &id2) {
                                        return id1.getAccount() < id2.getAccount();
                                    });

        if (it2 == reg_account_sorted.end() || it2->getAccount() != account) {
            return false;
        }
        it2->setSumIncome(it2->getSumIncome() + amount);
        return true;
    }

    bool expense(const std::string &account,
                 int amount) {
        auto identity = Identity("", "", account, amount, 0);
        auto it = lower_bound(reg_account_sorted.begin(), reg_account_sorted.end(), identity,
                              [](const Identity &id1, const Identity &id2) {
                                  return id1.getAccount() < id2.getAccount();
                              });
        if (it == reg_account_sorted.end() || it->getAccount() != account) {
            return false;
        }
        it->setSumExpense(it->getSumExpense() + amount);


        const std::string &name = it->getName();
        const std::string &addr = it->getAddress();

        auto tempIdentity = Identity(name, addr, "", 0, 0);
        auto it2 = lower_bound(reg_name_sorted.begin(), reg_name_sorted.end(),
                               tempIdentity, [](const Identity &id1, const Identity &id2) {
                    return std::make_pair(id1.getName(), id1.getAddress()) <
                           std::make_pair(id2.getName(), id2.getAddress());
                });

        if (it2 == reg_name_sorted.end() || it2->getName() != name || it2->getAddress() != addr) {
            return false;
        }
        it2->setSumExpense(it2->getSumExpense() + amount);
        return true;
    }

    bool expense(const std::string &name,
                 const std::string &addr,
                 int amount) {
        auto identity = Identity(name, addr, "", amount, 0);
        auto it = lower_bound(reg_name_sorted.begin(), reg_name_sorted.end(), identity,
                              [](const Identity &id1, const Identity &id2) {
                                  return std::make_pair(id1.getName(), id1.getAddress()) <
                                         std::make_pair(id2.getName(), id2.getAddress());
                              });
        if (it == reg_name_sorted.end() || it->getName() != name || it->getAddress() != addr) {
            return false;
        }
        it->setSumExpense(it->getSumExpense() + amount);


        std::string account = it->getAccount();
        auto tempIdentity = Identity("", "", account, 0, 0);

        auto it2 = lower_bound(reg_account_sorted.begin(), reg_account_sorted.end(), tempIdentity,
                               [](const Identity &id1, const Identity &id2) {
                                   return id1.getAccount() < id2.getAccount();
                               });
        if (it2 == reg_account_sorted.end() || it2->getAccount() != account) {
            return false;
        }
        it2->setSumExpense(it2->getSumExpense() + amount);

        return true;
    }

    bool audit(const std::string &name,
               const std::string &addr,
               std::string &account,
               int &sumIncome,
               int &sumExpense) const {
        auto identity = Identity(name, addr, "", 0, 0);
        auto it = lower_bound(reg_name_sorted.begin(), reg_name_sorted.end(), identity,
                              [](const Identity &id1, const Identity &id2) {
                                  return std::make_pair(id1.getName(), id1.getAddress()) <
                                         std::make_pair(id2.getName(), id2.getAddress());
                              });
        if (it == reg_name_sorted.end() || it->getName() != name || it->getAddress() != addr) {
            return false;
        }
        account = it->getAccount();
        sumIncome = it->getSumIncome();
        sumExpense = it->getSumExpense();
        return true;
    }

    CIterator listByName() const {
        return CIterator(reg_name_sorted.begin(), reg_name_sorted.end());
    }

private:
    vector<Identity> reg_name_sorted;
    vector<Identity> reg_account_sorted;
};

#ifndef __PROGTEST__

int main() {
    std::string acct;
    int sumIncome, sumExpense;
    CTaxRegister b0;
    assert(b0.birth("John Smith", "Oak Road 23", "123/456/789"));
    assert(b0.birth("Jane Hacker", "Main Street 17", "Xuj5#94"));
    assert(b0.birth("Peter Hacker", "Main Street 17", "634oddT"));
    assert(b0.birth("John Smith", "Main Street 17", "Z343rwZ"));
    assert(b0.income("Xuj5#94", 1000));
    assert(b0.income("634oddT", 2000));
    assert(b0.income("123/456/789", 3000));
    assert(b0.income("634oddT", 4000));
    assert(b0.income("Peter Hacker", "Main Street 17", 2000));
    assert(b0.expense("Jane Hacker", "Main Street 17", 2000));
    assert(b0.expense("John Smith", "Main Street 17", 500));
    assert(b0.expense("Jane Hacker", "Main Street 17", 1000));
    assert(b0.expense("Xuj5#94", 1300));
    assert(b0.audit("John Smith", "Oak Road 23", acct, sumIncome, sumExpense));
    assert(acct == "123/456/789");
    assert(sumIncome == 3000);
    assert(sumExpense == 0);
    assert(b0.audit("Jane Hacker", "Main Street 17", acct, sumIncome, sumExpense));
    assert(acct == "Xuj5#94");
    assert(sumIncome == 1000);
    assert(sumExpense == 4300);
    assert(b0.audit("Peter Hacker", "Main Street 17", acct, sumIncome, sumExpense));
    assert(acct == "634oddT");
    assert(sumIncome == 8000);
    assert(sumExpense == 0);
    assert(b0.audit("John Smith", "Main Street 17", acct, sumIncome, sumExpense));
    assert(acct == "Z343rwZ");
    assert(sumIncome == 0);
    assert(sumExpense == 500);
    CIterator it = b0.listByName();
    assert(!it.atEnd()
           && it.name() == "Jane Hacker"
           && it.addr() == "Main Street 17"
           && it.account() == "Xuj5#94");
    it.next();
    assert(!it.atEnd()
           && it.name() == "John Smith"
           && it.addr() == "Main Street 17"
           && it.account() == "Z343rwZ");
    it.next();
    assert(!it.atEnd()
           && it.name() == "John Smith"
           && it.addr() == "Oak Road 23"
           && it.account() == "123/456/789");
    it.next();
    assert(!it.atEnd()
           && it.name() == "Peter Hacker"
           && it.addr() == "Main Street 17"
           && it.account() == "634oddT");
    it.next();
    assert(it.atEnd());

    assert(b0.death("John Smith", "Main Street 17"));

    CTaxRegister b1;
    assert(b1.birth("John Smith", "Oak Road 23", "123/456/789"));
    assert(b1.birth("Jane Hacker", "Main Street 17", "Xuj5#94"));
    assert(!b1.income("634oddT", 4000));
    assert(!b1.expense("John Smith", "Main Street 18", 500));
    assert(!b1.audit("John Nowak", "Second Street 23", acct, sumIncome, sumExpense));
    assert(!b1.death("Peter Nowak", "5-th Avenue"));
    assert(!b1.birth("Jane Hacker", "Main Street 17", "4et689A"));
    assert(!b1.birth("Joe Hacker", "Elm Street 23", "Xuj5#94"));
    assert(b1.death("Jane Hacker", "Main Street 17"));
    assert(b1.birth("Joe Hacker", "Elm Street 23", "Xuj5#94"));
    assert(b1.audit("Joe Hacker", "Elm Street 23", acct, sumIncome, sumExpense));
    assert(acct == "Xuj5#94");
    assert(sumIncome == 0);
    assert(sumExpense == 0);
    assert(!b1.birth("Joe Hacker", "Elm Street 23", "AAj5#94"));

    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */
