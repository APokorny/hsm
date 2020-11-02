/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
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

template <typename Condition>
struct condition_node
{
    Condition condition;
    condition_node(Condition&& c) : condition(std::forward<Condition>(c)) {}
    template <typename... Ts>
    constexpr bool operator()(Ts&&... ts) const noexcept
    {
        return condition(std::forward<Ts>(ts)...);
    }
};

template <typename Action>
struct action_node
{
    Action action;
    action_node(Action&& a) : action(std::forward<Action>(a)) {}

    template <typename... Ts>
    constexpr void operator()(Ts&&... ts) const noexcept
    {
        action(std::forward<Ts>(ts)...);
    }
};

struct no_dest
{
};
template <typename A>
struct entry_action
{
    A action;
};
struct enter_node
{
    template <typename A>
    constexpr auto operator/(A&& a) const noexcept
    {
        return entry_action<std::decay_t<A>>{std::forward<std::decay_t<A>>(a)};
    }

    template <typename A>
    constexpr auto operator=(A&& a) const noexcept
    {
        return entry_action<std::decay_t<A>>{std::forward<std::decay_t<A>>(a)};
    }
};

constexpr enter_node enter;

template <typename A>
struct exit_action
{
    A action;
};
struct exit_node
{
    template <typename A>
    constexpr auto operator/(A&& a) const noexcept
    {
        return exit_action<std::decay_t<A>>{std::forward<std::decay_t<A>>(a)};
    }

    template <typename A>
    constexpr auto operator=(A&& a) const noexcept
    {
        return exit_action<std::decay_t<A>>{std::forward<std::decay_t<A>>(a)};
    }
};

constexpr exit_node exit;

template <typename TT, typename S, typename E, typename C, typename A, typename D>
struct transition
{
    using transition_type = TT;
    using source_type     = S;
    mutable C cond;
    mutable A action;
    transition(C&& c, A&& a) : cond{std::forward<C>(c)}, action{std::forward<A>(a)} {}
};

template <typename TT, typename S, typename E, typename C, typename D>
struct transition<TT, S, E, C, no_action, D>
{
    using transition_type = TT;
    using source_type     = S;
    mutable C         cond;
    mutable no_action action;
    transition(C&& c) : cond{std::forward<C>(c)} {}
    transition(C&& c, no_action&&) : cond{std::forward<C>(c)} {}
};

template <typename TT, typename S, typename E, typename D>
struct transition<TT, S, E, no_cond, no_action, D>
{
    using transition_type = TT;
    using source_type     = S;
    mutable no_cond   cond;
    mutable no_action action;
};

template <typename TT, typename S, typename E>
struct transition<TT, S, E, no_cond, no_action, no_dest>
{
    using transition_type = TT;
    using source_type     = S;
    mutable no_cond   cond;
    mutable no_action action;
    template <typename C>
    constexpr auto operator[](C&& c) const noexcept
    {
        return transition<TT, S, E, condition_node<std::decay_t<C>>, no_action, no_dest>{
            condition_node<std::decay_t<C>>{std::forward<std::decay_t<C>>(c)}};
    }
    template <typename A>
    constexpr auto operator/(A&& a) const noexcept
    {
        return transition<TT, S, E, no_cond, action_node<std::decay_t<A>>, no_dest>{
            no_cond{}, action_node<std::decay_t<A>>{std::forward<std::decay_t<A>>(a)}};
    }
    template <typename D>
    constexpr auto operator=(D const&) const noexcept
    {
        return transition<TT, S, E, no_cond, no_action, D>{};
    }
    constexpr auto operator=(internal_transition const&) const noexcept
    {
        return transition<internal_transition, S, E, no_cond, no_action, S>{};
    }
};

template <typename TT, typename S, typename E, typename C>
struct transition<TT, S, E, C, no_action, no_dest>
{
    using transition_type = TT;
    using source_type     = S;
    C mutable cond;
    no_action mutable action;
    transition(C&& c) : cond{std::forward<C>(c)} {}
    transition(C&& c, no_action&&) : cond{std::forward<C>(c)} {}
    template <typename A>
    constexpr auto operator/(A&& a) const noexcept
    {
        return transition<TT, S, E, C, action_node<std::decay_t<A>>, no_dest>{
            std::move(cond), action_node<std::decay_t<A>>{std::forward<std::decay_t<A>>(a)}};
    }
    template <typename D>
    constexpr auto operator=(D const&) const noexcept
    {
        return transition<TT, S, E, C, no_action, D>{std::move(cond)};
    }
    constexpr auto operator=(internal_transition const&) const noexcept
    {
        return transition<internal_transition, S, E, C, no_action, S>{std::move(cond)};
    }
};

template <typename TT, typename S, typename E, typename C, typename A>
struct transition<TT, S, E, C, A, no_dest>
{
    using transition_type = TT;
    using source_type     = S;
    C mutable cond;
    A mutable action;
    transition(C&& c, A&& a) : cond{std::forward<C>(c)}, action{std::forward<A>(a)} {}
    template <typename D>
    constexpr auto operator=(D const&) const noexcept
    {
        return transition<TT, S, E, C, A, D>{std::move(cond), std::move(action)};
    }
    constexpr auto operator=(internal_transition const&) const noexcept
    {
        return transition<internal_transition, S, E, C, A, S>{std::move(cond), std::move(action)};
    }
};

template <typename TT, typename S, typename E, typename A>
struct transition<TT, S, E, no_cond, A, no_dest>
{
    using transition_type = TT;
    using source_type     = S;
    mutable no_cond cond;
    mutable A       action;
    transition(no_cond&&, A&& a) : action{std::forward<A>(a)} {}
    template <typename D>
    constexpr auto operator=(D const&) const noexcept
    {
        return transition<TT, S, E, no_cond, A, D>{no_cond{}, std::move(action)};
    }
    constexpr auto operator=(internal_transition const&) const noexcept
    {
        return transition<internal_transition, S, E, no_cond, A, S>{no_cond{}, std::move(action)};
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

template <typename State, typename Condition, typename Action, typename Destination>
using h_trans = transition<empty_history, State, no_event, Condition, Action, Destination>;

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
    constexpr auto operator+(transition<TT, current_state, E, C, A, D>&& t) const noexcept
    {
        return transition<TT, this_type, E, C, A, D>{std::move(t.cond), std::move(t.action)};
    }
    template <typename Dest>
    constexpr auto operator=(Dest const&) const noexcept
    {
        return d_trans<this_type, no_cond, no_action, Dest>{};
    }
    template <typename C>
    constexpr auto operator[](C&& c) const noexcept
    {
        return d_trans<this_type, condition_node<std::decay_t<C>>, no_action, no_dest>{
            condition_node<std::decay_t<C>>{std::forward<std::decay_t<C>>(c)}};
    }
    template <typename A>
    constexpr auto operator/(A&& a) const noexcept
    {
        return d_trans<this_type, no_cond, action_node<std::decay_t<A>>, no_dest>{no_cond{},
                                                                                  action_node<A>{std::forward<std::decay_t<A>>(a)}};
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
    constexpr auto operator[](C&& c) const noexcept
    {
        return s_trans<condition_node<std::decay_t<C>>, no_action, no_dest>{
            condition_node<std::decay_t<C>>{std::forward<std::decay_t<C>>(c)}};
    }
    template <typename A>
    constexpr auto operator/(A&& a) const noexcept
    {
        return s_trans<no_cond, action_node<std::decay_t<A>>, no_dest>{no_cond{},
                                                                       action_node<std::decay_t<A>>{std::forward<std::decay_t<A>>(a)}};
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
        return h_trans<State, no_cond, no_action, Dest>{no_cond{}};
    }
    template <typename C>
    constexpr auto operator[](C&& c) const noexcept
    {
        return h_trans<State, condition_node<std::decay_t<C>>, no_action, no_dest>{
            condition_node<std::decay_t<C>>{std::forward<std::decay_t<C>>(c)}};
    }
    template <typename A>
    constexpr auto operator/(A&& a) const noexcept
    {
        return h_trans<State, no_cond, action_node<std::decay_t<A>>, no_dest>{
            no_cond{}, action_node<std::decay_t<A>>{std::forward<std::decay_t<A>>(a)}};
    }
};
constexpr history_state<current_state> history;

template <typename State>
struct deep_history_state
{
    template <typename Dest>
    constexpr auto operator=(Dest const&) const noexcept
    {
        return h_trans<State, no_cond, no_action, Dest>{no_cond{}};
    }
    template <typename C>
    constexpr auto operator[](C&& c) const noexcept
    {
        return h_trans<State, condition_node<std::decay_t<C>>, no_action, no_dest>{
            condition_node<std::decay_t<C>>{std::forward<std::decay_t<C>>(c)}};
    }
    template <typename A>
    constexpr auto operator/(A&& a) const noexcept
    {
        return h_trans<State, no_cond, action_node<std::decay_t<A>>, no_dest>{
            no_cond{}, action_node<std::decay_t<A>>{std::forward<std::decay_t<A>>(a)}};
    }
};
constexpr deep_history_state<current_state> deep_history;

constexpr internal_transition internal;

HSM_TEMPLATE_LITERAL(Key)
struct event
{
    using this_type = event;
#ifdef HSM_USE_PROPER_LITERALS
    //event(str_lit Key) {};
#endif
    template <typename D>
    constexpr auto operator=(D const&) const noexcept
    {
        return n_trans<current_state, this_type, no_cond, no_action, D>{no_cond{}, no_action{}};
    }
    constexpr auto operator=(internal_transition const&) const noexcept
    {
        return i_trans<current_state, this_type, no_cond, no_action, current_state>{no_cond{}, no_action{}};
    }

    template <typename C>
    constexpr auto operator[](C&& c) const noexcept
    {
        return n_trans<current_state, this_type, condition_node<std::decay_t<C>>, no_action, no_dest>{
            condition_node<std::decay_t<C>>{std::forward<std::decay_t<C>>(c)}, no_action{}};
    }
    template <typename A>
    constexpr auto operator/(A&& a) const noexcept
    {
        return n_trans<current_state, this_type, no_cond, action_node<std::decay_t<A>>, no_dest>{
            no_cond{}, action_node<std::decay_t<A>>{std::forward<std::decay_t<A>>(a)}};
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
auto history_of(state_ref<K> const&) noexcept { return history_state<state_ref<K>>{}; }

HSM_TEMPLATE_LITERAL(K)
auto deep_history_of(state_ref<K> const&) noexcept { return deep_history_state<state_ref<K>>{}; }

HSM_TEMPLATE_LITERAL(K)
auto final_of(state_ref<K> const&) noexcept { return final_state<state_ref<K>>{}; }

constexpr state_ref<root_state> root;

}  // namespace hsm

#endif
