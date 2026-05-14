#include <cassert>

template<typename T_>
class SetTester;

template<typename T_>
class Set {
public:
    Set() {}

    Set(const Set &) = delete;

    Set &operator=(const Set &) = delete;

    bool contains(const T_ &val) {
        auto ptr = m_Root;
        while (ptr) {
            if (val < ptr->m_Value) {
                ptr = ptr->m_Left;
            } else if (ptr->m_Value < val) {
                ptr = ptr->m_Right;
            } else {
                return true;
            }
        }
        return false;
    }

    bool insert(const T_ &val) {
        Node *newItem = new Node(val);
        Node *parent = nullptr;

        Node *current = m_Root;
        bool isLeft = true;
        while (current) {
            parent = current;
            if (val < current->m_Value) {
                current = current->m_Left;
                isLeft = true;
            } else if (current->m_Value < val) {
                current = current->m_Right;
                isLeft = false;
            } else {
                return false;
            }
        }

        if (m_First == nullptr) {
            m_First = m_Last = newItem;
        } else {
            m_Last->m_Next = newItem;
            newItem->m_Prev = m_Last;
            m_Last = newItem;
        }

        if (m_Root == nullptr) {
            m_Root = newItem;
        } else {
            if (isLeft) {
                parent->m_Left = newItem;
            } else {
                parent->m_Right = newItem;
            }
        }
        return true;
    }

    bool erase(const T_ &val) {
        Node *parent = nullptr;

        Node *current = m_Root;
        bool isLeft = true;

        while (current) {
            if (val < current->m_Value) {
                parent = current;
                current = current->m_Left;
                isLeft = true;
            } else if (current->m_Value < val) {
                parent = current;
                current = current->m_Right;
                isLeft = false;
            } else {
                break;
            }
        }
        if (!current) {
            return false;
        }

        //double link
        if (current == m_First) {
            m_First = m_First->m_Next;
            m_First->m_Prev = nullptr;
        } else if (current == m_Last) {
            m_Last->m_Prev->m_Next = nullptr;
        } else { //middle
            current->m_Prev->m_Next = current->m_Next;
            current->m_Next->m_Prev = current->m_Prev;
        }


        //BST
        if (!current->m_Left && !current->m_Right) {
            if (m_Root == current) {
                m_Root = nullptr;
            } else {
                parent->m_Next = nullptr;
            }
        } else if (!current->m_Left) {
            if (m_Root == current) {
                m_Root = current->m_Right;
            } else {
                if (isLeft) {
                    parent->m_Left = current->m_Right;
                } else {
                    parent->m_Right = current->m_Right;
                }
            }
        } else if (!current->m_Right) {
            if (m_Root == current) {
                m_Root = current->m_Left;
            } else {
                if (isLeft) {
                    parent->m_Left = current->m_Left;
                } else {
                    parent->m_Right = current->m_Left;
                }
            }
        } else {
            Node *successor = current->m_Right;
            Node *successorParent = current;

            while (successor->m_Left) {
                successorParent = successor;
                successor = successor->m_Left;
            }

            if (successorParent != current) {
                successorParent->m_Left = successor->m_Right;
                successor->m_Right = current->m_Right;
            }
            successor->m_Left = current->m_Left;

            if (current == m_Root) {
                m_Root = successor;
            } else if (isLeft) {
                parent->m_Left = successor;
            } else {
                parent->m_Prev = successor;
            }
        }
        delete current;

        return true;
    }

private:
    struct Node {
        Node(const T_ &v) : m_Value(v) {}

        Node *m_Left = nullptr;
        Node *m_Right = nullptr;
        Node *m_Prev = nullptr;
        Node *m_Next = nullptr;
        T_ m_Value;
    };

    Node *m_Root = nullptr;
    Node *m_First = nullptr;
    Node *m_Last = nullptr;

    friend SetTester<T_>;
};

#ifndef __TRAINER__

class UselessInt {
    int val;
public:
    UselessInt(int v) : val(v) {}

    bool operator<(const UselessInt &other) const {
        return val < other.val;
    }
};

int main() {
    Set<UselessInt> t;
    assert(!t.erase(5));
    assert(t.insert(5));
    //s
//    assert(t.insert(3));
//    assert(t.insert(7));
//    assert(t.insert(4));
//    assert(t.insert(8));
    //s
    assert(!t.insert(5));
    assert(t.contains(5));
    assert(t.erase(5));
    assert(t.insert(5));
    assert(t.insert(7));
    assert(t.insert(2));
    assert(t.insert(3));
    assert(!t.contains(1));
    assert(t.contains(2));
    assert(t.contains(3));
    assert(!t.contains(4));
    assert(t.contains(5));
    assert(!t.contains(6));
    assert(t.contains(7));
    assert(!t.contains(8));
    assert(t.insert(6));
    assert(t.erase(2));
    assert(!t.contains(1));
    assert(!t.contains(2));
    assert(t.contains(3));
    assert(!t.contains(4));
    assert(t.contains(5));
    assert(t.contains(6));
    assert(t.contains(7));
    assert(!t.contains(8));
    assert(t.erase(7));
    assert(!t.contains(1));
    assert(!t.contains(2));
    assert(t.contains(3));
    assert(!t.contains(4));
    assert(t.contains(5));
    assert(t.contains(6));
    assert(!t.contains(7));
    assert(!t.contains(8));
    assert(t.erase(5));
    assert(!t.contains(1));
    assert(!t.contains(2));
    assert(t.contains(3));
    assert(!t.contains(4));
    assert(!t.contains(5));
    assert(t.contains(6));
    assert(!t.contains(7));
    assert(!t.contains(8));
    return 0;
}

#endif