#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cmath>
#include <cfloat>
#include <cassert>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <span>
#include <algorithm>
#include <memory>
#include <compare>
#include <complex>
#endif /* __PROGTEST__ */

// keep this dummy version if you do not implement a real manipulator
std::ios_base & ( * poly_var ( const std::string & name ) ) ( std::ios_base & x )
{
    return [] ( std::ios_base & ios ) -> std::ios_base & { return ios; };
}
using namespace std;

class CPolynomial {
public:
    CPolynomial() = default;

    double operator[](size_t i) const {
        return i < m_coeff.size() ? m_coeff[i] : 0.0;
    }

    double& operator[](size_t i) {
        if (i >= m_coeff.size())
            m_coeff.resize(i + 1, 0.0);
        return m_coeff[i];
    }

    double operator()(double x) const {
        double output = 0.0;
        for (int i = static_cast<int>(m_coeff.size()) - 1; i >= 0; i--)
            output = output * x + m_coeff[i];
        return output;
    }

    CPolynomial operator*(double scalar) const {
        CPolynomial output = *this;
        output *= scalar;
        return output;
    }

    CPolynomial operator*(const CPolynomial& other) const {
        size_t degThis = degree();
        size_t degOther = other.degree();

        if (degThis == 0 && m_coeff.empty()) return CPolynomial();
        if (degOther == 0 && other.m_coeff.empty()) return CPolynomial();

        size_t resultDegree = degThis + degOther;

        if (degThis > 10000 && degOther > 10000) {
            return multiplyLarge(other);
        }

        CPolynomial output;
        output.m_coeff.resize(resultDegree + 1, 0.0);

        for (size_t i = 0; i <= degThis; i++) {
            if (m_coeff[i] == 0.0) continue;

            for (size_t j = 0; j <= degOther; j++) {
                if (other.m_coeff[j] == 0.0) continue;

                output.m_coeff[i + j] += m_coeff[i] * other.m_coeff[j];
            }
        }

        output.normalize();
        return output;
    }

    CPolynomial& operator*=(double scalar) {
        for (auto& coef : m_coeff)
            coef *= scalar;

        normalize();
        return *this;
    }

    CPolynomial& operator*=(const CPolynomial& other) {
        *this = *this * other;
        return *this;
    }

    bool operator==(const CPolynomial& other) const {
        normalize();
        other.normalize();

        if (m_coeff.size() != other.m_coeff.size())
            return false;

        for (size_t i = 0; i < m_coeff.size(); i++)
            if (m_coeff[i] != other.m_coeff[i])
                return false;

        return true;
    }

    bool operator!=(const CPolynomial& other) const {
        return !(*this == other);
    }

    explicit operator bool() const {
        for (const auto& coef : m_coeff)
            if (coef != 0.0)
                return true;
        return false;
    }

    bool operator!() const {
        return !static_cast<bool>(*this);
    }

    size_t degree() const {
        normalize();
        return m_coeff.empty() ? 0 : m_coeff.size() - 1;
    }

    friend std::ostream& operator<<(std::ostream& os, const CPolynomial& poly);

private:
    std::vector<double> m_coeff;

    void normalize() const {
        auto& coeffs = const_cast<std::vector<double>&>(m_coeff);
        while (!coeffs.empty() && coeffs.back() == 0.0)
            coeffs.pop_back();
    }

    CPolynomial multiplyLarge(const CPolynomial& other) const {
        size_t degThis = degree();
        size_t degOther = other.degree();
        size_t resultDegree = degThis + degOther;

        CPolynomial output;
        output.m_coeff.resize(resultDegree + 1, 0.0);

        const auto& a = m_coeff;
        const auto& b = other.m_coeff;
        auto& c = output.m_coeff;

        if (degThis < degOther / 100) {
            for (size_t i = 0; i <= degThis; i++) {
                if (a[i] == 0.0) continue;

                for (size_t j = 0; j <= degOther; j++) {
                    if (b[j] == 0.0) continue;
                    c[i + j] += a[i] * b[j];
                }
            }
        }
        else {
            for (size_t i = 0; i <= degThis; i++) {
                if (a[i] == 0.0) continue;

                for (size_t j = 0; j <= degOther; j++) {
                    if (b[j] == 0.0) continue;
                    c[i + j] += a[i] * b[j];
                }
            }
        }

        output.normalize();
        return output;
    }
};

std::ostream& operator<<(std::ostream& os, const CPolynomial& poly) {
    poly.normalize();

    static const int i = std::ios_base::xalloc();
    std::string varName = "x";
    if (void* p = os.pword(i)) {
        varName = *reinterpret_cast<std::string*>(p);
    }

    if (poly.m_coeff.empty()) {
        os << "0";
        return os;
    }

    bool firstTerm = true;
    for (int i = static_cast<int>(poly.m_coeff.size()) - 1; i >= 0; i--) {
        double coef = poly.m_coeff[i];
        if (coef == 0.0) continue;

        if (coef > 0 && !firstTerm) os << " + ";
        else if (coef < 0) {
            if (firstTerm) os << "- ";
            else os << " - ";
            coef = -coef;
        }

        firstTerm = false;

        if (i == 0 || coef != 1.0) {
            if (i == 0) os << coef;
            else if (coef == 1.0) {
                /*  nothing for coefficient 1 */
            }
            else os << coef << "*";
        }

        if (i > 0) {
            os << varName;
            if (i > 0) os << "^" << i;
        }
    }

    return os;
}


#ifndef __PROGTEST__
bool smallDiff(double a, double b)
{
    if (a == b)
        return true;

    double diff = std::abs(a - b);
    double absA = std::abs(a);
    double absB = std::abs(b);
    double largest = (absA > absB) ? absA : absB;

    // According to the prompt, tolerance is 0.1‰ (0.001 or 0.1%)
    return diff <= largest * 0.001;
}

bool dumpMatch(const CPolynomial& x, const std::vector<double>& ref)
{
    for (size_t i = 0; i < ref.size(); i++)
    {
        if (!smallDiff(x[i], ref[i]))
            return false;
    }

    for (size_t i = ref.size(); i <= x.degree(); i++)
    {
        if (!smallDiff(x[i], 0.0))
            return false;
    }

    return true;
}

int main ()
{
    CPolynomial a, b, c;
    std::ostringstream out, tmp;

    a[0] = -10;
    a[1] = 3.5;
    a[3] = 1;
    assert ( smallDiff ( a ( 2 ), 5 ) );
    out . str ("");
    out << a;
    assert ( out . str () == "x^3 + 3.5*x^1 - 10" );
    c = a * -2;
    assert ( c . degree () == 3
             && dumpMatch ( c, std::vector<double>{ 20.0, -7.0, -0.0, -2.0 } ) );

    out . str ("");
    out << c;
    assert ( out . str () == "- 2*x^3 - 7*x^1 + 20" );
    out . str ("");
    out << b;
    assert ( out . str () == "0" );
    b[5] = -1;
    b[2] = 3;
    out . str ("");
    out << b;
    assert ( out . str () == "- x^5 + 3*x^2" );
    c = a * b;
    assert ( c . degree () == 8
             && dumpMatch ( c, std::vector<double>{ -0.0, -0.0, -30.0, 10.5, -0.0, 13.0, -3.5, 0.0, -1.0 } ) );

    out . str ("");
    out << c;
    assert ( out . str () == "- x^8 - 3.5*x^6 + 13*x^5 + 10.5*x^3 - 30*x^2" );
    a *= 5;
    assert ( a . degree () == 3
             && dumpMatch ( a, std::vector<double>{ -50.0, 17.5, 0.0, 5.0 } ) );

    a *= b;
    assert ( a . degree () == 8
             && dumpMatch ( a, std::vector<double>{ 0.0, 0.0, -150.0, 52.5, -0.0, 65.0, -17.5, -0.0, -5.0 } ) );

    assert ( a != b );
    b[5] = 0;
    assert ( static_cast<bool> ( b ) );
    assert ( ! ! b );
    b[2] = 0;
    assert ( !(a == b) );
    a *= 0;
    assert ( a . degree () == 0
             && dumpMatch ( a, std::vector<double>{ 0.0 } ) );

    assert ( a == b );
    assert ( ! static_cast<bool> ( b ) );
    assert ( ! b );

    // bonus - manipulators

    out . str ("");
    out << poly_var ( "y" ) << c;
    assert ( out . str () == "- y^8 - 3.5*y^6 + 13*y^5 + 10.5*y^3 - 30*y^2" );
    out . str ("");
    tmp << poly_var ( "abc" );
    out . copyfmt ( tmp );
    out << c;
    assert ( out . str () == "- abc^8 - 3.5*abc^6 + 13*abc^5 + 10.5*abc^3 - 30*abc^2" );
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
