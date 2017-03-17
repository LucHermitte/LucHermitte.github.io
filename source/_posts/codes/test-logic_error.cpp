#include <vector>
#include <iostream>
#include <stdexcept>

void dispatch_error() {
    try {
        throw;
#if 0
#if defined(NDEBUG)
    } catch (std::logic_error const& e) {
        std::cerr << "logic: " << e.what() << "\n";
        throw;
#endif
    } catch (std::exception const& e) {
        std::cerr << "all  : " << e.what() << "\n";
#else
    } catch (std::runtime_error const& e) {
        std::cerr << "RT   : " << e.what() << "\n";
#endif
    }
}

int main ()
{
    std::vector<int> v { 1,2,3 };
    try {
        std::cout << v.at(10);
    } catch (std::logic_error const& e) {
        std::cerr << "logic: " << e.what() << "\n";
        throw;
    } catch (...) {
        dispatch_error();
    }
}
// Vim: let $CXXFLAGS='-std=c++0x -DNDEBUG -g'
