#include <iostream>
#include <cassert>
#include <limits>
#if 1
// Compiles with clang 3.6
// Doesn't compile with g++ 4.8.2, nor 4.9
//
/**
 * Internal constexpr function that computes \f$n!\f$ with a tail-recursion.
 * @param[in] n  
 * @param[in] r  pre-computed result
 * @pre n shall not induce an integer overflow
 * @post the result won't be null
 */
constexpr unsigned int fact_impl(unsigned int n, unsigned int r) {
    return n >= 1
        // ? (assert(std::numeric_limits<decltype(n)>::max()/n >= r), fact_impl(n-1, n*r))
        ? fact_impl((assert(std::numeric_limits<decltype(n)>::max()/n >= r), n-1), n*r)
        : (assert(r>0), r);
}
#else
// Copyright 2014 Eric Niebler,
// http://ericniebler.com/2014/09/27/assert-and-constexpr-in-cxx11/
struct assert_failure
{
    template<typename Fun>
    explicit assert_failure(Fun fun)
    {
        fun();
        // For good measure:
        std::quick_exit(EXIT_FAILURE);
    }
};

constexpr unsigned int fact_impl(unsigned int n, unsigned int r) {
    return
        n <= 1                                          ? r
#ifndef NDEBUG
        : std::numeric_limits<decltype(n)>::max()/n < r ? throw assert_failure( []{assert(!"int overflow");})
#endif
        :                                                 fact_impl(n-1, n*r)
        ;
}
#endif

constexpr unsigned int fact(unsigned int n) {
    return fact_impl(n, 1);
}

int main ()
{
    std::cout << fact(10) << "\n";
    // std::cout << fact(50) << "\n"; // won't be detected at compilation time, but at execution time

    for (unsigned int i; std::cout << "i ?" && std::cin >> i;) {
        std::cout << fact(i) << "\n";
    }
}
// Vim: let $CXXFLAGS='-std=c++0x -g'
