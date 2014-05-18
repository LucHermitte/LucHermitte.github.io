// test-assert.cpp
#include <iostream>
#include <cmath>
#include <cassert>
#include <limits>

namespace my {
    /** Computes square root.
     * @pre \c n must be positive, checked with an assertion
     * @post <tt>result * result == n</tt>, checked with an assertion
     *
     */
    double sqrt(double n) {
        assert(n >=0);
        const double result = std::sqrt(n);
        assert(std::abs(result*result - n) < std::numeric_limits<double>::epsilon() * 100);
    }

    /** Computes sinus
     * @post \c n belongs to [-1, 1], unchecked
     */
    double sin(double n) {
        const double r = std::sin(n);
        assert(r <= 1 && r >= -1);
        return r;
    }
} // my namespace


int main ()
{
    std::cout << my::sqrt(my::sin(0)-1) << std::endl;
}
// Vim: let $CXXFLAGS='-g'
