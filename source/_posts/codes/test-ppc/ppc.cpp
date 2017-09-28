#include <stdexcept>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cmath>
#include <cassert>

void treat(double d)
{
    std::cout << d << "\n";
}

namespace my {
double sqrt(double n) {
    assert(n>=0 && "sqrt can't process negative numbers");
    return std::sqrt(n);
}
} // my namespace

int main()
{
    const auto file = "distances.txt";
    try
    {
        std::ifstream f(file);
        if (!f) {
            throw std::runtime_error("Cannot open distances.txt");
        }
        double d;
        for (std::size_t l = 1 ; f >> d ; ++l) {
            if (d <= 0) 
                throw std::runtime_error(
                        "Invalid negative distance " + std::to_string(d)
                        +" at the "+std::to_string(l)
                        +"th line in distances file "+file);
            const auto sq = my::sqrt(d);
            treat(sq);
        }
        return EXIT_SUCCESS;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return EXIT_FAILURE;
}
// Vim: let $CXXFLAGS='-std=c++14 -g'


