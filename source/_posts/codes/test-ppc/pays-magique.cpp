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

int main()
{
    try
    {
        std::ifstream f("distances.txt");
        if (!f) {
            throw std::runtime_error("Cannot open distances.txt");
        }
        double d;
        while (f >> d) {
            const auto sq = std::sqrt(d);
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
// Vim: let $CXXFLAGS='-std=c++14 -g -fno-omit-frame-pointer -fsanitize=undefined'
