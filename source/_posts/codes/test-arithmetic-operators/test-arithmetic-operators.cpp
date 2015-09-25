#include "test-arithmetic-operators.h"
#include <iostream>

inline
#if METHOD_LEFT == 1
Vector operator+(Vector lhs, Vector const& rhs) {
    std::cout << "  before return\n";
    return lhs += rhs;
}
#if 0
// ambiguous overload
Vector operator+(Vector &&lhs, Vector const& rhs) {
    std::cout << "  before return\n";
    return lhs += rhs;
}
#endif
#elif METHOD_LEFT == 2
Vector operator+(Vector const& lhs, Vector const& rhs) {
    std::cout << "  before return\n";
    return Vector(lhs) += rhs;
}
#elif METHOD_LEFT == 3
Vector operator+(Vector lhs, Vector const& rhs) {
    lhs += rhs;
    std::cout << "  before return\n";
    return lhs;
}
// Cannot add a
//   Vector operator+(Vector &&lhs, Vector const& rhs);
// either
#elif METHOD_LEFT == 4
Vector operator+(Vector const& lhs, Vector const& rhs) {
    Vector res = Vector(lhs) += rhs;
    std::cout << "  before return\n";
    return res;
}
#elif METHOD_LEFT == 5
Vector operator+(Vector const& lhs, Vector const& rhs) {
    Vector res(lhs);
    res += rhs;
    std::cout << "  before return\n";
    return res;
}
#elif METHOD_LEFT == 6
Vector operator+(Vector const& lhs, Vector const& rhs) {
    Vector res(lhs);
    res += rhs;
    std::cout << "  before return (const&, const&)\n";
    return res;
}
Vector operator+(Vector && lhs, Vector const& rhs) {
    lhs += rhs;
    std::cout << "  before return (&&, const&)\n";
    return lhs;
}
#elif METHOD_LEFT == 7
Vector operator+(Vector const& lhs, Vector const& rhs) {
    auto res = Vector(lhs);
    res += rhs;
    std::cout << "  before return (const&, const&)\n";
    return res;
}
Vector operator+(Vector && lhs, Vector const& rhs) {
    lhs += rhs;
    std::cout << "  before return (&&, const&)\n";
    // force move as RVO cannot apply
    return std::move(lhs);
}
#elif METHOD_LEFT == 8
Vector operator+(Vector const& lhs, Vector const& rhs) {
    auto res = Vector(lhs);
    res += rhs;
    std::cout << "  before return (const&, const&)\n";
    return res;
}
Vector operator+(Vector const& lhs, Vector && rhs) {
    rhs += lhs;
    std::cout << "  before return (const&, &&)\n";
    // force move as RVO cannot apply
    return std::move(rhs);
}
#elif METHOD_LEFT == 9
Vector operator+(Vector const& lhs, Vector const& rhs) {
    // auto res = Vector(lhs);
    Vector res(lhs);
    res += rhs;
    std::cout << "  before return (const&, const&)\n";
    return res;
}
Vector operator+(Vector && lhs, Vector const& rhs) {
    lhs += rhs;
    std::cout << "  before return (&&, const&)\n";
    return std::move(lhs);
}
Vector operator+(Vector const& lhs, Vector && rhs) {
    rhs += lhs;
    std::cout << "  before return (const&, &&)\n";
    // force move as RVO cannot apply
    return std::move(rhs);
}
Vector operator+(Vector && lhs, Vector && rhs) {
    lhs += rhs;
    std::cout << "  before return (&&, &&)\n";
    return std::move(lhs);
}
#endif

Vector f(Observer & obs) {
    std::cout << "f(obs)\n";
    return Vector(obs);
}

int main ()
{
    Observer obs;
    Vector a(obs);
    Vector b(obs);
    Vector c(obs);
    Vector d(obs);
    {
        Focus focus(obs, "lvalue + lvalue");
        Vector r = a + b;
        std::cout << "  result obtained\n";
    }
    {
        Focus focus(obs, "lvalue + lvalue + lvalue");
        Vector r = a + b + c;
        std::cout << "  result obtained\n";
    }
    {
        Focus focus(obs, "lvalue + lvalue + lvalue + lvalue");
        Vector r = a + b + c + d;
        std::cout << "  result obtained\n";
    }
    {
        Focus focus(obs, "(lvalue + lvalue) + lvalue");
        Vector r = (a + b) + c;
        std::cout << "  result obtained\n";
    }
    {
        Focus focus(obs, "(lvalue + lvalue) + (lvalue + lvalue)");
        Vector r = (a + b) + (c + d);
        std::cout << "  result obtained\n";
    }
    {
        Focus focus(obs, "lvalue + f()");
        Vector r = a + f(obs);
        std::cout << "  result obtained\n";
    }
    {
        Focus focus(obs, "f() + lvalue");
        Vector r = f(obs) + a;
        std::cout << "  result obtained\n";
    }
    {
        Focus focus(obs, "f() + f()");
        Vector r = f(obs) + f(obs);
        std::cout << "  result obtained\n";
    }
}
// Vim: let $CXXFLAGS='-std=c++11 -O3 -pedantic -Wall -DMETHOD_LEFT=5'
// Vim: let $CXXFLAGS='-O3 -pedantic -Wall -DMETHOD_LEFT=5'
