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
        // Check if value already exists
        if (contains(val)) {
            return false;
        }

        // Create new node
        Node *newNode = new Node(val);

        // Add to binary search tree
        if (!m_Root) {
            // First element in the set
            m_Root = newNode;
        } else {
            Node *current = m_Root;
            Node *parent = nullptr;

            // Find the correct position in the BST
            while (current) {
                parent = current;
                if (val < current->m_Value) {
                    current = current->m_Left;
                } else {
                    current = current->m_Right;
                }
            }

            // Insert at the correct position
            if (val < parent->m_Value) {
                parent->m_Left = newNode;
            } else {
                parent->m_Right = newNode;
            }
        }

        // Add to doubly-linked list to maintain insertion order
        if (!m_First) {
            // First element
            m_First = newNode;
            m_Last = newNode;
        } else {
            // Add to the end of the list
            m_Last->m_Next = newNode;
            newNode->m_Prev = m_Last;
            m_Last = newNode;
        }

        return true;
    }

    bool erase(const T_ &val) {
        if (!m_Root) {
            return false;  // Empty tree
        }

        // Find the node to delete
        Node *current = m_Root;
        Node *parent = nullptr;
        bool isLeftChild = false;

        // Search for the node
        while (current) {
            if (val < current->m_Value) {
                parent = current;
                current = current->m_Left;
                isLeftChild = true;
            } else if (current->m_Value < val) {
                parent = current;
                current = current->m_Right;
                isLeftChild = false;
            } else {
                // Found the node
                break;
            }
        }

        // Node not found
        if (!current) {
            return false;
        }

        // Remove from doubly-linked list first
        if (current == m_First && current == m_Last) {
            // Only node in the list
            m_First = nullptr;
            m_Last = nullptr;
        } else if (current == m_First) {
            // First node
            m_First = current->m_Next;
            if (m_First) {
                m_First->m_Prev = nullptr;
            }
        } else if (current == m_Last) {
            // Last node
            m_Last = current->m_Prev;
            if (m_Last) {
                m_Last->m_Next = nullptr;
            }
        } else {
            // Middle node
            current->m_Prev->m_Next = current->m_Next;
            current->m_Next->m_Prev = current->m_Prev;
        }

        // Now remove from BST

        // Case 1: Node is a leaf
        if (!current->m_Left && !current->m_Right) {
            if (current == m_Root) {
                m_Root = nullptr;
            } else if (isLeftChild) {
                parent->m_Left = nullptr;
            } else {
                parent->m_Right = nullptr;
            }
        }
            // Case 2: Node has one child
        else if (!current->m_Left) {
            // Has right child only
            if (current == m_Root) {
                m_Root = current->m_Right;
            } else if (isLeftChild) {
                parent->m_Left = current->m_Right;
            } else {
                parent->m_Right = current->m_Right;
            }
        } else if (!current->m_Right) {
            // Has left child only
            if (current == m_Root) {
                m_Root = current->m_Left;
            } else if (isLeftChild) {
                parent->m_Left = current->m_Left;
            } else {
                parent->m_Right = current->m_Left;
            }
        }
            // Case 3: Node has two children
        else {
            // Find successor (smallest value in right subtree)
            Node *successor = current->m_Right;
            Node *successorParent = current;

            while (successor->m_Left) {
                successorParent = successor;
                successor = successor->m_Left;
            }

            // If successor is not a direct child of the node to be deleted
            if (successorParent != current) {
                successorParent->m_Left = successor->m_Right;
                successor->m_Right = current->m_Right;
            }

            // Update references
            successor->m_Left = current->m_Left;

            if (current == m_Root) {
                m_Root = successor;
            } else if (isLeftChild) {
                parent->m_Left = successor;
            } else {
                parent->m_Right = successor;
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