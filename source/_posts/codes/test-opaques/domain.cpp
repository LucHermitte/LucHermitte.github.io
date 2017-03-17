#include "domain.h"
#include <limits>
#include <iostream>

using namespace literals;
double g(decltype(3.5_c) v)
{
    return literals::value(v);
}

using positive = domain<double, decltype(0.55_c),  decltype(20000._c)>;
using negative = domain<double, decltype(-20000._c),  decltype(0._c)>;
using zero = decltype(0_c);

double f(domain<double, zero, decltype(10_c)> x)
{ return x.value(); }
double h(positive x)
{ return x.value(); }

int main ()
{
    std::cout << g(3.5_c) << "\n";
    // std::cout << f(4.5_c);

    negative n(-5);
    positive p(5);
    std::cout << p - n << "\n";
    std::cout << p - p << "\n";
    h(p);


    // std::cout << f(p) << "\n";
    std::cout << "digits<double>         -> " << std::numeric_limits<double>::digits << "\n";
    std::cout << "digits10<double>       -> " << std::numeric_limits<double>::digits10 << "\n";
    std::cout << "radix<double>          -> " << std::numeric_limits<double>::radix << "\n";
    std::cout << "min_exponent<double>   -> " << std::numeric_limits<double>::min_exponent << "\n";
    std::cout << "max_exponent<double>   -> " << std::numeric_limits<double>::max_exponent << "\n";
    std::cout << "min_exponent10<double> -> " << std::numeric_limits<double>::min_exponent10 << "\n";
    std::cout << "max_exponent10<double> -> " << std::numeric_limits<double>::max_exponent10 << "\n";

    auto v = make_domain(2, 0_c, 5_c);
    static_assert(std::ratio_less_equal<decltype(v)::min_type, zero>::value, "bizarre");
    std::cout << f(v) << "\n";
}
