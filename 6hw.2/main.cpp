#ifndef __PROGTEST__

#include <cstring>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cctype>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <memory>
#include <functional>
#include <stdexcept>

#endif /* __PROGTEST__ */

using namespace std;

class CText;

class CEmpty;

class CImage;

class CCell {
public:
    virtual ~CCell() = default;

    virtual CCell *clone() const = 0;

    virtual pair<size_t, size_t> getDim() const = 0;

    virtual vector<string> getLines(size_t width, size_t height) const = 0;

    virtual bool equals(const CCell &other) const = 0;

    virtual bool isText() const { return false; }

    virtual bool isImage() const { return false; }

    virtual bool isTable() const { return false; }

    virtual bool isEmpty() const { return false; }
};

bool operator==(const CCell &lparam, const CCell &rparam) {
    return lparam.equals(rparam);
}

bool operator!=(const CCell &lparam, const CCell &rparam) {
    return !lparam.equals(rparam);
}

class CEmpty : public CCell {
public:
    CCell *clone() const override {
        return new CEmpty(*this);
    }

    pair<size_t, size_t> getDim() const override {
        return {0, 0};
    }

    vector<string> getLines(size_t width, size_t height) const override {
        vector<string> result;
        for (size_t i = 0; i < height; ++i) {
            result.emplace_back(string(width, ' '));
        }
        return result;
    }

    bool equals(const CCell &other) const override {
        return other.isEmpty();
    }

    bool isEmpty() const override {
        return true;
    }
};

class CText : public CCell {
public:
    enum Alignment {
        ALIGN_LEFT,
        ALIGN_RIGHT
    };

    CText(const string &text, Alignment align) : m_text(text), m_alignment(align) {
        parseLines();
    }

    CCell *clone() const override {
        return new CText(*this);
    }

    void setText(const string &text) {
        m_text = text;
        parseLines();
    }


    pair<size_t, size_t> getDim() const override {
        size_t width = 0;
        for (const auto &line: m_lines) {
            width = max(width, line.length());
        }
        return {width, m_lines.size()};
    }

    vector<string> getLines(size_t width, size_t height) const override {
        vector<string> result;
        for (size_t i = 0; i < height; ++i) {
            const string &tempI = m_lines[i];
            if (i < m_lines.size()) {
                if (m_alignment == ALIGN_LEFT) {
                    result.emplace_back(tempI + string(width - tempI.length(), ' '));
                } else {
                    // RIGHT
                    result.emplace_back(string(width - tempI.length(), ' ') + tempI);
                }
            } else {
                result.emplace_back(string(width, ' '));
            }
        }
        return result;
    }

    bool equals(const CCell &other) const override {
        if (!other.isText()) {
            return false;
        }
        const CText &otherText = static_cast<const CText &>(other);
        return m_text == otherText.m_text && m_alignment == otherText.m_alignment;
    }

    void parseLines() {
        m_lines.clear();
        istringstream iss(m_text);
        string line;
        while (getline(iss, line)) {
            m_lines.emplace_back(line);
        }

        if (m_lines.empty()) {
            m_lines.emplace_back("");
        }
    }

    bool isText() const override {
        return true;
    }

private:
    string m_text;
    Alignment m_alignment;
    vector<string> m_lines;
};

class CImage : public CCell {
public:
    CImage &addRow(const string &row) {
        m_rows.emplace_back(row);
        return *this;
    }

    CCell *clone() const override {
        return new CImage(*this);
    }

    pair<size_t, size_t> getDim() const override {
        if (m_rows.empty()) {
            return {0, 0};
        }
        return {m_rows[0].length(), m_rows.size()};
    }

    vector<string> getLines(size_t width, size_t height) const override {
        vector<string> result(height, string(width, ' '));
        if (m_rows.empty()) {
            return result;
        }
        size_t imgWidth = m_rows[0].length();
        size_t imgHeight = m_rows.size();
        size_t leftPos = (width - imgWidth) / 2;
        size_t heightPos = (height - imgHeight) / 2;

        for (size_t i = 0; i < imgHeight && i + heightPos < height; ++i) {
            const string &row = m_rows[i];
            string &targetRow = result[i + heightPos];
            for (size_t j = 0; j < imgWidth && j + leftPos < width; ++j) {
                targetRow[j + leftPos] = row[j];
            }
        }
        return result;
    }

    bool equals(const CCell &other) const override {
        if (!other.isImage()) {
            return false;
        }
        const CImage &otherImage = static_cast<const CImage &>(other);
        return m_rows == otherImage.m_rows;
    }

    bool isImage() const override {
        return true;
    }

private:
    vector<string> m_rows;
};

class CTable : public CCell {
public:
    CTable(size_t rows, size_t cols) : m_rows(rows), m_cols(cols) {
        m_cells.resize(rows);
        for (size_t i = 0; i < rows; ++i) {
            m_cells[i].resize(cols);
            for (size_t j = 0; j < cols; ++j) {
                m_cells[i][j] = make_shared<CEmpty>();
            }
        }
    }

    CTable(const CTable &other) : m_rows(other.m_rows), m_cols(other.m_cols) {
        m_cells.resize(m_rows);
        for (size_t i = 0; i < m_rows; ++i) {
            m_cells[i].resize(m_cols);
            for (size_t j = 0; j < m_cols; ++j) {
                m_cells[i][j] = shared_ptr<CCell>(other.m_cells[i][j]->clone());
            }
        }
    }

    CTable &operator=(const CTable &other) {
        if (this != &other) {
            m_rows = other.m_rows;
            m_cols = other.m_cols;
            m_cells.resize(m_rows);
            for (size_t i = 0; i < m_rows; ++i) {
                m_cells[i].resize(m_cols);
                for (size_t j = 0; j < m_cols; ++j) {
                    m_cells[i][j] = shared_ptr<CCell>(other.m_cells[i][j]->clone());
                }
            }
        }
        return *this;
    }

    void setCell(size_t row, size_t col, const CCell &newContent) {
        m_cells[row][col] = shared_ptr<CCell>(newContent.clone());
    }

    CCell &getCell(size_t row, size_t col) const {
        return *m_cells[row][col];
    }

    bool operator==(const CTable &other) const {
        if (m_rows != other.m_rows || m_cols != other.m_cols) {
            return false;
        }
        for (size_t i = 0; i < m_rows; ++i) {
            for (size_t j = 0; j < m_cols; ++j) {
                if (*m_cells[i][j] != *other.m_cells[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }

    bool operator!=(const CTable &other) const {
        return !(*this == other);
    }

    friend ostream &operator<<(ostream &os, const CTable &table);

    CCell *clone() const override {
        return new CTable(*this);
    }

    pair<size_t, size_t> getDim() const override {
        if (m_rows == 0 || m_cols == 0) {
            return {0, 0};
        }
        size_t totalWidth = 1;
        vector<size_t> colWidths(m_cols, 0);
        for (size_t j = 0; j < m_cols; ++j) {
            for (size_t i = 0; i < m_rows; ++i) {
                pair<size_t, size_t> cellDim = m_cells[i][j]->getDim();
                colWidths[j] = max(colWidths[j], cellDim.first);
            }
            totalWidth += colWidths[j] + 1;
        }

        size_t totalHeight = 1;
        for (size_t i = 0; i < m_rows; ++i) {
            size_t rowHeight = 0;
            for (size_t j = 0; j < m_cols; ++j) {
                pair<size_t, size_t> cellDim = m_cells[i][j]->getDim();
                rowHeight = max(rowHeight, cellDim.second);
            }
            totalHeight += rowHeight + 1;
        }
        return {totalWidth, totalHeight};
    }

    vector<string> getLines(size_t width, size_t height) const override {
        ostringstream oss;
        oss << *this;
        string tableStr = oss.str();
        istringstream iss(tableStr);
        vector<string> result;
        string line;
        while (getline(iss, line) && result.size() < height) {
            if (line.length() < width) {
                result.emplace_back(line + string(width - line.length(), ' '));
            } else {
                result.emplace_back(line.substr(0, width));
            }
        }

        while (result.size() < height) {
            result.emplace_back(string(width, ' '));
        }
        return result;
    }

    bool equals(const CCell &other) const override {
        if (!other.isTable()) {
            return false;
        }
        const CTable &otherTable = static_cast<const CTable &>(other);
        if (m_rows != otherTable.m_rows || m_cols != otherTable.m_cols) {
            return false;
        }

        for (size_t i = 0; i < m_rows; ++i) {
            for (size_t j = 0; j < m_cols; ++j) {
                if (*m_cells[i][j] != *otherTable.m_cells[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }

    bool isTable() const override {
        return true;
    }

private:
    size_t m_rows;
    size_t m_cols;
    vector<vector<shared_ptr<CCell>>> m_cells;
};

ostream &operator<<(ostream &os, const CTable &table) {
    if (table.m_rows == 0 || table.m_cols == 0) {
        return os;
    }

    vector<size_t> colNum(table.m_cols, 0);
    vector<size_t> rawNum(table.m_rows, 0);
    for (size_t i = 0; i < table.m_rows; ++i) {
        for (size_t j = 0; j < table.m_cols; ++j) {
            pair<size_t, size_t> dims = table.m_cells[i][j]->getDim();
            colNum[j] = max(colNum[j], dims.first);
            rawNum[i] = max(rawNum[i], dims.second);
        }
    }

    for (size_t i = 0; i < table.m_rows; ++i) {
        os << "+";
        for (size_t j = 0; j < table.m_cols; ++j) {
            os << string(colNum[j], '-') << "+";
        }
        os << endl;

        vector<vector<string>> cellLines(table.m_cols);
        for (size_t j = 0; j < table.m_cols; ++j) {
            cellLines[j] = table.m_cells[i][j]->getLines(colNum[j], rawNum[i]);
        }
        for (size_t lineIdex = 0; lineIdex < rawNum[i]; ++lineIdex) {
            os << "|";
            for (size_t j = 0; j < table.m_cols; ++j) {
                os << cellLines[j][lineIdex] << "|";
            }
            os << endl;
        }
    }
    os << "+";
    for (size_t j = 0; j < table.m_cols; ++j) {
        os << string(colNum[j], '-') << "+";
    }
    os << endl;

    return os;
}

#ifndef __PROGTEST__

int main() {
    std::ostringstream oss;
    CTable t0(3, 2);
    t0.setCell(0, 0, CText("Hello,\n"
                           "Hello Kitty", CText::ALIGN_LEFT));
    t0.setCell(1, 0, CText("Lorem ipsum dolor sit amet", CText::ALIGN_LEFT));
    t0.setCell(2, 0, CText("Bye,\n"
                           "Hello Kitty", CText::ALIGN_RIGHT));
    t0.setCell(1, 1, CImage()
            .addRow("###                   ")
            .addRow("#  #                  ")
            .addRow("#  # # ##   ###    ###")
            .addRow("###  ##    #   #  #  #")
            .addRow("#    #     #   #  #  #")
            .addRow("#    #     #   #  #  #")
            .addRow("#    #      ###    ###")
            .addRow("                     #")
            .addRow("                   ## ")
            .addRow("                      ")
            .addRow(" #    ###   ###   #   ")
            .addRow("###  #   # #     ###  ")
            .addRow(" #   #####  ###   #   ")
            .addRow(" #   #         #  #   ")
            .addRow("  ##  ###   ###    ## "));
    t0.setCell(2, 1, CEmpty());
    oss.str("");
    oss.clear();
    oss << t0;
    assert (oss.str() ==
            "+--------------------------+----------------------+\n"
            "|Hello,                    |                      |\n"
            "|Hello Kitty               |                      |\n"
            "+--------------------------+----------------------+\n"
            "|Lorem ipsum dolor sit amet|###                   |\n"
            "|                          |#  #                  |\n"
            "|                          |#  # # ##   ###    ###|\n"
            "|                          |###  ##    #   #  #  #|\n"
            "|                          |#    #     #   #  #  #|\n"
            "|                          |#    #     #   #  #  #|\n"
            "|                          |#    #      ###    ###|\n"
            "|                          |                     #|\n"
            "|                          |                   ## |\n"
            "|                          |                      |\n"
            "|                          | #    ###   ###   #   |\n"
            "|                          |###  #   # #     ###  |\n"
            "|                          | #   #####  ###   #   |\n"
            "|                          | #   #         #  #   |\n"
            "|                          |  ##  ###   ###    ## |\n"
            "+--------------------------+----------------------+\n"
            "|                      Bye,|                      |\n"
            "|               Hello Kitty|                      |\n"
            "+--------------------------+----------------------+\n");
    t0.setCell(0, 1, t0.getCell(1, 1));
    t0.setCell(2, 1, CImage()
            .addRow("*****   *      *  *      ******* ******  *")
            .addRow("*    *  *      *  *      *            *  *")
            .addRow("*    *  *      *  *      *           *   *")
            .addRow("*    *  *      *  *      *****      *    *")
            .addRow("****    *      *  *      *         *     *")
            .addRow("*  *    *      *  *      *        *       ")
            .addRow("*   *   *      *  *      *       *       *")
            .addRow("*    *    *****   ****** ******* ******  *"));
    dynamic_cast<CText &> ( t0.getCell(1, 0)).setText("Lorem ipsum dolor sit amet,\n"
                                                      "consectetur adipiscing\n"
                                                      "elit. Curabitur scelerisque\n"
                                                      "lorem vitae lectus cursus,\n"
                                                      "vitae porta ante placerat. Class aptent taciti\n"
                                                      "sociosqu ad litora\n"
                                                      "torquent per\n"
                                                      "conubia nostra,\n"
                                                      "per inceptos himenaeos.\n"
                                                      "\n"
                                                      "Donec tincidunt augue\n"
                                                      "sit amet metus\n"
                                                      "pretium volutpat.\n"
                                                      "Donec faucibus,\n"
                                                      "ante sit amet\n"
                                                      "luctus posuere,\n"
                                                      "mauris tellus");
    oss.str("");
    oss.clear();
    oss << t0;
    assert (oss.str() ==
            "+----------------------------------------------+------------------------------------------+\n"
            "|Hello,                                        |          ###                             |\n"
            "|Hello Kitty                                   |          #  #                            |\n"
            "|                                              |          #  # # ##   ###    ###          |\n"
            "|                                              |          ###  ##    #   #  #  #          |\n"
            "|                                              |          #    #     #   #  #  #          |\n"
            "|                                              |          #    #     #   #  #  #          |\n"
            "|                                              |          #    #      ###    ###          |\n"
            "|                                              |                               #          |\n"
            "|                                              |                             ##           |\n"
            "|                                              |                                          |\n"
            "|                                              |           #    ###   ###   #             |\n"
            "|                                              |          ###  #   # #     ###            |\n"
            "|                                              |           #   #####  ###   #             |\n"
            "|                                              |           #   #         #  #             |\n"
            "|                                              |            ##  ###   ###    ##           |\n"
            "+----------------------------------------------+------------------------------------------+\n"
            "|Lorem ipsum dolor sit amet,                   |                                          |\n"
            "|consectetur adipiscing                        |          ###                             |\n"
            "|elit. Curabitur scelerisque                   |          #  #                            |\n"
            "|lorem vitae lectus cursus,                    |          #  # # ##   ###    ###          |\n"
            "|vitae porta ante placerat. Class aptent taciti|          ###  ##    #   #  #  #          |\n"
            "|sociosqu ad litora                            |          #    #     #   #  #  #          |\n"
            "|torquent per                                  |          #    #     #   #  #  #          |\n"
            "|conubia nostra,                               |          #    #      ###    ###          |\n"
            "|per inceptos himenaeos.                       |                               #          |\n"
            "|                                              |                             ##           |\n"
            "|Donec tincidunt augue                         |                                          |\n"
            "|sit amet metus                                |           #    ###   ###   #             |\n"
            "|pretium volutpat.                             |          ###  #   # #     ###            |\n"
            "|Donec faucibus,                               |           #   #####  ###   #             |\n"
            "|ante sit amet                                 |           #   #         #  #             |\n"
            "|luctus posuere,                               |            ##  ###   ###    ##           |\n"
            "|mauris tellus                                 |                                          |\n"
            "+----------------------------------------------+------------------------------------------+\n"
            "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
            "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
            "|                                              |*    *  *      *  *      *           *   *|\n"
            "|                                              |*    *  *      *  *      *****      *    *|\n"
            "|                                              |****    *      *  *      *         *     *|\n"
            "|                                              |*  *    *      *  *      *        *       |\n"
            "|                                              |*   *   *      *  *      *       *       *|\n"
            "|                                              |*    *    *****   ****** ******* ******  *|\n"
            "+----------------------------------------------+------------------------------------------+\n");
    CTable t1(t0);
    t1.setCell(1, 0, CEmpty());
    t1.setCell(1, 1, CEmpty());
    oss.str("");
    oss.clear();
    oss << t0;
    assert (oss.str() ==
            "+----------------------------------------------+------------------------------------------+\n"
            "|Hello,                                        |          ###                             |\n"
            "|Hello Kitty                                   |          #  #                            |\n"
            "|                                              |          #  # # ##   ###    ###          |\n"
            "|                                              |          ###  ##    #   #  #  #          |\n"
            "|                                              |          #    #     #   #  #  #          |\n"
            "|                                              |          #    #     #   #  #  #          |\n"
            "|                                              |          #    #      ###    ###          |\n"
            "|                                              |                               #          |\n"
            "|                                              |                             ##           |\n"
            "|                                              |                                          |\n"
            "|                                              |           #    ###   ###   #             |\n"
            "|                                              |          ###  #   # #     ###            |\n"
            "|                                              |           #   #####  ###   #             |\n"
            "|                                              |           #   #         #  #             |\n"
            "|                                              |            ##  ###   ###    ##           |\n"
            "+----------------------------------------------+------------------------------------------+\n"
            "|Lorem ipsum dolor sit amet,                   |                                          |\n"
            "|consectetur adipiscing                        |          ###                             |\n"
            "|elit. Curabitur scelerisque                   |          #  #                            |\n"
            "|lorem vitae lectus cursus,                    |          #  # # ##   ###    ###          |\n"
            "|vitae porta ante placerat. Class aptent taciti|          ###  ##    #   #  #  #          |\n"
            "|sociosqu ad litora                            |          #    #     #   #  #  #          |\n"
            "|torquent per                                  |          #    #     #   #  #  #          |\n"
            "|conubia nostra,                               |          #    #      ###    ###          |\n"
            "|per inceptos himenaeos.                       |                               #          |\n"
            "|                                              |                             ##           |\n"
            "|Donec tincidunt augue                         |                                          |\n"
            "|sit amet metus                                |           #    ###   ###   #             |\n"
            "|pretium volutpat.                             |          ###  #   # #     ###            |\n"
            "|Donec faucibus,                               |           #   #####  ###   #             |\n"
            "|ante sit amet                                 |           #   #         #  #             |\n"
            "|luctus posuere,                               |            ##  ###   ###    ##           |\n"
            "|mauris tellus                                 |                                          |\n"
            "+----------------------------------------------+------------------------------------------+\n"
            "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
            "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
            "|                                              |*    *  *      *  *      *           *   *|\n"
            "|                                              |*    *  *      *  *      *****      *    *|\n"
            "|                                              |****    *      *  *      *         *     *|\n"
            "|                                              |*  *    *      *  *      *        *       |\n"
            "|                                              |*   *   *      *  *      *       *       *|\n"
            "|                                              |*    *    *****   ****** ******* ******  *|\n"
            "+----------------------------------------------+------------------------------------------+\n");
    oss.str("");
    oss.clear();
    oss << t1;
    assert (oss.str() ==
            "+-----------+------------------------------------------+\n"
            "|Hello,     |          ###                             |\n"
            "|Hello Kitty|          #  #                            |\n"
            "|           |          #  # # ##   ###    ###          |\n"
            "|           |          ###  ##    #   #  #  #          |\n"
            "|           |          #    #     #   #  #  #          |\n"
            "|           |          #    #     #   #  #  #          |\n"
            "|           |          #    #      ###    ###          |\n"
            "|           |                               #          |\n"
            "|           |                             ##           |\n"
            "|           |                                          |\n"
            "|           |           #    ###   ###   #             |\n"
            "|           |          ###  #   # #     ###            |\n"
            "|           |           #   #####  ###   #             |\n"
            "|           |           #   #         #  #             |\n"
            "|           |            ##  ###   ###    ##           |\n"
            "+-----------+------------------------------------------+\n"
            "+-----------+------------------------------------------+\n"
            "|       Bye,|*****   *      *  *      ******* ******  *|\n"
            "|Hello Kitty|*    *  *      *  *      *            *  *|\n"
            "|           |*    *  *      *  *      *           *   *|\n"
            "|           |*    *  *      *  *      *****      *    *|\n"
            "|           |****    *      *  *      *         *     *|\n"
            "|           |*  *    *      *  *      *        *       |\n"
            "|           |*   *   *      *  *      *       *       *|\n"
            "|           |*    *    *****   ****** ******* ******  *|\n"
            "+-----------+------------------------------------------+\n");
    t1 = t0;
    t1.setCell(0, 0, CEmpty());
    t1.setCell(1, 1, CImage()
            .addRow("  ********                    ")
            .addRow(" **********                   ")
            .addRow("**        **                  ")
            .addRow("**             **        **   ")
            .addRow("**             **        **   ")
            .addRow("***         ********  ********")
            .addRow("****        ********  ********")
            .addRow("****           **        **   ")
            .addRow("****           **        **   ")
            .addRow("****      **                  ")
            .addRow(" **********                   ")
            .addRow("  ********                    "));
    oss.str("");
    oss.clear();
    oss << t0;
    assert (oss.str() ==
            "+----------------------------------------------+------------------------------------------+\n"
            "|Hello,                                        |          ###                             |\n"
            "|Hello Kitty                                   |          #  #                            |\n"
            "|                                              |          #  # # ##   ###    ###          |\n"
            "|                                              |          ###  ##    #   #  #  #          |\n"
            "|                                              |          #    #     #   #  #  #          |\n"
            "|                                              |          #    #     #   #  #  #          |\n"
            "|                                              |          #    #      ###    ###          |\n"
            "|                                              |                               #          |\n"
            "|                                              |                             ##           |\n"
            "|                                              |                                          |\n"
            "|                                              |           #    ###   ###   #             |\n"
            "|                                              |          ###  #   # #     ###            |\n"
            "|                                              |           #   #####  ###   #             |\n"
            "|                                              |           #   #         #  #             |\n"
            "|                                              |            ##  ###   ###    ##           |\n"
            "+----------------------------------------------+------------------------------------------+\n"
            "|Lorem ipsum dolor sit amet,                   |                                          |\n"
            "|consectetur adipiscing                        |          ###                             |\n"
            "|elit. Curabitur scelerisque                   |          #  #                            |\n"
            "|lorem vitae lectus cursus,                    |          #  # # ##   ###    ###          |\n"
            "|vitae porta ante placerat. Class aptent taciti|          ###  ##    #   #  #  #          |\n"
            "|sociosqu ad litora                            |          #    #     #   #  #  #          |\n"
            "|torquent per                                  |          #    #     #   #  #  #          |\n"
            "|conubia nostra,                               |          #    #      ###    ###          |\n"
            "|per inceptos himenaeos.                       |                               #          |\n"
            "|                                              |                             ##           |\n"
            "|Donec tincidunt augue                         |                                          |\n"
            "|sit amet metus                                |           #    ###   ###   #             |\n"
            "|pretium volutpat.                             |          ###  #   # #     ###            |\n"
            "|Donec faucibus,                               |           #   #####  ###   #             |\n"
            "|ante sit amet                                 |           #   #         #  #             |\n"
            "|luctus posuere,                               |            ##  ###   ###    ##           |\n"
            "|mauris tellus                                 |                                          |\n"
            "+----------------------------------------------+------------------------------------------+\n"
            "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
            "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
            "|                                              |*    *  *      *  *      *           *   *|\n"
            "|                                              |*    *  *      *  *      *****      *    *|\n"
            "|                                              |****    *      *  *      *         *     *|\n"
            "|                                              |*  *    *      *  *      *        *       |\n"
            "|                                              |*   *   *      *  *      *       *       *|\n"
            "|                                              |*    *    *****   ****** ******* ******  *|\n"
            "+----------------------------------------------+------------------------------------------+\n");
    oss.str("");
    oss.clear();
    oss << t1;
    assert (oss.str() ==
            "+----------------------------------------------+------------------------------------------+\n"
            "|                                              |          ###                             |\n"
            "|                                              |          #  #                            |\n"
            "|                                              |          #  # # ##   ###    ###          |\n"
            "|                                              |          ###  ##    #   #  #  #          |\n"
            "|                                              |          #    #     #   #  #  #          |\n"
            "|                                              |          #    #     #   #  #  #          |\n"
            "|                                              |          #    #      ###    ###          |\n"
            "|                                              |                               #          |\n"
            "|                                              |                             ##           |\n"
            "|                                              |                                          |\n"
            "|                                              |           #    ###   ###   #             |\n"
            "|                                              |          ###  #   # #     ###            |\n"
            "|                                              |           #   #####  ###   #             |\n"
            "|                                              |           #   #         #  #             |\n"
            "|                                              |            ##  ###   ###    ##           |\n"
            "+----------------------------------------------+------------------------------------------+\n"
            "|Lorem ipsum dolor sit amet,                   |                                          |\n"
            "|consectetur adipiscing                        |                                          |\n"
            "|elit. Curabitur scelerisque                   |        ********                          |\n"
            "|lorem vitae lectus cursus,                    |       **********                         |\n"
            "|vitae porta ante placerat. Class aptent taciti|      **        **                        |\n"
            "|sociosqu ad litora                            |      **             **        **         |\n"
            "|torquent per                                  |      **             **        **         |\n"
            "|conubia nostra,                               |      ***         ********  ********      |\n"
            "|per inceptos himenaeos.                       |      ****        ********  ********      |\n"
            "|                                              |      ****           **        **         |\n"
            "|Donec tincidunt augue                         |      ****           **        **         |\n"
            "|sit amet metus                                |      ****      **                        |\n"
            "|pretium volutpat.                             |       **********                         |\n"
            "|Donec faucibus,                               |        ********                          |\n"
            "|ante sit amet                                 |                                          |\n"
            "|luctus posuere,                               |                                          |\n"
            "|mauris tellus                                 |                                          |\n"
            "+----------------------------------------------+------------------------------------------+\n"
            "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
            "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
            "|                                              |*    *  *      *  *      *           *   *|\n"
            "|                                              |*    *  *      *  *      *****      *    *|\n"
            "|                                              |****    *      *  *      *         *     *|\n"
            "|                                              |*  *    *      *  *      *        *       |\n"
            "|                                              |*   *   *      *  *      *       *       *|\n"
            "|                                              |*    *    *****   ****** ******* ******  *|\n"
            "+----------------------------------------------+------------------------------------------+\n");
    CTable t2(2, 2);
    t2.setCell(0, 0, CText("OOP", CText::ALIGN_LEFT));
    t2.setCell(0, 1, CText("Encapsulation", CText::ALIGN_LEFT));
    t2.setCell(1, 0, CText("Polymorphism", CText::ALIGN_LEFT));
    t2.setCell(1, 1, CText("Inheritance", CText::ALIGN_LEFT));
    oss.str("");
    oss.clear();
    oss << t2;
    assert (oss.str() ==
            "+------------+-------------+\n"
            "|OOP         |Encapsulation|\n"
            "+------------+-------------+\n"
            "|Polymorphism|Inheritance  |\n"
            "+------------+-------------+\n");
    t1.setCell(0, 0, t2);
    dynamic_cast<CText &> ( t2.getCell(0, 0)).setText("Object Oriented Programming");
    oss.str("");
    oss.clear();
    oss << t2;
    assert (oss.str() ==
            "+---------------------------+-------------+\n"
            "|Object Oriented Programming|Encapsulation|\n"
            "+---------------------------+-------------+\n"
            "|Polymorphism               |Inheritance  |\n"
            "+---------------------------+-------------+\n");
    oss.str("");
    oss.clear();
    oss << t1;
    assert (oss.str() ==
            "+----------------------------------------------+------------------------------------------+\n"
            "|+------------+-------------+                  |          ###                             |\n"
            "||OOP         |Encapsulation|                  |          #  #                            |\n"
            "|+------------+-------------+                  |          #  # # ##   ###    ###          |\n"
            "||Polymorphism|Inheritance  |                  |          ###  ##    #   #  #  #          |\n"
            "|+------------+-------------+                  |          #    #     #   #  #  #          |\n"
            "|                                              |          #    #     #   #  #  #          |\n"
            "|                                              |          #    #      ###    ###          |\n"
            "|                                              |                               #          |\n"
            "|                                              |                             ##           |\n"
            "|                                              |                                          |\n"
            "|                                              |           #    ###   ###   #             |\n"
            "|                                              |          ###  #   # #     ###            |\n"
            "|                                              |           #   #####  ###   #             |\n"
            "|                                              |           #   #         #  #             |\n"
            "|                                              |            ##  ###   ###    ##           |\n"
            "+----------------------------------------------+------------------------------------------+\n"
            "|Lorem ipsum dolor sit amet,                   |                                          |\n"
            "|consectetur adipiscing                        |                                          |\n"
            "|elit. Curabitur scelerisque                   |        ********                          |\n"
            "|lorem vitae lectus cursus,                    |       **********                         |\n"
            "|vitae porta ante placerat. Class aptent taciti|      **        **                        |\n"
            "|sociosqu ad litora                            |      **             **        **         |\n"
            "|torquent per                                  |      **             **        **         |\n"
            "|conubia nostra,                               |      ***         ********  ********      |\n"
            "|per inceptos himenaeos.                       |      ****        ********  ********      |\n"
            "|                                              |      ****           **        **         |\n"
            "|Donec tincidunt augue                         |      ****           **        **         |\n"
            "|sit amet metus                                |      ****      **                        |\n"
            "|pretium volutpat.                             |       **********                         |\n"
            "|Donec faucibus,                               |        ********                          |\n"
            "|ante sit amet                                 |                                          |\n"
            "|luctus posuere,                               |                                          |\n"
            "|mauris tellus                                 |                                          |\n"
            "+----------------------------------------------+------------------------------------------+\n"
            "|                                          Bye,|*****   *      *  *      ******* ******  *|\n"
            "|                                   Hello Kitty|*    *  *      *  *      *            *  *|\n"
            "|                                              |*    *  *      *  *      *           *   *|\n"
            "|                                              |*    *  *      *  *      *****      *    *|\n"
            "|                                              |****    *      *  *      *         *     *|\n"
            "|                                              |*  *    *      *  *      *        *       |\n"
            "|                                              |*   *   *      *  *      *       *       *|\n"
            "|                                              |*    *    *****   ****** ******* ******  *|\n"
            "+----------------------------------------------+------------------------------------------+\n");
    assert (t0 != t1);
    assert (!(t0 == t1));
    assert (t0.getCell(1, 1) == t0.getCell(0, 1));
    assert (!(t0.getCell(1, 1) != t0.getCell(0, 1)));
    assert (t0.getCell(0, 0) != t0.getCell(0, 1));
    assert (!(t0.getCell(0, 0) == t0.getCell(0, 1)));
    t1.setCell(0, 0, t1);
    oss.str("");
    oss.clear();
    oss << t1;
    assert (oss.str() ==
            "+-------------------------------------------------------------------------------------------+------------------------------------------+\n"
            "|+----------------------------------------------+------------------------------------------+|                                          |\n"
            "||+------------+-------------+                  |          ###                             ||                                          |\n"
            "|||OOP         |Encapsulation|                  |          #  #                            ||                                          |\n"
            "||+------------+-------------+                  |          #  # # ##   ###    ###          ||                                          |\n"
            "|||Polymorphism|Inheritance  |                  |          ###  ##    #   #  #  #          ||                                          |\n"
            "||+------------+-------------+                  |          #    #     #   #  #  #          ||                                          |\n"
            "||                                              |          #    #     #   #  #  #          ||                                          |\n"
            "||                                              |          #    #      ###    ###          ||                                          |\n"
            "||                                              |                               #          ||                                          |\n"
            "||                                              |                             ##           ||                                          |\n"
            "||                                              |                                          ||                                          |\n"
            "||                                              |           #    ###   ###   #             ||                                          |\n"
            "||                                              |          ###  #   # #     ###            ||                                          |\n"
            "||                                              |           #   #####  ###   #             ||                                          |\n"
            "||                                              |           #   #         #  #             ||          ###                             |\n"
            "||                                              |            ##  ###   ###    ##           ||          #  #                            |\n"
            "|+----------------------------------------------+------------------------------------------+|          #  # # ##   ###    ###          |\n"
            "||Lorem ipsum dolor sit amet,                   |                                          ||          ###  ##    #   #  #  #          |\n"
            "||consectetur adipiscing                        |                                          ||          #    #     #   #  #  #          |\n"
            "||elit. Curabitur scelerisque                   |        ********                          ||          #    #     #   #  #  #          |\n"
            "||lorem vitae lectus cursus,                    |       **********                         ||          #    #      ###    ###          |\n"
            "||vitae porta ante placerat. Class aptent taciti|      **        **                        ||                               #          |\n"
            "||sociosqu ad litora                            |      **             **        **         ||                             ##           |\n"
            "||torquent per                                  |      **             **        **         ||                                          |\n"
            "||conubia nostra,                               |      ***         ********  ********      ||           #    ###   ###   #             |\n"
            "||per inceptos himenaeos.                       |      ****        ********  ********      ||          ###  #   # #     ###            |\n"
            "||                                              |      ****           **        **         ||           #   #####  ###   #             |\n"
            "||Donec tincidunt augue                         |      ****           **        **         ||           #   #         #  #             |\n"
            "||sit amet metus                                |      ****      **                        ||            ##  ###   ###    ##           |\n"
            "||pretium volutpat.                             |       **********                         ||                                          |\n"
            "||Donec faucibus,                               |        ********                          ||                                          |\n"
            "||ante sit amet                                 |                                          ||                                          |\n"
            "||luctus posuere,                               |                                          ||                                          |\n"
            "||mauris tellus                                 |                                          ||                                          |\n"
            "|+----------------------------------------------+------------------------------------------+|                                          |\n"
            "||                                          Bye,|*****   *      *  *      ******* ******  *||                                          |\n"
            "||                                   Hello Kitty|*    *  *      *  *      *            *  *||                                          |\n"
            "||                                              |*    *  *      *  *      *           *   *||                                          |\n"
            "||                                              |*    *  *      *  *      *****      *    *||                                          |\n"
            "||                                              |****    *      *  *      *         *     *||                                          |\n"
            "||                                              |*  *    *      *  *      *        *       ||                                          |\n"
            "||                                              |*   *   *      *  *      *       *       *||                                          |\n"
            "||                                              |*    *    *****   ****** ******* ******  *||                                          |\n"
            "|+----------------------------------------------+------------------------------------------+|                                          |\n"
            "+-------------------------------------------------------------------------------------------+------------------------------------------+\n"
            "|Lorem ipsum dolor sit amet,                                                                |                                          |\n"
            "|consectetur adipiscing                                                                     |                                          |\n"
            "|elit. Curabitur scelerisque                                                                |        ********                          |\n"
            "|lorem vitae lectus cursus,                                                                 |       **********                         |\n"
            "|vitae porta ante placerat. Class aptent taciti                                             |      **        **                        |\n"
            "|sociosqu ad litora                                                                         |      **             **        **         |\n"
            "|torquent per                                                                               |      **             **        **         |\n"
            "|conubia nostra,                                                                            |      ***         ********  ********      |\n"
            "|per inceptos himenaeos.                                                                    |      ****        ********  ********      |\n"
            "|                                                                                           |      ****           **        **         |\n"
            "|Donec tincidunt augue                                                                      |      ****           **        **         |\n"
            "|sit amet metus                                                                             |      ****      **                        |\n"
            "|pretium volutpat.                                                                          |       **********                         |\n"
            "|Donec faucibus,                                                                            |        ********                          |\n"
            "|ante sit amet                                                                              |                                          |\n"
            "|luctus posuere,                                                                            |                                          |\n"
            "|mauris tellus                                                                              |                                          |\n"
            "+-------------------------------------------------------------------------------------------+------------------------------------------+\n"
            "|                                                                                       Bye,|*****   *      *  *      ******* ******  *|\n"
            "|                                                                                Hello Kitty|*    *  *      *  *      *            *  *|\n"
            "|                                                                                           |*    *  *      *  *      *           *   *|\n"
            "|                                                                                           |*    *  *      *  *      *****      *    *|\n"
            "|                                                                                           |****    *      *  *      *         *     *|\n"
            "|                                                                                           |*  *    *      *  *      *        *       |\n"
            "|                                                                                           |*   *   *      *  *      *       *       *|\n"
            "|                                                                                           |*    *    *****   ****** ******* ******  *|\n"
            "+-------------------------------------------------------------------------------------------+------------------------------------------+\n");
    t1.setCell(0, 0, t1);
    oss.str("");
    oss.clear();
    oss << t1;
    assert (oss.str() ==
            "+----------------------------------------------------------------------------------------------------------------------------------------+------------------------------------------+\n"
            "|+-------------------------------------------------------------------------------------------+------------------------------------------+|                                          |\n"
            "||+----------------------------------------------+------------------------------------------+|                                          ||                                          |\n"
            "|||+------------+-------------+                  |          ###                             ||                                          ||                                          |\n"
            "||||OOP         |Encapsulation|                  |          #  #                            ||                                          ||                                          |\n"
            "|||+------------+-------------+                  |          #  # # ##   ###    ###          ||                                          ||                                          |\n"
            "||||Polymorphism|Inheritance  |                  |          ###  ##    #   #  #  #          ||                                          ||                                          |\n"
            "|||+------------+-------------+                  |          #    #     #   #  #  #          ||                                          ||                                          |\n"
            "|||                                              |          #    #     #   #  #  #          ||                                          ||                                          |\n"
            "|||                                              |          #    #      ###    ###          ||                                          ||                                          |\n"
            "|||                                              |                               #          ||                                          ||                                          |\n"
            "|||                                              |                             ##           ||                                          ||                                          |\n"
            "|||                                              |                                          ||                                          ||                                          |\n"
            "|||                                              |           #    ###   ###   #             ||                                          ||                                          |\n"
            "|||                                              |          ###  #   # #     ###            ||                                          ||                                          |\n"
            "|||                                              |           #   #####  ###   #             ||                                          ||                                          |\n"
            "|||                                              |           #   #         #  #             ||          ###                             ||                                          |\n"
            "|||                                              |            ##  ###   ###    ##           ||          #  #                            ||                                          |\n"
            "||+----------------------------------------------+------------------------------------------+|          #  # # ##   ###    ###          ||                                          |\n"
            "|||Lorem ipsum dolor sit amet,                   |                                          ||          ###  ##    #   #  #  #          ||                                          |\n"
            "|||consectetur adipiscing                        |                                          ||          #    #     #   #  #  #          ||                                          |\n"
            "|||elit. Curabitur scelerisque                   |        ********                          ||          #    #     #   #  #  #          ||                                          |\n"
            "|||lorem vitae lectus cursus,                    |       **********                         ||          #    #      ###    ###          ||                                          |\n"
            "|||vitae porta ante placerat. Class aptent taciti|      **        **                        ||                               #          ||                                          |\n"
            "|||sociosqu ad litora                            |      **             **        **         ||                             ##           ||                                          |\n"
            "|||torquent per                                  |      **             **        **         ||                                          ||                                          |\n"
            "|||conubia nostra,                               |      ***         ********  ********      ||           #    ###   ###   #             ||                                          |\n"
            "|||per inceptos himenaeos.                       |      ****        ********  ********      ||          ###  #   # #     ###            ||                                          |\n"
            "|||                                              |      ****           **        **         ||           #   #####  ###   #             ||                                          |\n"
            "|||Donec tincidunt augue                         |      ****           **        **         ||           #   #         #  #             ||                                          |\n"
            "|||sit amet metus                                |      ****      **                        ||            ##  ###   ###    ##           ||          ###                             |\n"
            "|||pretium volutpat.                             |       **********                         ||                                          ||          #  #                            |\n"
            "|||Donec faucibus,                               |        ********                          ||                                          ||          #  # # ##   ###    ###          |\n"
            "|||ante sit amet                                 |                                          ||                                          ||          ###  ##    #   #  #  #          |\n"
            "|||luctus posuere,                               |                                          ||                                          ||          #    #     #   #  #  #          |\n"
            "|||mauris tellus                                 |                                          ||                                          ||          #    #     #   #  #  #          |\n"
            "||+----------------------------------------------+------------------------------------------+|                                          ||          #    #      ###    ###          |\n"
            "|||                                          Bye,|*****   *      *  *      ******* ******  *||                                          ||                               #          |\n"
            "|||                                   Hello Kitty|*    *  *      *  *      *            *  *||                                          ||                             ##           |\n"
            "|||                                              |*    *  *      *  *      *           *   *||                                          ||                                          |\n"
            "|||                                              |*    *  *      *  *      *****      *    *||                                          ||           #    ###   ###   #             |\n"
            "|||                                              |****    *      *  *      *         *     *||                                          ||          ###  #   # #     ###            |\n"
            "|||                                              |*  *    *      *  *      *        *       ||                                          ||           #   #####  ###   #             |\n"
            "|||                                              |*   *   *      *  *      *       *       *||                                          ||           #   #         #  #             |\n"
            "|||                                              |*    *    *****   ****** ******* ******  *||                                          ||            ##  ###   ###    ##           |\n"
            "||+----------------------------------------------+------------------------------------------+|                                          ||                                          |\n"
            "|+-------------------------------------------------------------------------------------------+------------------------------------------+|                                          |\n"
            "||Lorem ipsum dolor sit amet,                                                                |                                          ||                                          |\n"
            "||consectetur adipiscing                                                                     |                                          ||                                          |\n"
            "||elit. Curabitur scelerisque                                                                |        ********                          ||                                          |\n"
            "||lorem vitae lectus cursus,                                                                 |       **********                         ||                                          |\n"
            "||vitae porta ante placerat. Class aptent taciti                                             |      **        **                        ||                                          |\n"
            "||sociosqu ad litora                                                                         |      **             **        **         ||                                          |\n"
            "||torquent per                                                                               |      **             **        **         ||                                          |\n"
            "||conubia nostra,                                                                            |      ***         ********  ********      ||                                          |\n"
            "||per inceptos himenaeos.                                                                    |      ****        ********  ********      ||                                          |\n"
            "||                                                                                           |      ****           **        **         ||                                          |\n"
            "||Donec tincidunt augue                                                                      |      ****           **        **         ||                                          |\n"
            "||sit amet metus                                                                             |      ****      **                        ||                                          |\n"
            "||pretium volutpat.                                                                          |       **********                         ||                                          |\n"
            "||Donec faucibus,                                                                            |        ********                          ||                                          |\n"
            "||ante sit amet                                                                              |                                          ||                                          |\n"
            "||luctus posuere,                                                                            |                                          ||                                          |\n"
            "||mauris tellus                                                                              |                                          ||                                          |\n"
            "|+-------------------------------------------------------------------------------------------+------------------------------------------+|                                          |\n"
            "||                                                                                       Bye,|*****   *      *  *      ******* ******  *||                                          |\n"
            "||                                                                                Hello Kitty|*    *  *      *  *      *            *  *||                                          |\n"
            "||                                                                                           |*    *  *      *  *      *           *   *||                                          |\n"
            "||                                                                                           |*    *  *      *  *      *****      *    *||                                          |\n"
            "||                                                                                           |****    *      *  *      *         *     *||                                          |\n"
            "||                                                                                           |*  *    *      *  *      *        *       ||                                          |\n"
            "||                                                                                           |*   *   *      *  *      *       *       *||                                          |\n"
            "||                                                                                           |*    *    *****   ****** ******* ******  *||                                          |\n"
            "|+-------------------------------------------------------------------------------------------+------------------------------------------+|                                          |\n"
            "+----------------------------------------------------------------------------------------------------------------------------------------+------------------------------------------+\n"
            "|Lorem ipsum dolor sit amet,                                                                                                             |                                          |\n"
            "|consectetur adipiscing                                                                                                                  |                                          |\n"
            "|elit. Curabitur scelerisque                                                                                                             |        ********                          |\n"
            "|lorem vitae lectus cursus,                                                                                                              |       **********                         |\n"
            "|vitae porta ante placerat. Class aptent taciti                                                                                          |      **        **                        |\n"
            "|sociosqu ad litora                                                                                                                      |      **             **        **         |\n"
            "|torquent per                                                                                                                            |      **             **        **         |\n"
            "|conubia nostra,                                                                                                                         |      ***         ********  ********      |\n"
            "|per inceptos himenaeos.                                                                                                                 |      ****        ********  ********      |\n"
            "|                                                                                                                                        |      ****           **        **         |\n"
            "|Donec tincidunt augue                                                                                                                   |      ****           **        **         |\n"
            "|sit amet metus                                                                                                                          |      ****      **                        |\n"
            "|pretium volutpat.                                                                                                                       |       **********                         |\n"
            "|Donec faucibus,                                                                                                                         |        ********                          |\n"
            "|ante sit amet                                                                                                                           |                                          |\n"
            "|luctus posuere,                                                                                                                         |                                          |\n"
            "|mauris tellus                                                                                                                           |                                          |\n"
            "+----------------------------------------------------------------------------------------------------------------------------------------+------------------------------------------+\n"
            "|                                                                                                                                    Bye,|*****   *      *  *      ******* ******  *|\n"
            "|                                                                                                                             Hello Kitty|*    *  *      *  *      *            *  *|\n"
            "|                                                                                                                                        |*    *  *      *  *      *           *   *|\n"
            "|                                                                                                                                        |*    *  *      *  *      *****      *    *|\n"
            "|                                                                                                                                        |****    *      *  *      *         *     *|\n"
            "|                                                                                                                                        |*  *    *      *  *      *        *       |\n"
            "|                                                                                                                                        |*   *   *      *  *      *       *       *|\n"
            "|                                                                                                                                        |*    *    *****   ****** ******* ******  *|\n"
            "+----------------------------------------------------------------------------------------------------------------------------------------+------------------------------------------+\n");

    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */
