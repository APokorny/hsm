/* ==========================================================================
 Copyright (c) 2019-2023 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */


#ifndef HSM_FRONT_HPP_INCLUDED
#define HSM_FRONT_HPP_INCLUDED

#include <memory>
#include "tiny_tuple/tuple.h"
#include "hsm/hsm_fwd.hpp"
#include "hsm/detail/front.hpp"

namespace hsm
{
struct root_state
{
};
struct normal
{
};
struct completion
{
};
struct start
{
};
struct empty_history
{
};
struct internal_transition
{
};
struct current_state
{
};
struct no_cond
{
    template <typename... Ts>
    constexpr bool operator()(Ts&&...) const noexcept
    {
        return true;
    }
};

struct no_action
{
    template <typename... Ts>
    constexpr void operator()(Ts&&...) const noexcept
    {
    }
};

struct no_dest
{
};
template <typename A>
struct entry_action
{
    using action = A;
};
struct enter_node
{
    template <typename A>
    constexpr auto operator/(A&&) const noexcept
    {
        return entry_action<std::decay_t<A>>{};
    }

    template <typename A>
    constexpr auto operator=(A&&) const noexcept
    {
        return entry_action<std::decay_t<A>>{};
    }
};

constexpr enter_node enter;

template <typename A>
struct exit_action
{
    using action = A;
};
struct exit_node
{
    template <typename A>
    constexpr auto operator/(A&&) const noexcept
    {
        return exit_action<std::decay_t<A>>{};
    }

    template <typename A>
    constexpr auto operator=(A&&) const noexcept
    {
        return exit_action<std::decay_t<A>>{};
    }
};

constexpr exit_node exit;

template <typename TT, typename S, typename E, typename C, typename A, typename D>
struct transition
{
    using transition_type = TT;
    using source_type     = S;
    using dest_type       = D;
    using cond            = C;
    using action          = A;
    template <typename Condition>
        requires std::is_same_v<C, no_cond>
    constexpr auto operator[](Condition&&) const noexcept
    {
        return transition<TT, S, E, std::decay_t<Condition>, A, D>{};
    }
    template <typename Action>
        requires std::is_same_v<A, no_action>
    constexpr auto operator/(Action&&) const noexcept
    {
        return transition<TT, S, E, C, std::decay_t<Action>, D>{};
    }
    template <typename Dest>
        requires std::is_same_v<D, no_dest>
    constexpr auto operator=(Dest&&) const noexcept
    {
        return transition<TT, S, E, C, A, std::decay_t<Dest>>{};
    }
    constexpr auto operator=(internal_transition const&) const noexcept
        requires std::is_same_v<D, no_dest>
    {
        return transition<internal_transition, S, E, C, A, S>{};
    }
};

template <typename Ref, typename... Cs>
struct state
{
    tiny_tuple::tuple<Cs...> data;  // temp hack
    constexpr state(Cs&&... cs) : data{std::forward<Cs>(cs)...} {}
    constexpr state(tiny_tuple::tuple<Cs...>&& elements) : data(std::move(elements)) {}
};

template <typename State, typename Event, typename Condition, typename Action, typename Destination>
using n_trans = transition<normal, State, Event, Condition, Action, Destination>;

template <typename State, typename Event, typename Condition, typename Action, typename Destination>
using i_trans = transition<internal_transition, State, Event, Condition, Action, Destination>;

template <typename State, typename Condition, typename Action, typename Destination>
using d_trans = transition<completion, State, no_event, Condition, Action, Destination>;

template <typename Condition, typename Action, typename Destination>
using s_trans = transition<start, current_state, no_event, Condition, Action, Destination>;

template <typename State, typename Destination>
using h_trans = transition<empty_history, State, no_event, no_cond, no_action, Destination>;

HSM_TEMPLATE_LITERAL(Key)
struct state_ref
{
    // workaround for type deduction
    using this_type = state_ref;
    template <typename... Ts>
    constexpr auto operator()(Ts&&... ts) const noexcept
    {
        return state<this_type, Ts...>{std::move(ts)...};
    }
    HSM_TEMPLATE_LITERAL(T)
    constexpr auto operator+(event<T> const&) const noexcept { return n_trans<this_type, event<T>, no_cond, no_action, no_dest>{}; }

    template <typename TT, typename E, typename C, typename A, typename D>
    constexpr auto operator+(transition<TT, current_state, E, C, A, D>&&) const noexcept
    {
        return transition<TT, this_type, E, C, A, D>{};
    }
    template <typename Dest>
    constexpr auto operator=(Dest const&) const noexcept
    {
        return d_trans<this_type, no_cond, no_action, Dest>{};
    }
    template <typename C>
    constexpr auto operator[](C&&) const noexcept
    {
        return d_trans<this_type, std::decay_t<C>, no_action, no_dest>{};
    }
    template <typename A>
    constexpr auto operator/(A&&) const noexcept
    {
        return d_trans<this_type, no_cond, std::decay_t<A>, no_dest>{};
    }
};

struct initial_state
{
    template <typename Dest>
    constexpr auto operator=(Dest const&) const noexcept
    {
        return s_trans<no_cond, no_action, Dest>{};
    }
    template <typename C>
    constexpr auto operator[](C&&) const noexcept
    {
        return s_trans<std::decay_t<C>, no_action, no_dest>{};
    }
    template <typename A>
    constexpr auto operator/(A&&) const noexcept
    {
        return s_trans<no_cond, std::decay_t<A>, no_dest>{};
    }
};
constexpr initial_state initial;

template <typename State>
struct final_state
{
};
constexpr final_state<current_state> X;

template <typename State>
struct history_state
{
    template <typename Dest>
    constexpr auto operator=(Dest const&) const noexcept
    {
        return h_trans<history_state<State>, Dest>{};
    }
};
constexpr history_state<current_state> history;

template <typename State>
struct deep_history_state
{
    template <typename Dest>
    constexpr auto operator=(Dest const&) const noexcept
    {
        return h_trans<deep_history_state<State>, Dest>{};
    }
};
constexpr deep_history_state<current_state> deep_history;

constexpr internal_transition internal;

HSM_TEMPLATE_LITERAL(Key)
struct event
{
    using this_type = event;
#ifdef HSM_USE_PROPER_LITERALS
    // event(str_lit Key) {};
#endif
    template <typename D>
    constexpr auto operator=(D const&) const noexcept
    {
        return n_trans<current_state, this_type, no_cond, no_action, D>{};
    }
    constexpr auto operator=(internal_transition const&) const noexcept
    {
        return i_trans<current_state, this_type, no_cond, no_action, current_state>{};
    }

    template <typename C>
    constexpr auto operator[](C&&) const noexcept
    {
        return n_trans<current_state, this_type, std::decay_t<C>, no_action, no_dest>{};
    }
    template <typename A>
    constexpr auto operator/(A&&) const noexcept
    {
        return n_trans<current_state, this_type, no_cond, std::decay_t<A>, no_dest>{};
    }
};

struct no_event : event<no_event>
{
    using event<no_event>::operator=;
    using event<no_event>::operator[];
    using event<no_event>::operator/;
};

constexpr no_event any;

template <char... String>
struct slit
{
};
template <char... String>
struct elit
{
};

#ifdef HSM_USE_PROPER_LITERALS
namespace literals
{
template <str_lit str>
constexpr auto operator""_state() noexcept
{
    return hsm::state_ref{str};
}

template <str_lit str>
constexpr auto operator""_ev() noexcept
{
    return hsm::event<elit<>>{};
}
}  // namespace literals

using literals::operator""_ev;
using literals::operator""_state;

#else

#if defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-string-literal-operator-template"
#endif

namespace literals
{
template <typename CharT, CharT... String>
constexpr state_ref<slit<String...>> operator""_state() noexcept
{
    return {};
}

template <typename CharT, CharT... String>
constexpr event<elit<String...>> operator""_ev() noexcept
{
    return {};
}
}  // namespace literals

using literals::operator""_ev;
using literals::operator""_state;

#if defined(__clang__)
#pragma GCC diagnostic pop
#endif

#endif

HSM_TEMPLATE_LITERAL(K)
constexpr auto history_of(state_ref<K> const&) noexcept { return history_state<state_ref<K>>{}; }

HSM_TEMPLATE_LITERAL(K)
constexpr auto deep_history_of(state_ref<K> const&) noexcept { return deep_history_state<state_ref<K>>{}; }

HSM_TEMPLATE_LITERAL(K)
constexpr auto final_of(state_ref<K> const&) noexcept { return final_state<state_ref<K>>{}; }

constexpr state_ref<root_state> root;

}  // namespace hsm

#endif
