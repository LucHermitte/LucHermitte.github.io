/** Defines domain for double and floats.
 *
 * @file	domain.h
 * @author	Luc Hermitte <EMAIL:luc{dot}hermitte{at}gmail{dot}com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 *
 * A number belongs to a domain of definition.
 *
 * The addition of two doubles belonging to domains will produce a new
 * domain on-the-fly.
 *
 * This way, a function can specify to work on numbers from a specific
 * domain and reject (at compile-time) numbers incompatible to the
 * domain. e.g.
 * @code
 * domain<double, -pi/2, +pi/2> arcsin(domain<double,-1,+1>);
 * @endcode
 *
 * <b>Problems</b>:
 * - We cannot have double as template parameters as we could have
 *   integers: `domain<double, 0, 3.1415926>` won't compile. Thus we
 *   need a cumbersome hack to workaround around this issue:
 *   @code
 *   domain<double, decltype(0_c), decltype(3.1415926_c)>
 *   @endcode
 * - Handle infinites
 * - Error messages are not very expressive (yet?)
 *
 * @todo support
 * - multiplication, division
 * - detect that \f$x-x\f$ will produce the domain \f$[0,0]\f$ --
 * except that rounding errors may give something different from \f$0\f$
 * :(
 * - half-open and open domains
 * - detect that operations on const expressions in order to produce
 * ponctual ranges, i.e.:
 *   @code
 *     const auto a = make_domain(3.5);
 *     const auto b = make_domain(42);
 *     static_assert(is_same<decltype(a+b), domain<double, decltype(45.5_c), decltype(45.5_c)>>::value)
 *   @endcode
 * - double max and min as parameters, and thus operations on `DBL_MAX`
 * and `DBL_MIN`, e.g.
 *   @code
 *     using positive = domain<double, 0_min>;
 *     using negative = domain<double, 0_max>;
 *   @endcode
 */
#ifndef DOMAIN_H__
#define DOMAIN_H__

#include <algorithm>
#include <cassert>
#include <ratio>
#include <ostream>
#include <utility>
#include <limits>

static_assert(std::numeric_limits<double>::is_iec559, "Oups");

namespace literals
{
    /**
     * @author rnickb
     * Licence: CC-BY-SA
     * @see http://stackoverflow.com/a/31244563/15934
     * @see http://blog.stackoverflow.com/2009/06/attribution-required
     * @{
     */
    template <std::intmax_t Value, char...>
        struct ParseNumeratorImpl {
            static constexpr std::intmax_t value = Value;
        };

    template <std::intmax_t Value, char First, char... Rest>
        struct ParseNumeratorImpl<Value, First, Rest...> {
            static constexpr std::intmax_t value =
                (First == '.')
                ? ParseNumeratorImpl<Value, Rest...>::value
                : ParseNumeratorImpl<10 * Value + (First - '0'), Rest...>::value;
        };

    template <char... Chars>
        struct ParseNumerator {
            static constexpr std::intmax_t value = ParseNumeratorImpl<0, Chars...>::value;
        };

    template <std::intmax_t Value, bool, char...>
        struct ParseDenominatorImpl {
            static constexpr std::intmax_t value = Value;
        };

    template <std::intmax_t Value, bool RightOfDecimalPoint, char First, char... Rest>
        struct ParseDenominatorImpl<Value, RightOfDecimalPoint, First, Rest...> {
            static constexpr std::intmax_t value =
                (First == '.' && sizeof...(Rest) > 0)
                ? ParseDenominatorImpl<1, true, Rest...>::value
                : RightOfDecimalPoint
                ? ParseDenominatorImpl<Value * 10, true, Rest...>::value
                : ParseDenominatorImpl<1, false, Rest...>::value;
        };

    template <char... Chars>
        using ParseDenominator = ParseDenominatorImpl<1, false, Chars...>;
    /** @} */

    template <std::intmax_t Num, std::intmax_t Denom>
        std::ratio<-Num, Denom> operator-(std::ratio<Num, Denom>) {
            return {};
        }
    template <char... Chars>
        constexpr auto operator"" _c() {
            return std::ratio<ParseNumerator<Chars...>::value,
                   ParseDenominator<Chars...>::value>{};
        }
    template <std::intmax_t Num, std::intmax_t Den>
        constexpr double value(std::ratio<Num,Den>) {return static_cast<double>(Num)/Den; }
} // namespace literals

template <typename T>
constexpr bool belongs(T const& value_, T const& min_, T const& max_)
{ return min_ <= value_ && value_ <= max_; }

template <typename min_, typename max_, typename min2_, typename max2_>
struct contains
{
    static constexpr bool value
        =  std::ratio_less_equal<min_, min2_>::value
        && std::ratio_greater_equal<max_, max2_>::value;
};

template <typename T, typename min_, typename max_>
struct domain
{
    constexpr explicit domain(T value_) : m_value(value_)
    {
        assert(::belongs(value_, min(), max()));
    }
    using min_type = min_;
    using max_type = max_;


    template <typename U, typename min2_, typename max2_>
    static constexpr bool contains(domain<U,min2_,max2_> const& rhs_) {
        return (min() <= rhs_.min()) && (max() >= rhs_.max());
    }

    template <typename U, typename min2_, typename max2_>
        domain(domain<U,min2_,max2_> const& rhs_
                , typename std::enable_if<::contains<min_,max_,min2_,max2_>::value>::type * = nullptr)
        : m_value(rhs_.value()) {}

    constexpr T const& value() const { return m_value; }

#if 0
    template <typename U, typename min2_, typename max2_>
        domain(domain<U,min2_,max2_> value_
                , typename std::enable_if<! ::contains<min_,max_,min2_,max2_>::value>::type * = nullptr)
        {
            static_assert(false, "Cannot restrict U domain into current domain");
        }
#endif

    static constexpr T min() { return literals::value(min_{}); }
    static constexpr T max() { return literals::value(max_{}); }

    friend std::ostream & operator<<(std::ostream & os, const domain & v)
    {
        return os << v.m_value << " in [" << v.min() << ", " << v.max() << "]";
    }

    // cannot define += and co

    template <typename U, typename minL_, typename maxL_, typename minR_, typename maxR_>
        inline friend
        constexpr auto operator+(domain<U, minL_, maxL_> const& lhs, domain<U, minR_, maxR_> const& rhs)
        {
            return domain<U, std::ratio_add<minL_, minR_>, std::ratio_add<maxL_, maxR_>>(lhs.m_value + rhs.m_value);
        }
    template <typename U, typename minL_, typename maxL_, typename minR_, typename maxR_>
        inline friend
        constexpr auto operator-(domain<U, minL_, maxL_> const& lhs, domain<U, minR_, maxR_> const& rhs)
        {
            return domain<U, std::ratio_subtract<minL_, maxR_>, std::ratio_subtract<maxL_, minR_>>(lhs.m_value - rhs.m_value);
        }
#if 0
    template <typename U, typename Lmin, typename Rmin>
        inline friend
        constexpr auto operator*(domain<U, minL_, maxL_> const& lhs, domain<U, minR_, maxR_> const& rhs)
        {
            using std::min;
            using std::max;
            constexpr U newmin = min(lhs.min()*rhs.min(), lhs.min()*rhs.max(), lhs.max()*rhs.min(), lhs.max()*rhs.max());
            constexpr U newmax = min(lhs.min()*rhs.min(), lhs.min()*rhs.max(), lhs.max()*rhs.min(), lhs.max()*rhs.max());
            return domain<U, newmin, newmax>(lhs.m_value * rhs.m_value);
        }
#endif
private:
    T m_value;
};

template <typename T, typename min_, typename max_>
inline
constexpr auto make_domain(T v, min_ m, max_ M) {
    return domain<T,min_,max_>(v);
}

#endif // DOMAIN_H__
