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
struct no_event
{
};

struct current_state
{
};
template <typename T>
struct event;
template <typename Key>
struct state_ref;
struct no_cond
{
    template <typename... Ts>
    constexpr bool operator()(Ts&&... ts) const noexcept
    {
        return true;
    }
};

struct no_action
{
    template <typename... Ts>
    constexpr void operator()(Ts&&... ts) const noexcept
    {
    }
};

template <typename Condition, size_t Id>
struct condition_node
{
    constexpr static size_t condition_id = Id;
    Condition               condition;
    condition_node(Condition&& c) : condition(std::forward<Condition>(c)) {}
    template <typename... Ts>
    constexpr bool operator()(Ts&&... ts) const noexcept
    {
        return condition(std::forward<Ts>(ts)...);
    }
};

template <typename Action, size_t Id>
struct action_node
{
    constexpr static size_t action_id = Id;
    Action                  action;
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
template <typename A, size_t ActionId>
struct entry_action
{
    static constexpr size_t action_id = ActionId;
    A                       action;
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

template <typename A, size_t ActionId>
struct exit_action
{
    static constexpr size_t action_id = ActionId;
    A                       action;
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
    constexpr auto operator[](C&& cond) const noexcept
    {
        return transition<TT, S, E, condition_node<std::decay_t<C>>, no_action, no_dest>{
            condition_node<std::decay_t<C>>{std::forward<std::decay_t<C>>(cond)}};
    }
    template <typename A>
    constexpr auto operator/(A&& action) const noexcept
    {
        return transition<TT, S, E, no_cond, action_node<std::decay_t<A>>, no_dest>{
            no_cond{}, action_node<std::decay_t<A>>{std::forward<std::decay_t<A>>(action)}};
    }
    template <typename D>
    constexpr auto operator=(D const&) const noexcept
    {
        return transition<TT, S, E, no_cond, no_action, D>{};
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
    constexpr auto operator/(A&& action) const noexcept
    {
        return transition<TT, S, E, C, action_node<std::decay_t<A>>, no_dest>{
            std::move(cond), action_node<std::decay_t<A>>{std::forward<std::decay_t<A>>(action)}};
    }
    template <typename D>
    constexpr auto operator=(D const&) const noexcept
    {
        return transition<TT, S, E, C, no_action, D>{std::move(cond)};
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
};

template <typename K, typename... Cs>
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

template <typename Key>
struct state_ref
{
    template <typename... Ts>
    constexpr auto operator()(Ts&&... ts) const noexcept
    {
        return state<Key, Ts...>{std::move(ts)...};
    }
    template <typename T>
    constexpr auto operator+(event<T> const&) const noexcept
    {
        return n_trans<state_ref<Key>, event<T>, no_cond, no_action, no_dest>{};
    }

    template <typename TT, typename E, typename C, typename A, typename D>
    constexpr auto operator+(transition<TT, current_state, E, C, A, D>&& t) const noexcept
    {
        return transition<TT, state_ref<Key>, E, C, A, D>{std::move(t.cond), std::move(t.action)};
    }
    template <typename Dest>
    constexpr auto operator=(Dest const&) const noexcept
    {
        return d_trans<state_ref<Key>, no_cond, no_action, Dest>{};
    }
    template <typename C>
    constexpr auto operator[](C&& c) const noexcept
    {
        return d_trans<state_ref<Key>, condition_node<std::decay_t<C>>, no_action, no_dest>{
            condition_node<std::decay_t<C>>{std::forward<std::decay_t<C>>(c)}};
    }
    template <typename A>
    constexpr auto operator/(A&& a) const noexcept
    {
        return d_trans<state_ref<Key>, no_cond, action_node<std::decay_t<A>>, no_dest>{no_cond{},
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

template <typename Key>
struct event
{
    template <typename D>
    constexpr auto operator=(D const&) const noexcept
    {
        return n_trans<current_state, event<Key>, no_cond, no_action, D>{no_cond{}, no_action{}};
    }
    template <typename D>
    constexpr auto operator=(internal_transition const&) const noexcept
    {
        return i_trans<current_state, event<Key>, no_cond, no_action, current_state>{no_cond{}, no_action{}};
    }

    template <typename C>
    constexpr auto operator[](C&& c) const noexcept
    {
        return n_trans<current_state, event<Key>, condition_node<std::decay_t<C>>, no_action, no_dest>{
            condition_node<std::decay_t<C>>{std::forward<std::decay_t<C>>(c)}, no_action{}};
    }
    template <typename A>
    constexpr auto operator/(A&& a) const noexcept
    {
        return n_trans<current_state, event<Key>, no_cond, action_node<std::decay_t<A>>, no_dest>{
            no_cond{}, action_node<std::decay_t<A>>{std::forward<std::decay_t<A>>(a)}};
    }
};

template <char... String>
struct slit
{
};
template <char... String>
struct elit
{
};

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

template <typename K>
auto history_of(state_ref<K> const&) noexcept
{
    return history_state<state_ref<K>>{};
}

template <typename K>
auto deep_history_of(state_ref<K> const&) noexcept
{
    return deep_history_state<state_ref<K>>{};
}

template <typename K>
auto final_of(state_ref<K> const&) noexcept
{
    return final_state<state_ref<K>>{};
}

}  // namespace hsm

#endif
