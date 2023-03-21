/* ==========================================================================
 Copyright (c) 2020 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#include <algorithm>

namespace hsm
{
// type to hold the strings of user defined literals - this is only needed
// with class type template parameter support.
template <std::size_t N>
struct str_lit
{
    constexpr str_lit(char const (&foo)[N + 1]) { std::copy_n(foo, N + 1, data); }
    constexpr auto operator<=>(str_lit const &) const = default;
    char           data[N + 1];
};
template <std::size_t N>
str_lit(char const (&str)[N]) -> str_lit<N - 1>;
}  // namespace hsm
