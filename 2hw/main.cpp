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
using namespace std;

ios_base &( *poly_var(const string &name))(ios_base &x) {
    return [](ios_base &ios) -> ios_base & { return ios; };
}
//---------------------------------------------------------------------------------

/*
 * Class CPolynomial
 * ----------------
 * Represents a polynomial with real coefficients.
 * Supports operations such as evaluation, multiplication, comparison,
 *      coefficient access, and formatted output.
 * Coefficients are stored in a vector where the index represents the power of x.
 */
class CPolynomial {
public:
    /*
     * Operator []
     * ----------
     * Provides read-only access to the coefficient at the specified index.
     *
     * @param index    The exponent of x whose coefficient is requested.
     * @return         The coefficient of x^index.
     */
    double operator[](size_t index) const {
        return index < m_coeff.size() ? m_coeff[index] : 0.0;
    }

    /*
     * Operator []
     * ----------
     * Provides read-write access to the coefficient at the specified index.
     * Resizes the coefficient vector if necessary.
     *
     * @param index    The exponent of x whose coefficient is to be accessed or modified.
     * @return         Reference to the coefficient of x^index.
     */
    double &operator[](size_t index) {
        if (index >= m_coeff.size()) {
            m_coeff.resize(index * 2 + 1, 0.0);
        }
        return m_coeff[index];
    }

    /*
     * Operator ()
     * ----------
     * Evaluates the polynomial for a given x, using Horner's method for efficiency
     *
     * @param x        The value at which to evaluate the polynomial
     * @return         The result of evaluating the polynomial at x
     */
    double operator()(double x) const {
        double res = 0.0;
        for (int i = static_cast<int>(m_coeff.size()) - 1; i >= 0; i--){
            res = res * x + m_coeff[i];
        }
        return res;
    }

    /*
     * Operator *=
     * ----------
     * Multiplies the polynomial by a scalar value.
     *
     * @param num      The scalar value to multiply by.
     * @return         Reference to the modified polynomial.
     */
    CPolynomial &operator*=(double num) {
        for (auto &coeff: m_coeff) {
            coeff *= num;
        }
        return *this;
    }

    /*
    * Operator *=
    * ----------
    * Multiplies the polynomial by another polynomial.
    * Modifies the polynomial in-place by computing the product
    *       and assigning it to this polynomial.
    *
    * @param pol      The polynomial to multiply by.
    * @return         Reference to the modified polynomial.
    */
    CPolynomial &operator*=(const CPolynomial &pol) {
        *this = *this * pol;
        return *this;
    }

    /*
     * Operator *
     * ---------
     * Creates a new polynomial that is the product of this polynomial and a scalar
     *
     * @param num      The scalar value to multiply by.
     * @return         A new polynomial representing the product
     */
    CPolynomial operator*(double num) const {
        CPolynomial res = *this;
        res *= num;
        return res;
    }

    /*
     * Operator *
     * ---------
     * Creates a new polynomial that is the product of this polynomial and another polynomial.
     * Checks if degrees are not zero before multiplying.
     * Multiplies coefficients respectively and stores the result in a new polynomial
     *
     * @param pol      The polynomial to multiply by.
     * @return         A new polynomial representing the product.
     */
    CPolynomial operator*(const CPolynomial &pol) const {
        CPolynomial res;
        size_t degree1 = degree();
        size_t degree2 = pol.degree();

        if (degree1 == 0 && m_coeff.empty()) return CPolynomial();
        if (degree2 == 0 && pol.m_coeff.empty()) return CPolynomial();
        size_t res_degree = degree1 + degree2;
        res.m_coeff.resize(res_degree + 1, 0.0);

        for (size_t i = 0; i <= degree1; i++) {
            if (m_coeff[i] != 0) {
                for (size_t j = 0; j <= degree2; j++) {
                    if (pol.m_coeff[j] != 0) {
                        res.m_coeff[i + j] += m_coeff[i] * pol.m_coeff[j];
                    }
                }
            }
        }
        return res;
    }

    /*
     * Operator !=
     * ----------
     * Checks if this polynomial is not equal to another polynomial.
     *
     * @param other    The polynomial to compare with.
     * @return         True if polynomials are not equal, false otherwise.
     */
    bool operator!=(const CPolynomial &other) const {
        return !(*this == other);
    }

    /*
     * Operator ==
     * ----------
     * Checks if this polynomial is equal to another polynomial.
     * Two polynomials are equal if they have the same degree and
     *      matching coefficients for all powers.
     *
     * @param pol      The polynomial to compare with.
     * @return         True if polynomials are equal, false otherwise.
     */
    bool operator==(const CPolynomial &pol) const {
        size_t degree1 = degree();
        size_t degree2 = pol.degree();
        if (degree1 != degree2) {
            return false;
        }

        for (size_t i = 0; i < m_coeff.size(); i++) {
            if (m_coeff[i] != pol.m_coeff[i]) {
                return false;
            }
        }
        return true;
    }

    /*
    * Operator bool
    * ------------
    * Converts the polynomial to a boolean value.
    * Returns true if the polynomial has at least one non-zero coefficient
    * explicit is used to prevent implicit conversions to bool
    *
    * @return         True if polynomial is non-zero, false otherwise.
    */
    explicit operator bool() const {
        for (const auto &coeff: m_coeff)
            if (coeff != 0.0)
                return true;
        return false;
    }

    /*
     * Operator !
     * ---------
     * Checks if all coefficients of the polynomial are zero.
     *
     * @return         True if polynomial is zero, false otherwise.
     */
    bool operator!() const {
        return !static_cast<bool>(*this);
    }

    /*
     * Function degree
     * --------------
     * Determines the degree of the polynomial, which is the highest power
     *      with a non-zero coefficient.
     * Returns 0 for the zero polynomial.
     *
     * @return         The degree of the polynomial.
     */
    size_t degree() const {
        for (int i = static_cast<int>(m_coeff.size()) - 1; i >= 0; i--) {
            if (m_coeff[i] != 0.0) {
                return i;
            }
        }
        return 0;
    }

    /*
     * Operator
     * ----------
     * Formats the polynomial for output to a stream according to the specified rules:
     * - Terms are displayed from highest to lowest power
     * - Terms with zero coefficients are not displayed
     * - Terms with coefficients of +1 or -1 do not show the number 1
     * - Proper signs are used between terms , which means no excessive negative signs
     * - The zero polynomial is displayed as "0"
     *
     * @param os       The output stream to write to.
     * @param pol      The polynomial to output.
     * @return         Reference to the output stream.
     */
    friend ostream &operator<<(ostream &os, const CPolynomial &pol) {
        string variable_name = "x";
        if (pol.m_coeff.empty()) {
            os << "0";
            return os;
        }
        bool first_term = true;
        bool forward_zeroes = true;
        for (int i = static_cast<int>(pol.m_coeff.size()) - 1; i >= 0; i--) {
            if (pol.m_coeff[i] == 0 && forward_zeroes) {
                continue;
            }
            forward_zeroes = false;
            double coeff = pol.m_coeff[i];
            if (coeff == 0.0) continue;

            if (coeff > 0 && !first_term) os << " + ";
            else if (coeff < 0) {
                if (first_term) os << "- ";
                else os << " - ";
                coeff = -coeff;
            }

            first_term = false;
            if (i == 0 || coeff != 1.0) {
                if (i == 0) os << coeff;
                else if (coeff == 1.0) {
                    /*  nothing for coefficient 1 */
                } else os << coeff << "*";
            }
            if (i > 0) {
                os << variable_name;
                if (i > 0) os << "^" << i;
            }
        }
        return os;
    }

private:
    //Stores the coefficients of the polynomial.
    vector<double> m_coeff; // The index in the vector represents the power of x.

};


#ifndef __PROGTEST__

bool smallDiff(double a, double b) {
    if (a == b)
        return true;

    double diff = abs(a - b);
    double absA = abs(a);
    double absB = abs(b);
    double largest = (absA > absB) ? absA : absB;

    return diff <= largest * 0.001;
}

bool dumpMatch(const CPolynomial &x, const vector<double> &ref) {
    for (size_t i = 0; i < ref.size(); i++) {
        if (!smallDiff(x[i], ref[i]))
            return false;
    }

    for (size_t i = ref.size(); i <= x.degree(); i++) {
        if (!smallDiff(x[i], 0.0))
            return false;
    }

    return true;
}

int main() {
    CPolynomial a, b, c;
    ostringstream out, tmp;

    a[0] = -10;
    a[1] = 3.5;
    a[3] = 1;
    assert (smallDiff(a(2), 5));
    out.str("");
    out << a;
    assert (out.str() == "x^3 + 3.5*x^1 - 10");
    c = a * -2;
    assert (c.degree() == 3
            && dumpMatch(c, vector<double>{20.0, -7.0, -0.0, -2.0}));

    out.str("");
    out << c;
    assert (out.str() == "- 2*x^3 - 7*x^1 + 20");
    out.str("");
    out << b;
    assert (out.str() == "0");
    b[5] = -1;
    b[2] = 3;
    out.str("");
    out << b;
    assert (out.str() == "- x^5 + 3*x^2");
    c = a * b;
    assert (c.degree() == 8
            && dumpMatch(c, vector<double>{-0.0, -0.0, -30.0, 10.5, -0.0, 13.0, -3.5, 0.0, -1.0}));

    out.str("");
    out << c;
    assert (out.str() == "- x^8 - 3.5*x^6 + 13*x^5 + 10.5*x^3 - 30*x^2");
    a *= 5;
    assert (a.degree() == 3
            && dumpMatch(a, vector<double>{-50.0, 17.5, 0.0, 5.0}));

    a *= b;
    assert (a.degree() == 8
            && dumpMatch(a, vector<double>{0.0, 0.0, -150.0, 52.5, -0.0, 65.0, -17.5, -0.0, -5.0}));

    assert (a != b);
    b[5] = 0;
    assert (static_cast<bool> ( b ));
    assert (!!b);
    b[2] = 0;
    assert (!(a == b));
    a *= 0;
    assert (a.degree() == 0
            && dumpMatch(a, vector<double>{0.0}));

    assert (a == b);
    assert (!static_cast<bool> ( b ));
    assert (!b);

    // bonus - manipulators

//    out.str("");
//    out << poly_var("y") << c;
//    assert (out.str() == "- y^8 - 3.5*y^6 + 13*y^5 + 10.5*y^3 - 30*y^2");
//    out.str("");
//    tmp << poly_var("abc");
//    out.copyfmt(tmp);
//    out << c;
//    assert (out.str() == "- abc^8 - 3.5*abc^6 + 13*abc^5 + 10.5*abc^3 - 30*abc^2");
    return EXIT_SUCCESS;
}

#endif /* __PROGTEST__ */
