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
constexpr void set_action(action_node<A, I>&& node, Actions& actions)
{
    actions[I] = std::move(node.action);
}
template <typename C, size_t I, typename Conditions>
constexpr void set_condition(condition_node<C, I>&& node, Conditions& conds)
{
    conds[I] = std::move(node.condition);
}
template <typename Conditions>
constexpr void set_condition(no_cond&&, Conditions&)
{
}
template <typename Actions>
constexpr void set_action(no_action&&, Actions&)
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
constexpr void initialize_ca_array(exit_action<A, I>&& exit, Conditions&, Actions& actions)
{
    actions[I] = std::move(exit.action);
}
template <typename A, size_t I, typename Conditions, typename Actions>
constexpr void initialize_ca_array(entry_action<A, I>&& entry, Conditions&, Actions& actions)
{
    actions[I] = std::move(entry.action);
}
template <typename TT, typename S, typename E, typename C, typename A, typename D, typename Conditions, typename Actions>
constexpr void initialize_ca_array(hsm::transition<TT, S, E, C, A, D>&& t, Conditions& conds, Actions& actions)
{
    set_action(std::move(t.action), actions);
    set_condition(std::move(t.cond), conds);
}
template <typename K, typename... Cs, typename Conditions, typename Actions>
constexpr void initialize_ca_array(hsm::state<K, Cs...>&& sm, Conditions& conds, Actions& actions)
{
    tiny_tuple::foreach (std::move(sm.data),
                         [&conds, &actions](auto&& element) { initialize_ca_array(std::move(element), conds, actions); });
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

template <int TO, uint32_t TF, size_t E, size_t D, size_t C, size_t A, typename Transitions>
constexpr int apply_transition(hsm::back::transition<TF, E, D, C, A>, Transitions& trans)
{
    using tte = typename Transitions::value_type;
    trans[TO] = tte{typename tte::event_id(E),                   //
                    typename tte::state_id(D),                   //
                    static_cast<typename tte::condition_id>(C),  //
                    static_cast<typename tte::action_id>(A),     //
                    static_cast<back::transition_flags>(TF)};
    return 0;
}

template <int TO, typename... Ts, int... Is, typename Transitions>
constexpr void apply_transitions(kvasir::mpl::list<Ts...>, std::integer_sequence<int, Is...>, Transitions& trans)
{
    int f[] = {apply_transition<TO + Is>(Ts{}, trans)...};
    (void)f;
}
struct sort_transition
{
    template <typename T1, typename T2>
    using f = kvasir::mpl::bool_<(T1::flags & cast(transition_flags::transition_type_mask)) <
                                 (T2::flags & cast(transition_flags::transition_type_mask))>;
};

struct normal_transition
{
    template <typename T1>
    using f = kvasir::mpl::bool_<(T1::flags & cast(transition_flags::transition_type_mask)) >= cast(transition_flags::internal)>;
};
template <int I, int TO, uint32_t Flags, size_t Id, size_t StateCount, size_t Parent, size_t Entry, size_t Exit, typename... Ts,
          typename Transitions, typename States>
constexpr auto apply_state(hsm::back::state<Flags, Id, StateCount, Parent, Entry, Exit, Ts...> sm, Transitions& transitions, States& states)
{
    using state_table_entry      = typename States::value_type;
    using sorted_transitions     = kvasir::mpl::call<kvasir::mpl::stable_sort<sort_transition>, Ts...>;
    using just_normal_transition = kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::find_if<normal_transition>>, sorted_transitions>;
    using num_normal_transition =
        kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::find_if<normal_transition, kvasir::mpl::size<>>>, sorted_transitions>;

    apply_transitions<TO>(sorted_transitions{}, std::make_integer_sequence<int, sizeof...(Ts)>(), transitions);
    states[I] = state_table_entry{static_cast<typename state_table_entry::transition_table_offset_type>(TO),
                                  static_cast<state_table_entry::action_id>(Entry),
                                  static_cast<state_table_entry::action_id>(Exit),
                                  static_cast<typename state_table_entry::state_id>(Parent),
                                  static_cast<typename state_table_entry::state_id>(StateCount),
                                  static_cast<uint16_t>(sizeof...(Ts)),
                                  static_cast<uint8_t>(sizeof...(Ts) - num_normal_transition::value),  // number of transitions
                                  static_cast<back::state_flags>(Flags)};
    return kvasir::mpl::uint_<TO + sizeof...(Ts)>();
}

template <int I, int TO, typename SM, typename Transitions, typename State>
constexpr void find_and_apply_state(SM&& sm, Transitions& transitions, std::array<State, I>& states)
{
}

template <int I, int TO, typename SM, typename Transitions, typename State, size_t SC>
constexpr void find_and_apply_state(SM&& sm, Transitions& transitions, std::array<State, SC>& states)
{
    using state_found = typename kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::find_if<is_state<I>, kvasir::mpl::front<>>>, SM>::value;
    auto transition_offset = apply_state<I, TO>(state_found{}, transitions, states);
    find_and_apply_state<I + 1, decltype(transition_offset)::value>(std::forward<SM>(sm), transitions, states);
}

}  // namespace detail
}  // namespace back
}  // namespace hsm

#endif
