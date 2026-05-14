#ifndef __PROGTEST__

#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <iostream>
#include <sstream>

#endif /* __PROGTEST__ */

bool isValidID(const char *id) {
    if (strlen(id) != 11 || id[6] != '/')
        return false;

    for (int i = 0; i < 6; i++)
        if (id[i] < '0' || id[i] > '9')
            return false;

    for (int i = 7; i < 11; i++)
        if (id[i] < '0' || id[i] > '9')
            return false;

    return true;
}

bool isValidDate(const char *date) {
    if (strlen(date) != 10 || date[4] != '-' || date[7] != '-')
        return false;

    for (int i = 0; i < 4; i++)
        if (date[i] < '0' || date[i] > '9')
            return false;

    for (int i = 5; i < 7; i++)
        if (date[i] < '0' || date[i] > '9')
            return false;

    for (int i = 8; i < 10; i++)
        if (date[i] < '0' || date[i] > '9')
            return false;

    int month = (date[5] - '0') * 10 + (date[6] - '0');
    int day = (date[8] - '0') * 10 + (date[9] - '0');

    if (month < 1 || month > 12 || day < 1 || day > 31)
        return false;

    return true;
}

class CStringPool {
private:
    static const size_t TABLE_SIZE = 4093;

    struct Node {
        char *str;
        Node *next;

        Node(const char *s, Node *n) : next(n) {
            str = new char[strlen(s) + 1];
            strcpy(str, s);
        }
    };

    Node *m_table[TABLE_SIZE] = {nullptr};

    size_t hash(const char *str) const {
        size_t h = 2166136261U;
        for (const char *p = str; *p; p++) {
            h ^= static_cast<size_t>(*p);
            h *= 16777619U;
        }
        return h % TABLE_SIZE;
    }

public:
    CStringPool() = default;

    ~CStringPool() {
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            Node *current = m_table[i];
            while (current) {
                Node *next = current->next;
                delete[] current->str;
                delete current;
                current = next;
            }
        }
    }

    const char *intern(const char *str) {
        size_t idx = hash(str);

        for (Node *current = m_table[idx]; current; current = current->next) {
            if (strcmp(current->str, str) == 0)
                return current->str;
        }

        m_table[idx] = new Node(str, m_table[idx]);
        return m_table[idx]->str;
    }
};

static CStringPool g_stringPool;

struct CAddress {
    const char *m_date;
    const char *m_street;
    const char *m_city;
    int *m_refCount;

    CAddress(const char *d, const char *s, const char *c) {
        m_date = g_stringPool.intern(d);
        m_street = g_stringPool.intern(s);
        m_city = g_stringPool.intern(c);
        m_refCount = new int(1);
    }

    ~CAddress() {
        if (--(*m_refCount) == 0) {
            delete m_refCount;
        }
    }

    CAddress(const CAddress &src) {
        m_date = src.m_date;
        m_street = src.m_street;
        m_city = src.m_city;
        m_refCount = src.m_refCount;
        (*m_refCount)++;
    }
};

struct CPerson {
    const char *m_id;
    const char *m_name;
    const char *m_surname;
    CAddress **m_addresses;
    int m_address_count;
    int m_address_capacity;
    int *m_refCount;
    bool m_shared;

    CPerson(const char *i, const char *n, const char *s) {
        m_id = g_stringPool.intern(i);
        m_name = g_stringPool.intern(n);
        m_surname = g_stringPool.intern(s);
        m_address_count = 0;
        m_address_capacity = 1;
        m_addresses = new CAddress *[m_address_capacity];
        m_refCount = new int(1);
        m_shared = false;
    }

    ~CPerson() {
        if (--(*m_refCount) == 0) {
            for (int i = 0; i < m_address_count; i++)
                delete m_addresses[i];
            delete[] m_addresses;
            delete m_refCount;
        }
    }

    CPerson(const CPerson &src) {
        if (!src.m_shared) {
            m_id = src.m_id;
            m_name = src.m_name;
            m_surname = src.m_surname;
            m_addresses = src.m_addresses;
            m_address_count = src.m_address_count;
            m_address_capacity = src.m_address_capacity;
            m_refCount = src.m_refCount;
            (*m_refCount)++;
            m_shared = true;
            const_cast<CPerson &>(src).m_shared = true;
        } else {
            m_id = src.m_id;
            m_name = src.m_name;
            m_surname = src.m_surname;
            m_address_count = src.m_address_count;
            m_address_capacity = src.m_address_capacity;
            m_addresses = new CAddress *[m_address_capacity];
            for (int i = 0; i < m_address_count; i++) {
                m_addresses[i] = new CAddress(*src.m_addresses[i]);
            }
            m_refCount = new int(1);
            m_shared = false;
        }
    }

    void detach() {
        if (!m_shared)
            return;

        CAddress **tmp_new_addresses = new CAddress *[m_address_capacity];
        for (int i = 0; i < m_address_count; i++) {
            tmp_new_addresses[i] = new CAddress(*m_addresses[i]);
        }

        if (--(*m_refCount) == 0) {
            for (int i = 0; i < m_address_count; i++)
                delete m_addresses[i];
            delete[] m_addresses;
            delete m_refCount;
        }

        m_addresses = tmp_new_addresses;
        m_refCount = new int(1);
        m_shared = false;
    }

    bool addAddress(const char *date, const char *street, const char *city) {
        detach();

        for (int i = 0; i < m_address_count; i++) {
            if (strcmp(m_addresses[i]->m_date, date) == 0) {
                return false;
            }
        }

        if (m_address_count >= m_address_capacity) {
            m_address_capacity *= 2;
            CAddress **tmp_new_addresses = new CAddress *[m_address_capacity];
            for (int i = 0; i < m_address_count; i++)
                tmp_new_addresses[i] = m_addresses[i];
            delete[] m_addresses;
            m_addresses = tmp_new_addresses;
        }

        m_addresses[m_address_count++] = new CAddress(date, street, city);

        int pos = 0;
        int left = 0, right = m_address_count - 2;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (strcmp(m_addresses[mid]->m_date, date) < 0)
                left = mid + 1;
            else
                right = mid - 1;
        }
        pos = left;

        if (pos < m_address_count - 1) {
            CAddress *temp = m_addresses[m_address_count - 1];
            for (int i = m_address_count - 1; i > pos; i--)
                m_addresses[i] = m_addresses[i - 1];
            m_addresses[pos] = temp;
        }

        return true;
    }
};

class CRegister {
private:
    CPerson **people;
    int m_person_count;
    int m_person_capacity;

    int findPerson(const char *id) const {
        int left = 0;
        int right = m_person_count - 1;

        while (left <= right) {
            int mid = left + (right - left) / 2;
            int cmp = strcmp(people[mid]->m_id, id);

            if (cmp == 0)
                return mid;

            if (cmp < 0)
                left = mid + 1;
            else
                right = mid - 1;
        }

        return -1;
    }

    int findInsertPos(const char *id) const {
        int left = 0;
        int right = m_person_count - 1;

        while (left <= right) {
            int mid = left + (right - left) / 2;
            int cmp = strcmp(people[mid]->m_id, id);

            if (cmp < 0)
                left = mid + 1;
            else
                right = mid - 1;
        }

        return left;
    }

public:
    CRegister() {
        m_person_count = 0;
        m_person_capacity = 16;
        people = new CPerson *[m_person_capacity];
    }

    CRegister(const CRegister &src) {
        m_person_count = src.m_person_count;
        m_person_capacity = src.m_person_capacity;
        people = new CPerson *[m_person_capacity];

        for (int i = 0; i < m_person_count; i++)
            people[i] = new CPerson(*src.people[i]);
    }

    ~CRegister() {
        for (int i = 0; i < m_person_count; i++)
            delete people[i];

        delete[] people;
    }

    CRegister &operator=(const CRegister &src) {
        if (this == &src)
            return *this;

        for (int i = 0; i < m_person_count; i++)
            delete people[i];

        delete[] people;

        m_person_count = src.m_person_count;
        m_person_capacity = src.m_person_capacity;
        people = new CPerson *[m_person_capacity];

        for (int i = 0; i < m_person_count; i++)
            people[i] = new CPerson(*src.people[i]);

        return *this;
    }

    bool add(const char id[], const char name[], const char surname[],
             const char date[], const char street[], const char city[]) {
        if (!isValidID(id) || !isValidDate(date))
            return false;

        if (m_person_count > 0 && findPerson(id) >= 0)
            return false;

        if (m_person_count >= m_person_capacity) {
            m_person_capacity *= 2;
            CPerson **tmp_new_people = new CPerson *[m_person_capacity];

            for (int i = 0; i < m_person_count; i++)
                tmp_new_people[i] = people[i];

            delete[] people;
            people = tmp_new_people;
        }

        CPerson *newPerson = new CPerson(id, name, surname);
        newPerson->addAddress(date, street, city);

        int pos = findInsertPos(id);

        for (int i = m_person_count; i > pos; i--)
            people[i] = people[i - 1];

        people[pos] = newPerson;
        m_person_count++;

        return true;
    }

    bool resettle(const char id[], const char date[], const char street[], const char city[]) {
        if (!isValidID(id) || !isValidDate(date))
            return false;

        int idx = findPerson(id);
        if (idx >= 0) {
            return people[idx]->addAddress(date, street, city);
        }

        return false;
    }

    bool print(std::ostream &os, const char id[]) const {
        if (!isValidID(id))
            return false;

        int idx = findPerson(id);
        if (idx >= 0) {
            os << people[idx]->m_id << " " << people[idx]->m_name << " " << people[idx]->m_surname << std::endl;

            for (int j = 0; j < people[idx]->m_address_count; j++) {
                os << people[idx]->m_addresses[j]->m_date << " "
                   << people[idx]->m_addresses[j]->m_street << " "
                   << people[idx]->m_addresses[j]->m_city << std::endl;
            }

            return true;
        }

        return false;
    }
};

#ifndef __PROGTEST__

int main() {
    char lID[12], lDate[12], lName[50], lSurname[50], lStreet[50], lCity[50];
    std::ostringstream oss;
    CRegister a;
    assert (a.add("123456/7890", "John", "Smith", "2000-01-01", "Main m_street", "Seattle") == true);
    assert (a.add("987654/3210", "Freddy", "Kruger", "2001-02-03", "Elm m_street", "Sacramento") == true);
    assert (a.resettle("123456/7890", "2003-05-12", "Elm m_street", "Atlanta") == true);
    assert (a.resettle("123456/7890", "2002-12-05", "Sunset boulevard", "Los Angeles") == true);
    oss.str("");
    assert (a.print(oss, "123456/7890") == true);
    assert (!strcmp(oss.str().c_str(), R"###(123456/7890 John Smith
2000-01-01 Main m_street Seattle
2002-12-05 Sunset boulevard Los Angeles
2003-05-12 Elm m_street Atlanta
)###"));
    oss.str("");
    assert (a.print(oss, "987654/3210") == true);
    assert (!strcmp(oss.str().c_str(), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm m_street Sacramento
)###"));
    CRegister b(a);
    assert (b.resettle("987654/3210", "2008-04-12", "Elm m_street", "Cinccinati") == true);
    assert (a.resettle("987654/3210", "2007-02-11", "Elm m_street", "Indianapolis") == true);
    oss.str("");
    assert (a.print(oss, "987654/3210") == true);
    assert (!strcmp(oss.str().c_str(), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm m_street Sacramento
2007-02-11 Elm m_street Indianapolis
)###"));
    oss.str("");
    assert (b.print(oss, "987654/3210") == true);
    assert (!strcmp(oss.str().c_str(), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm m_street Sacramento
2008-04-12 Elm m_street Cinccinati
)###"));
    a = b;
    assert (a.resettle("987654/3210", "2011-05-05", "Elm m_street", "Salt Lake City") == true);
    oss.str("");
    assert (a.print(oss, "987654/3210") == true);
    assert (!strcmp(oss.str().c_str(), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm m_street Sacramento
2008-04-12 Elm m_street Cinccinati
2011-05-05 Elm m_street Salt Lake City
)###"));
    oss.str("");
    assert (b.print(oss, "987654/3210") == true);
    assert (!strcmp(oss.str().c_str(), R"###(987654/3210 Freddy Kruger
2001-02-03 Elm m_street Sacramento
2008-04-12 Elm m_street Cinccinati
)###"));
    assert (b.add("987654/3210", "Joe", "Lee", "2010-03-17", "Abbey road", "London") == false);
    assert (a.resettle("987654/3210", "2001-02-03", "Second m_street", "Milwaukee") == false);
    oss.str("");
    assert (a.print(oss, "666666/6666") == false);

    CRegister c;
    strncpy(lID, "123456/7890", sizeof(lID));
    strncpy(lName, "John", sizeof(lName));
    strncpy(lSurname, "Smith", sizeof(lSurname));
    strncpy(lDate, "2000-01-01", sizeof(lDate));
    strncpy(lStreet, "Main m_street", sizeof(lStreet));
    strncpy(lCity, "Seattle", sizeof(lCity));
    assert (c.add(lID, lName, lSurname, lDate, lStreet, lCity) == true);
    strncpy(lID, "987654/3210", sizeof(lID));
    strncpy(lName, "Freddy", sizeof(lName));
    strncpy(lSurname, "Kruger", sizeof(lSurname));
    strncpy(lDate, "2001-02-03", sizeof(lDate));
    strncpy(lStreet, "Elm m_street", sizeof(lStreet));
    strncpy(lCity, "Sacramento", sizeof(lCity));
    assert (c.add(lID, lName, lSurname, lDate, lStreet, lCity) == true);
    strncpy(lID, "123456/7890", sizeof(lID));
    strncpy(lDate, "2003-05-12", sizeof(lDate));
    strncpy(lStreet, "Elm m_street", sizeof(lStreet));
    strncpy(lCity, "Atlanta", sizeof(lCity));
    assert (c.resettle(lID, lDate, lStreet, lCity) == true);
    strncpy(lID, "123456/7890", sizeof(lID));
    strncpy(lDate, "2002-12-05", sizeof(lDate));
    strncpy(lStreet, "Sunset boulevard", sizeof(lStreet));
    strncpy(lCity, "Los Angeles", sizeof(lCity));
    assert (c.resettle(lID, lDate, lStreet, lCity) == true);
    oss.str("");
    assert (c.print(oss, "123456/7890") == true);
    assert (!strcmp(oss.str().c_str(), R"###(123456/7890 John Smith
2000-01-01 Main m_street Seattle
2002-12-05 Sunset boulevard Los Angeles
2003-05-12 Elm m_street Atlanta
)###"));

    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */
