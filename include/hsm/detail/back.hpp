/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef HSM_DETAIL_BACK_HPP_INCLUDED
#define HSM_DETAIL_BACK_HPP_INCLUDED
#include <hsm/hsm_fwd.hpp>
#include <kvasir/mpl/algorithm/stable_sort.hpp>
#include <kvasir/mpl/algorithm/find_if.hpp>
#include <kvasir/mpl/sequence/size.hpp>

namespace hsm
{
namespace back
{
namespace detail
{
template <typename T>
struct get_action_impl
{
    using type = kvasir::mpl::uint_<0>;
};

template <typename A, size_t Id>
struct get_action_impl<action_node<A, Id>>
{
    using type = kvasir::mpl::uint_<Id>;
};

template <typename T>
struct get_condition_impl
{
    using type = kvasir::mpl::uint_<0>;
};

template <typename C, size_t Id>
struct get_condition_impl<condition_node<C, Id>>
{
    using type = kvasir::mpl::uint_<Id>;
};

template <typename C, typename T>
struct get_state_impl
{
    using type = T;
};

template <typename C>
struct get_state_impl<C, hsm::current_state>
{
    using type = C;
};

template <typename C>
struct get_state_impl<C, hsm::internal_transition>
{
    using type = C;
};

template <typename C>
struct get_state_impl<C, hsm::initial_state>
{
    using type = C;
};

template <typename C, typename T>
struct get_state_impl<C, hsm::history_state<T>>
{
    using type = typename get_state_impl<C, T>::type;
};

template <typename C, typename T>
struct get_state_impl<C, hsm::deep_history_state<T>>
{
    using type = typename get_state_impl<C, T>::type;
};

template <typename C, typename T>
struct get_state_impl<C, hsm::final_state<T>>
{
    using type = typename get_state_impl<C, T>::type;
};

template <typename A, size_t I, typename Actions>
constexpr void set_action(action_node<A, I>& node, Actions& actions)
{
    actions[I] = std::move(node.action);
}
template <typename C, size_t I, typename Conditions>
constexpr void set_condition(condition_node<C, I>& node, Conditions& conds)
{
    conds[I] = std::move(node.condition);
}
template <typename Conditions>
constexpr void set_condition(no_cond, Conditions&)
{
}
template <typename Actions>
constexpr void set_action(no_action, Actions&)
{
}

template <typename T, typename Conditions, typename Actions>
constexpr void initialize_ca_array(hsm::event<T>, Conditions&, Actions&)
{
}

template <typename T, typename Conditions, typename Actions>
constexpr void initialize_ca_array(hsm::state_ref<T>, Conditions&, Actions&)
{
}

template <typename A, size_t I, typename Conditions, typename Actions>
constexpr void initialize_ca_array(exit_action<A, I>& exit, Conditions&, Actions& actions)
{
    actions[I] = std::move(exit.action);
}
template <typename A, size_t I, typename Conditions, typename Actions>
constexpr void initialize_ca_array(entry_action<A, I>& entry, Conditions&, Actions& actions)
{
    actions[I] = std::move(entry.action);
}
template <typename TT, typename S, typename E, typename C, typename A, typename D, typename Conditions, typename Actions>
constexpr void initialize_ca_array(hsm::transition<TT, S, E, C, A, D>& t, Conditions& conds, Actions& actions)
{
    set_action(t.action, actions);
    set_condition(t.cond, conds);
}
template <typename K, typename... Cs, typename Conditions, typename Actions>
constexpr void initialize_ca_array(hsm::state<K, Cs...>& sm, Conditions& conds, Actions& actions)
{
    tiny_tuple::foreach (sm.data, [&conds, &actions](auto& element) { initialize_ca_array(element, conds, actions); });
}

template <int Id>
struct is_state
{
    template <typename T>
    struct f_impl : kvasir::mpl::bool_<false>
    {
    };
    template <typename N, uint32_t F, size_t S, size_t P, size_t Entry, size_t Exit, typename... Ts>
    struct f_impl<tiny_tuple::detail::item<N, hsm::back::state<F, Id, S, P, Entry, Exit, Ts...>>> : kvasir::mpl::bool_<true>
    {
    };
    template <typename T>
    using f = f_impl<T>;
};

struct is_any_state
{
    template <typename T>
    struct f_impl : kvasir::mpl::bool_<false>
    {
    };
    template <typename N, uint32_t F, size_t Id, size_t S, size_t P, size_t Entry, size_t Exit, typename... Ts>
    struct f_impl<tiny_tuple::detail::item<N, hsm::back::state<F, Id, S, P, Entry, Exit, Ts...>>> : kvasir::mpl::bool_<true>
    {
    };
    template <typename T>
    using f = f_impl<T>;
};

struct sort_transition
{
    constexpr static transition_flags tm       = transition_flags::transition_type_mask;
    constexpr static transition_flags internal = transition_flags::internal;
    constexpr static transition_flags normal   = transition_flags::normal;
    template <typename T1, typename T2>
    using f =
        kvasir::mpl::bool_<(T1::flags & tm) < (T2::flags & tm) ||
                           (((T1::flags & tm) == (T2::flags & tm) && ((((T2::flags & tm) == normal) && T1::event > T2::event) ||
                                                                      ((((T2::flags & tm) == internal) && T1::event > T2::event)))))>;
};

struct by_state_id
{
    template <typename T1, typename T2>
    using f = kvasir::mpl::bool_<(T1::value::id < T2::value::id)>;
};

struct extract_transition_count
{
    template <typename T>
    using f = kvasir::mpl::uint_<T::transition_count>;
};



struct normal_transition
{
    template <typename T1>
    using f = kvasir::mpl::bool_<(T1::flags & transition_flags::transition_type_mask) >= transition_flags::internal>;
};

template <typename C = kvasir::mpl::listify>
struct unpack_transitions
{
    template <typename... T>
    using f = typename kvasir::mpl::detail::unpack_impl<C, typename T::transitions...>::type;
};

}  // namespace detail
}  // namespace back
}  // namespace hsm

#endif
