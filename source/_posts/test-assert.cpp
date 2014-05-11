#include <iostream>
#include <cmath>
#include <cassert>

namespace my {
    double sqrt(double n) {
        assert(n >=0);
        return std::sqrt(n);
    }

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
