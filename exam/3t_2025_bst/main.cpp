#include<iostream>
#include<string>
#include<cassert>
#include<sstream>

using namespace std;

class CTree {
public:
    CTree() = default;

    CTree(const CTree &src) = delete;

    CTree &operator=(const CTree &src) = delete;

    ~CTree() {
        //TODO
    }

    bool isSet(const string &key) {
        CNode *current = m_Root;
        while (current) {
            if (key < current->m_Key) {
                current = current->m_L;
            } else if (key > current->m_Key) {
                current = current->m_R;
            } else {
                return true;
            }
        }
        return false;
    }

    bool insert(const string &key, const string &val) {

        CNode **current =&m_Root;

        while (*current) {
            if (key < (*current)->m_Key) {
                current = &((*current)->m_L);

            } else if (key > (*current)->m_Key) {
                current = &((*current)->m_R);

            } else {
                return false;
            }
        }
        CNode *newItem = new CNode(key, val);
        if (m_First == nullptr) {
            m_First = newItem;
        } else {
            m_Last->m_NextOrder = newItem;
        }
            m_Last = newItem;

        *current=newItem;
        return true;
    }

    friend ostream &operator<<(ostream &os, const CTree &src) {
        auto current = src.m_First;
        os << "{";
        while (current) {
            os << current->m_Key << " => " << current->m_Val;
            if (current != src.m_Last) {
                os << ", ";
            }
            current = current->m_NextOrder;
        }
        os << "}";
        return os;
    }

protected:
    class CNode {
    public:
        CNode(const string &key, const string &val)
                : m_Key(key), m_Val(val) {}

        string m_Key, m_Val;
        CNode *m_L = nullptr, *m_R = nullptr;
        CNode *m_NextOrder = nullptr;
    };

    CNode *m_Root = nullptr;
    CNode *m_First = nullptr, *m_Last = nullptr;

    friend int main();
};

int main(void) {
    CTree t;
    stringstream ss;
    ss << t;
    assert(ss.str() == "{}");
    ss.clear();
    ss.str("");
    assert(t.insert("PA1", "done"));
    assert(t.m_First == t.m_Last);
    assert(t.m_First->m_Key == "PA1");
    assert(!t.isSet("UOS"));
    assert(t.insert("PA2", "fail"));
    assert(t.insert("UOS", "funny"));

    ss << t;
    cout << ss.str() << endl << flush;
    assert(ss.str() == "{PA1 => done, PA2 => fail, UOS => funny}");
    ss.clear();
    ss.str("");


    assert(t.m_Root->m_L == nullptr);
    assert(t.m_Last->m_Key == "UOS");
    assert(t.m_Root->m_R->m_Key == "PA2");
    assert(t.m_Root->m_R->m_L == nullptr);
    assert(t.m_Root->m_R->m_R->m_Key == "UOS");
    assert(t.m_First->m_NextOrder->m_NextOrder == t.m_Last);
    assert(t.isSet("PA2"));

    assert(t.insert("CAO", "lul"));
    assert(t.insert("LIN", "F"));
    assert(t.m_Root->m_L->m_Key == "CAO");
    assert(t.m_Root->m_L->m_L == nullptr);
    assert(t.m_Root->m_L->m_R->m_Key == "LIN");
    assert(t.m_Last == t.m_Root->m_L->m_R);
    assert(t.m_Root->m_L->m_R->m_L == nullptr);
    assert(t.m_First->m_NextOrder->m_NextOrder->m_NextOrder->m_NextOrder = t.m_Root->m_L->m_R);
    assert(t.isSet("LIN"));

    ss << t;
    assert(ss.str() == "{PA1 => done, PA2 => fail, UOS => funny, CAO => lul, LIN => F}");
    ss.clear();
    ss.str("");

    assert(t.insert("SAP", "shit"));
    assert(t.m_Root->m_R->m_R->m_L->m_Key == "SAP");
    assert(t.m_Last == t.m_Root->m_R->m_R->m_L);


    ss << t;
    assert(ss.str() == "{PA1 => done, PA2 => fail, UOS => funny, CAO => lul, LIN => F, SAP => shit}");
    ss.clear();
    ss.str("");

    assert(!t.isSet("PA3"));
    assert(t.isSet("LIN"));
    assert(t.isSet("SAP"));

    return 0;
}

