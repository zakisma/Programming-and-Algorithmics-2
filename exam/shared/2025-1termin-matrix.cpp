#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cassert>

// ####################################################################
// Forward declaration
template<typename T_>
class CMatrixView;

// ####################################################################
template<typename T_>
class CMatrix {
public:
    CMatrix(const size_t h, const size_t w) : m_H(h), m_W(w), m_Version(std::make_shared<size_t>(0)) {
        for (size_t i = 0; i < h; i++)
            m_Data.emplace_back(m_W, T_());
    }

    // Copy constructor
    CMatrix(const CMatrix &other) : m_H(other.m_H), m_W(other.m_W), m_Version(std::make_shared<size_t>(0)) {
        m_Data = other.m_Data;
    }

    // Assignment operator
    CMatrix &operator=(const CMatrix &other) {
        if (this != &other) {
            m_H = other.m_H;
            m_W = other.m_W;
            m_Data = other.m_Data;
            // Increment version to invalidate existing views
            (*m_Version)++;
        }
        return *this;
    }

    // operator () to access elements
    T_ &operator()(size_t row, size_t col) {
        if (row >= m_H || col >= m_W)
            throw std::out_of_range("out of range");
        return m_Data[row][col];
    }

    CMatrixView<T_> createView(ssize_t rowOffset, ssize_t colOffset) {
        return CMatrixView<T_>(this, rowOffset, colOffset, m_Version);
    }

    size_t m_H;
    size_t m_W;
    std::vector<std::vector<T_>> m_Data;
    std::shared_ptr<size_t> m_Version; // Version counter for invalidating views
};

// ####################################################################
template<typename T_>
class CMatrixView {
public:
    // Constructor
    CMatrixView(CMatrix<T_> *matrix, ssize_t rowOffset, ssize_t colOffset, std::shared_ptr<size_t> version)
            : m_Matrix(matrix), m_RowOffset(rowOffset), m_ColOffset(colOffset),
              m_Version(version), m_OriginalVersion(*version) {}

    // Copy constructor
    CMatrixView(const CMatrixView &other)
            : m_Matrix(other.m_Matrix), m_RowOffset(other.m_RowOffset),
              m_ColOffset(other.m_ColOffset), m_Version(other.m_Version),
              m_OriginalVersion(other.m_OriginalVersion) {}

    // Assignment operator
    CMatrixView &operator=(const CMatrixView &other) {
        if (this != &other) {
            m_Matrix = other.m_Matrix;
            m_RowOffset = other.m_RowOffset;
            m_ColOffset = other.m_ColOffset;
            m_Version = other.m_Version;
            m_OriginalVersion = other.m_OriginalVersion;
        }
        return *this;
    }

    // operator () to access elements
    T_ &operator()(ssize_t rowOffset, ssize_t colOffset) {
        // Check if the matrix still exists and hasn't been modified
        auto version_ptr = m_Version.lock();
        if (!version_ptr || *version_ptr != m_OriginalVersion) {
            throw std::logic_error("Matrix has been destroyed or modified");
        }

        // Calculate actual indices in the matrix
        ssize_t actualRow = m_RowOffset + rowOffset;
        ssize_t actualCol = m_ColOffset + colOffset;

        // Check bounds
        if (actualRow < 0 || actualCol < 0 ||
            actualRow >= static_cast<ssize_t>(m_Matrix->m_H) ||
            actualCol >= static_cast<ssize_t>(m_Matrix->m_W)) {
            throw std::out_of_range("out of range");
        }

        return m_Matrix->m_Data[actualRow][actualCol];
    }

private:
    CMatrix<T_> *m_Matrix;
    ssize_t m_RowOffset;
    ssize_t m_ColOffset;
    std::weak_ptr<size_t> m_Version; // Weak pointer to detect matrix destruction
    size_t m_OriginalVersion; // Original version when view was created
};

// ####################################################################
int main() {
    bool exceptionThrown = false;

    CMatrix<int> m1(3, 3);

    m1(0, 0) = 100;
    m1(0, 1) = 200;
    m1(2, 2) = 400;
    m1(1, 1) = 0;

    assert(m1(0, 0) == 100);
    assert(m1(1, 1) == 0);
    assert(m1(2, 2) == 400);

    CMatrixView<int> v1 = m1.createView(1, 1);

    assert(v1(-1, -1) == 100);
    v1(-1, -1) = 50;
    assert(v1(-1, -1) == 50);

    exceptionThrown = false;
    try {
        v1(2, 2) = 0;
    } catch (const std::out_of_range &) {
        exceptionThrown = true;
    }
    assert(exceptionThrown);

    CMatrix<int> m2(4, 5);

    m2(1, 3) = 100;
    m2(0, 2) = 200;
    m2(3, 4) = 400;
    m2(0, 1) = 600;

    assert(m2(1, 3) == 100);
    assert(m2(0, 1) == 600);
    assert(m2(3, 4) == 400);

    CMatrixView<int> v2 = m2.createView(-2, 5);

    assert(v2(2, -3) == 200);
    assert(v2(5, -1) == 400);

    CMatrixView<int> v3(v2);

    assert(v3(2, -3) == 200);
    assert(v3(5, -1) == 400);

    v3 = v1;

    assert(v3(-1, -1) == 50);

    m2 = m1;

    exceptionThrown = false;
    try {
        v2(2, 2) = 0;
    } catch (const std::logic_error &) {
        exceptionThrown = true;
    }
    assert(exceptionThrown);

    return EXIT_SUCCESS;
}