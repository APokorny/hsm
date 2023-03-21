/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#pragma once

#include <type_traits>
#include <kvasir/mpl/types/int.hpp>

namespace hsm
{
namespace back
{
template <typename E>
struct enable_bitmask_ops : kvasir::mpl::bool_<false>
{
};

template <typename E, typename F>
constexpr std::enable_if_t<
    enable_bitmask_ops<E>::value && !std::is_same<E, F>::value && std::is_convertible<F, std::underlying_type_t<E>>::value, E>
operator|(E lhs, F rhs)
{
    using underlying = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
}

template <typename F, typename E>
constexpr std::enable_if_t<
    enable_bitmask_ops<E>::value && !std::is_same<E, F>::value && std::is_convertible<F, std::underlying_type_t<E>>::value, E>
operator|(F lhs, E rhs)
{
    using underlying = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
}

template <typename E>
constexpr std::enable_if_t<enable_bitmask_ops<E>::value, E> operator|(E lhs, E rhs)
{
    using underlying = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
}

template <typename E, typename F>
constexpr std::enable_if_t<
    enable_bitmask_ops<E>::value && !std::is_same<E, F>::value && std::is_convertible<F, std::underlying_type_t<E>>::value, E>
operator&(E lhs, F rhs)
{
    using underlying = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
}

template <typename F, typename E>
constexpr std::enable_if_t<
    enable_bitmask_ops<E>::value && !std::is_same<E, F>::value && std::is_convertible<F, std::underlying_type_t<E>>::value, E>
operator&(F lhs, E rhs)
{
    using underlying = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
}

template <typename E>
constexpr std::enable_if_t<enable_bitmask_ops<E>::value, E> operator&(E lhs, E rhs)
{
    using underlying = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
}

template <typename E>
constexpr std::enable_if_t<enable_bitmask_ops<E>::value, E> operator^(E lhs, E rhs)
{
    using underlying = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<underlying>(lhs) ^ static_cast<underlying>(rhs));
}

template <typename E>
constexpr std::enable_if_t<enable_bitmask_ops<E>::value, E> operator~(E lhs)
{
    using underlying = std::underlying_type_t<E>;
    return static_cast<E>(~static_cast<underlying>(lhs));
}

template <typename E>
constexpr std::enable_if_t<enable_bitmask_ops<E>::value, E&> operator|=(E& lhs, E rhs)
{
    using underlying = std::underlying_type_t<E>;
    lhs              = static_cast<E>(static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
    return lhs;
}

template <typename E>
constexpr std::enable_if_t<enable_bitmask_ops<E>::value, E&> operator&=(E& lhs, E rhs)
{
    using underlying = std::underlying_type_t<E>;
    lhs              = static_cast<E>(static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
    return lhs;
}

template <typename E>
constexpr std::enable_if_t<enable_bitmask_ops<E>::value, E&> operator^=(E& lhs, E rhs)
{
    using underlying = std::underlying_type_t<E>;
    lhs              = static_cast<E>(static_cast<underlying>(lhs) ^ static_cast<underlying>(rhs));
    return lhs;
}

template <typename E>
constexpr std::enable_if_t<enable_bitmask_ops<E>::value, typename std::underlying_type_t<E>> cast(E v)
{
    return static_cast<typename std::underlying_type_t<E>>(v);
}
template <typename E>
constexpr std::enable_if_t<enable_bitmask_ops<E>::value, bool> is_set(E v, E f)
{
    return (cast(v) & cast(f)) == cast(f);
}

}  // namespace back
}  // namespace hsm
