#include <stdexcept>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cmath>

void treat(double d)
{
    std::cout << d << "\n";
}

namespace my {
double sqrt(double n) {
    if (n<0) throw std::domain_error("Negative number sent to sqrt");
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
            double sq ;
            try {
                sq = my::sqrt(d);
            }
            catch (std::logic_error const&) {
                throw std::runtime_error(
                        "Invalid negative distance " + std::to_string(d)
                        +" at the "+std::to_string(l)
                        +"th line in distances file "+file);
            }
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

