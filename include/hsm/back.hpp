/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef HSM_BACK_HPP_INCLUDED
#define HSM_BACK_HPP_INCLUDED
#include "hsm/hsm_fwd.hpp"
#include "hsm/front.hpp"
#include "hsm/detail/hsm.hpp"
#include "hsm/detail/back.hpp"

#include <kvasir/mpl/algorithm/fold_left.hpp>
#include <kvasir/mpl/algorithm/transform.hpp>
#include <kvasir/mpl/algorithm/count_if.hpp>
#include <kvasir/mpl/algorithm/replace_if.hpp>
#include <kvasir/mpl/functions/arithmetic/plus.hpp>
#include <kvasir/mpl/sequence/take.hpp>
#include <kvasir/mpl/sequence/push_back.hpp>
namespace hsm
{
namespace back
{
template <size_t StateCount, typename StateId, size_t EventCount, typename EventId, size_t ActionCount, typename ActionId,
          size_t ConditionCount, typename ConditionId, size_t TransitionCount, typename TransitionOffset>
struct sm_traits
{
    using state_id                                = StateId;
    using event_id                                = EventId;
    using action_id                               = ActionId;
    using condition_id                            = ConditionId;
    using transition_offset                       = TransitionOffset;
    using tt_entry                                = hsm::detail::tt_entry<event_id, state_id, condition_id, action_id>;
    using state_entry                             = hsm::detail::state_entry<state_id, transition_offset, action_id>;
    static constexpr state_id    count            = StateCount;
    static constexpr ActionId    action_count     = ActionCount;
    static constexpr ConditionId condition_count  = ConditionCount;
    static constexpr size_t      transition_count = TransitionCount;
};

template <typename T>
struct to_flag : kvasir::mpl::uint_<static_cast<uint8_t>(transition_flags::normal)>
{
};

template <>
struct to_flag<hsm::completion> : kvasir::mpl::uint_<static_cast<uint8_t>(transition_flags::completion)>
{
};
template <>
struct to_flag<hsm::start> : kvasir::mpl::uint_<static_cast<uint8_t>(transition_flags::initial)>
{
};
template <>
struct to_flag<hsm::empty_history> : kvasir::mpl::uint_<static_cast<uint8_t>(transition_flags::history)>
{
};
template <>
struct to_flag<hsm::internal_transition> : kvasir::mpl::uint_<static_cast<uint8_t>(transition_flags::internal)>
{
};

template <typename T>
struct source_flag : kvasir::mpl::uint_<0>
{
};

template <typename D>
struct dest_flag : kvasir::mpl::uint_<0>
{
};
template <typename C>
struct dest_flag<final_state<C>> : kvasir::mpl::uint_<static_cast<uint8_t>(transition_flags::to_final)>
{
};
template <typename C>
struct dest_flag<history_state<C>> : kvasir::mpl::uint_<static_cast<uint8_t>(transition_flags::to_shallow_history)>
{
};
template <typename C>
struct dest_flag<deep_history_state<C>> : kvasir::mpl::uint_<static_cast<uint8_t>(transition_flags::to_deep_history)>
{
};

template <typename T>
struct condition_flag : kvasir::mpl::uint_<0>
{
};
template <typename T, size_t Id>
struct condition_flag<condition_node<T, Id>> : kvasir::mpl::uint_<static_cast<uint8_t>(transition_flags::has_condition)>
{
};

template <typename T>
struct action_flag : kvasir::mpl::uint_<0>
{
};
template <typename T, size_t Id>
struct action_flag<action_node<T, Id>> : kvasir::mpl::uint_<static_cast<uint8_t>(transition_flags::has_action)>
{
};

template <uint32_t Flags, size_t Id, size_t Size, size_t ParentId, size_t Entry, size_t Exit, typename... Transitions>
struct state
{
    using transitions                                   = kvasir::mpl::list<Transitions...>;
    static constexpr size_t            id               = Id;
    static constexpr size_t            parent           = ParentId;
    static constexpr size_t            children_count   = Size;
    static constexpr size_t            entry            = Entry;
    static constexpr size_t            exit             = Exit;
    static constexpr size_t            transition_count = sizeof...(Transitions);
    static constexpr back::state_flags flags            = static_cast<back::state_flags>(Flags);
};

template <uint32_t Flags, size_t E, size_t D, size_t C, size_t A>
struct transition
{
    static constexpr back::transition_flags flags     = static_cast<back::transition_flags>(Flags);
    static constexpr size_t                 dest      = D;
    static constexpr size_t                 event     = E;
    static constexpr size_t                 condition = C;
    static constexpr size_t                 action    = A;
};

template <typename SM, size_t Parent_id = 0, size_t Count = 0, size_t EvIds = 0, size_t TsCount = 0>
struct assembly_status
{
    using type                               = SM;
    static constexpr size_t count            = Count;
    static constexpr size_t event_count      = EvIds;
    static constexpr size_t transition_count = TsCount;
};

template <bool b>
struct if_
{
    template <typename T, typename E>
    using f = E;
};

template <>
struct if_<true>
{
    template <typename T, typename E>
    using f = T;
};

template <typename T>
struct unpack_impl
{
    using type = T;
};
template <typename T>
struct unpack_impl<state_ref<T>>
{
    using type = T;
};
template <typename T>
struct unpack_impl<event<T>>
{
    using type = T;
};

template <typename T>
using unpack = typename unpack_impl<T>::type;

template <uint32_t Flags, typename... Ps>
struct combine_flags : kvasir::mpl::uint_<Flags>
{
};
template <uint32_t Flags, typename T>
struct combine_flags<Flags, history_state<T>, empty_history> : kvasir::mpl::uint_<Flags | cast(state_flags::has_history)>
{
};
template <uint32_t Flags, typename T>
struct combine_flags<Flags, deep_history_state<T>, empty_history>
    : kvasir::mpl::uint_<Flags | static_cast<uint32_t>(state_flags::has_deep_history)>
{
};
template <uint32_t Flags, typename T>
struct combine_flags<Flags, T, start>
    : kvasir::mpl::uint_<Flags | ((Flags & static_cast<uint32_t>(state_flags::has_initial_transition))
                                      ? static_cast<uint32_t>(state_flags::has_mulitple_initial_transition)
                                      : static_cast<uint32_t>(state_flags::has_initial_transition))>
{
};

template <uint32_t Flags, typename T>
struct combine_flags<Flags, T, completion> : kvasir::mpl::uint_<Flags | static_cast<uint32_t>(state_flags::has_default_transition)>
{
};

namespace km = kvasir::mpl;
struct assemble_state_machine
{
    template <typename CS, typename Part>
    struct f_impl
    {
        using type = CS;
    };

    template <typename... Items, size_t P, size_t SC, size_t EC, size_t TC, typename TT, typename S, typename E, typename C, typename A,
              typename D>
    struct f_impl<assembly_status<tiny_tuple::map<Items...>, P, SC, EC, TC>, hsm::transition<TT, S, E, C, A, D>>
    {
        using sm   = tiny_tuple::map<Items...>;
        using type = typename if_<std::is_same<no_event, E>::value || tiny_tuple::has_key<unpack<E>, sm>::value>::template f<
            assembly_status<sm, P, SC, EC, TC + 1>,
            assembly_status<tiny_tuple::map<Items..., tiny_tuple::detail::item<unpack<E>, km::uint_<EC>>>, P, SC, EC + 1, TC + 1>>;
    };

    template <typename... Items, size_t P, size_t SC, size_t EC, size_t TC, typename K>
    struct f_impl<assembly_status<tiny_tuple::map<Items...>, P, SC, EC, TC>, hsm::state_ref<K>>
    {
        using sm   = tiny_tuple::map<Items...>;
        using type = typename if_<tiny_tuple::has_key<K, sm>::value>::template f<
            assembly_status<sm, P, SC, EC, TC>,
            assembly_status<tiny_tuple::map<Items..., tiny_tuple::detail::item<unpack<K>, back::state<0, SC, 0, P, 0, 0>>>, P, SC + 1, EC,
                            TC>>;
    };
    template <typename... Items, size_t P, size_t SC, size_t EC, size_t TC, typename K>
    struct f_impl<assembly_status<tiny_tuple::map<Items...>, P, SC, EC, TC>, hsm::event<K>>
    {
        using sm   = tiny_tuple::map<Items...>;
        using type = typename if_<tiny_tuple::has_key<K, sm>::value>::template f<
            assembly_status<sm, P, SC, EC, TC>,
            assembly_status<tiny_tuple::map<Items..., tiny_tuple::detail::item<unpack<K>, km::uint_<EC>>>, P, SC, EC + 1, TC>>;
    };

    template <typename... Items, size_t P, size_t SC, size_t EC, size_t TC, typename K, typename... Elements>
    struct f_impl<assembly_status<tiny_tuple::map<Items...>, P, SC, EC, TC>, hsm::state<K, Elements...>>
    {
        constexpr static uint32_t id = SC;
        using back_state =
            km::call<km::push_front<assembly_status<tiny_tuple::map<Items...>, id, SC + 1, EC, TC>, km::fold_left<assemble_state_machine>>,
                     Elements...>;
        constexpr static uint32_t size = back_state::count - id - 1;
        using final_table              = km::call<
            km::unpack<km::push_back<tiny_tuple::detail::item<unpack<K>, back::state<0, id, size, P, 0, 0>>, km::cfe<tiny_tuple::map>>>,
            typename back_state::type>;

        using type = assembly_status<final_table, P, back_state::count, back_state::event_count, back_state::transition_count>;
    };

    template <typename State, typename Param>
    using f = typename f_impl<State, Param>::type;
};

template <typename SM, typename CurrentState>
struct attach_transition_state
{
    using type = SM;
};

template <typename C, typename T>
using get_state = typename detail::get_state_impl<C, T>::type;

template <typename SM, typename E>
using get_event_id = std::decay_t<decltype(tiny_tuple::get<unpack<E>>(std::declval<SM>()))>;

template <typename SM, typename S>
using get_state_id = kvasir::mpl::uint_<std::decay_t<decltype(tiny_tuple::get<unpack<S>>(std::declval<SM>()))>::id>;

template <typename Action>
using get_action_id = typename detail::get_action_impl<Action>::type;

template <typename Condition>
using get_condition_id = typename detail::get_condition_impl<Condition>::type;

template <typename Source, typename FrontTransition, typename Transition>
struct apply_transition
{
    template <typename T>
    struct f_impl
    {
        using type = T;
    };

    template <uint32_t Flags, size_t Id, size_t Size, size_t ParentId, size_t Entry, size_t Exit, typename... Transitions, bool F>
    struct f_impl<tiny_tuple::detail::item<Source, hsm::back::state<Flags, Id, Size, ParentId, Entry, Exit, Transitions...>, F>>
    {
        static constexpr uint32_t combined_flags =
            combine_flags<Flags, typename FrontTransition::source_type, typename FrontTransition::transition_type>::value;
        using type =
            tiny_tuple::detail::item<Source, state<combined_flags, Id, Size, ParentId, Entry, Exit, Transitions..., Transition>, F>;
    };
    template <typename T>
    using f = typename f_impl<T>::type;
};

template <typename Source, size_t ActionId>
struct apply_entry_action
{
    template <typename T>
    struct f_impl
    {
        using type = T;
    };

    template <uint32_t Flags, size_t Id, size_t Size, size_t ParentId, size_t Entry, size_t Exit, typename... Transitions, bool F>
    struct f_impl<tiny_tuple::detail::item<Source, back::state<Flags, Id, Size, ParentId, Entry, Exit, Transitions...>, F>>
    {
        using type = tiny_tuple::detail::item<
            Source, state<Flags | static_cast<uint32_t>(state_flags::has_entry), Id, Size, ParentId, ActionId, Exit, Transitions...>>;
    };
    template <typename T>
    using f = typename f_impl<T>::type;
};

template <typename Source, size_t ActionId>
struct apply_exit_action
{
    template <typename T>
    struct f_impl
    {
        using type = T;
    };

    template <uint32_t Flags, size_t Id, size_t Size, size_t ParentId, size_t Entry, size_t Exit, typename... Transitions, bool F>
    struct f_impl<tiny_tuple::detail::item<Source, back::state<Flags, Id, Size, ParentId, Entry, Exit, Transitions...>, F>>
    {
        using type = tiny_tuple::detail::item<
            Source, state<Flags | static_cast<uint32_t>(state_flags::has_exit), Id, Size, ParentId, Entry, ActionId, Transitions...>>;
    };
    template <typename T>
    using f = typename f_impl<T>::type;
};

struct attach_transitions
{
    template <typename L, typename R>
    struct f_impl
    {
        using type = L;
    };

    template <typename... Items, typename Current, typename A, size_t Id>
    struct f_impl<attach_transition_state<tiny_tuple::map<Items...>, Current>, hsm::entry_action<A, Id>>
    {
        using type =
            attach_transition_state<km::call<km::transform<apply_entry_action<Current, Id>, km::cfe<tiny_tuple::map>>, Items...>, Current>;
    };

    template <typename... Items, typename Current, typename A, size_t Id>
    struct f_impl<attach_transition_state<tiny_tuple::map<Items...>, Current>, hsm::exit_action<A, Id>>
    {
        using type =
            attach_transition_state<km::call<km::transform<apply_exit_action<Current, Id>, km::cfe<tiny_tuple::map>>, Items...>, Current>;
    };

    template <typename... Items, typename Current, typename TT, typename S, typename E, typename C, typename A, typename D>
    struct f_impl<attach_transition_state<tiny_tuple::map<Items...>, Current>, hsm::transition<TT, S, E, C, A, D>>
    {
        using sm                          = tiny_tuple::map<Items...>;
        using source_state                = get_state<Current, S>;
        using dest_state                  = get_state<Current, D>;
        constexpr static size_t event_id  = get_event_id<sm, E>::value;
        constexpr static size_t source_id = get_state_id<sm, source_state>::value;
        constexpr static size_t dest_id   = get_state_id<sm, dest_state>::value;
        using transition_entry            = back::transition<to_flag<TT>::value | source_flag<S>::value | dest_flag<D>::value |
                                                      condition_flag<C>::value | action_flag<A>::value,
                                                  event_id, dest_id, get_condition_id<C>::value, get_action_id<A>::value>;
        using front_transition            = hsm::transition<TT, S, E, C, A, D>;
        using type                        = attach_transition_state<
            typename km::call<
                km::transform<apply_transition<unpack<source_state>, front_transition, transition_entry>, km::cfe<tiny_tuple::map>>,
                Items...>,
            Current>;
    };

    template <typename Map, typename P, typename K, typename... Elements>
    struct f_impl<attach_transition_state<Map, P>, ::hsm::state<K, Elements...>>
    {
        using recurse = km::call<km::push_front<attach_transition_state<Map, K>, km::fold_left<attach_transitions>>, Elements...>;
        using type    = attach_transition_state<typename recurse::type, P>;
    };

    template <typename State, typename Param>
    using f = typename f_impl<State, Param>::type;
};

template <size_t ActionCounter = 0, size_t ConditionCounter = 0>
struct counters
{
    static constexpr size_t a_counter = ActionCounter;
    static constexpr size_t c_counter = ConditionCounter;

    using add_item             = counters<a_counter, c_counter>;
    using add_action           = counters<a_counter + 1, c_counter>;
    using add_condition        = counters<a_counter, c_counter + 1>;
    using add_action_condition = counters<a_counter + 1, c_counter + 1>;
};

template <typename Counter, typename K>
constexpr auto enumerate_action_item(Counter, K&& item)
{
    return tiny_tuple::tuple<Counter, K>(Counter{}, std::move(item));
}

template <typename Counter, typename A>
constexpr auto enumerate_action_item(Counter, hsm::entry_action<A>&& item)
{
    using counter = typename Counter::add_action;
    return tiny_tuple::tuple<counter, hsm::entry_action<A, Counter::a_counter>>(
        counter{}, entry_action<A, Counter::a_counter>{std::move(item.action)});
}

template <typename Counter, typename A>
constexpr auto enumerate_action_item(Counter, hsm::exit_action<A>&& item)
{
    using counter = typename Counter::add_action;
    return tiny_tuple::tuple<counter, hsm::exit_action<A, Counter::a_counter>>(counter{},
                                                                               exit_action<A, Counter::a_counter>{std::move(item.action)});
}

template <typename Counter, typename TT, typename S, typename E, typename A, typename D>
constexpr auto enumerate_action_item(Counter, hsm::transition<TT, S, E, no_cond, action_node<A>, D>&& item)
{
    using counter = typename Counter::add_action;
    return tiny_tuple::tuple<counter, hsm::transition<TT, S, E, no_cond, action_node<A, Counter::a_counter>, D>>(
        counter{}, hsm::transition<TT, S, E, no_cond, action_node<A, Counter::a_counter>, D>(
                       no_cond{}, action_node<A, Counter::a_counter>{std::move(item.action.action)}));
}

template <typename Counter, typename TT, typename S, typename E, typename C, typename A, typename D>
constexpr auto enumerate_action_item(Counter, hsm::transition<TT, S, E, condition_node<C>, action_node<A>, D>&& item)
{
    using counter = typename Counter::add_action_condition;
    return tiny_tuple::tuple<counter,
                             hsm::transition<TT, S, E, condition_node<C, Counter::c_counter>, action_node<A, Counter::a_counter>, D>>(
        counter{}, hsm::transition<TT, S, E, condition_node<C, Counter::c_counter>, action_node<A, Counter::a_counter>, D>(
                       condition_node<C, Counter::c_counter>{std::move(item.cond.condition)},
                       action_node<A, Counter::a_counter>{std::move(item.action.action)}));
}

template <typename Counter, typename TT, typename S, typename E, typename C, typename D>
constexpr auto enumerate_action_item(Counter, hsm::transition<TT, S, E, condition_node<C>, no_action, D>&& item)
{
    using counter = typename Counter::add_condition;
    return tiny_tuple::tuple<counter, hsm::transition<TT, S, E, condition_node<C, Counter::c_counter>, no_action, D>>(
        counter{}, hsm::transition<TT, S, E, condition_node<C, Counter::c_counter>, no_action, D>(
                       condition_node<C, Counter::c_counter>{std::move(item.cond.condition)}));
}

template <typename Counter, typename K, typename... Params, typename T, typename... Ts>
constexpr auto enumerate_state_elements(Counter c, hsm::state<K, Params...>&& scope, T&& item)
{
    auto enum_item   = enumerate_action_item(c, std::move(item));
    using enum_state = hsm::state<K, Params..., std::decay_t<decltype(tiny_tuple::get<1>(enum_item))>>;
    using counter    = std::decay_t<decltype(tiny_tuple::get<0>(enum_item))>;
    return tiny_tuple::tuple<counter, enum_state>(counter{},
                                                  enum_state(append(std::move(scope.data), std::move(tiny_tuple::get<1>(enum_item)))));
}

template <typename Counter, typename K, typename... Params, typename T, typename... Ts>
constexpr auto enumerate_state_elements(Counter c, hsm::state<K, Params...>&& scope, T&& item, Ts&&... items)
{
    auto enum_item = enumerate_action_item(c, std::move(item));
    return enumerate_state_elements(tiny_tuple::get<0>(enum_item),
                                    hsm::state<K, Params..., std::decay_t<decltype(std::move(tiny_tuple::get<1>(enum_item)))>>(
                                        tiny_tuple::append(std::move(scope.data), std::move(tiny_tuple::get<1>(enum_item)))),
                                    std::move(items)...);
}

namespace detail
{
template <typename Counter, typename K, typename... Elements, int... Is>
constexpr auto enumerate_state_elements_util(Counter c, hsm::state<K, Elements...>&& item, std::integer_sequence<int, Is...>)
{
    return enumerate_state_elements(c, hsm::state<K>{}, std::move(tiny_tuple::get<Is>(item.data))...);
}
}  // namespace detail

template <typename Counter, typename K, typename... Elements>
constexpr auto enumerate_action_item(Counter c, hsm::state<K, Elements...>&& item)
{
    return detail::enumerate_state_elements_util(c, std::move(item), std::make_integer_sequence<int, sizeof...(Elements)>());
}

template <typename SM, typename Conditions, typename Actions>
constexpr void initialize_ca_array(SM& sm, Conditions& conds, Actions& actions)
{
    detail::initialize_ca_array(sm, conds, actions);
}

template <typename STE, typename... States>
auto get_state_table(kvasir::mpl::list<States...>) noexcept
{
    namespace km                 = kvasir::mpl;
    namespace hbd                = hsm::back::detail;
    static constexpr STE table[] = {STE{
        static_cast<typename STE::transition_table_offset_type>(km::call<                               //
                                                                km::take<                               //
                                                                    km::uint_<States::id>,              //
                                                                    km::transform<                      //
                                                                        hbd::extract_transition_count,  //
                                                                        km::push_front<                 //
                                                                            km::uint_<0>,               //
                                                                            km::fold_left<km::plus<>>   //
                                                                            >                           //
                                                                        >                               //
                                                                    >,
                                                                States...>::value),
        static_cast<typename STE::action_id>(States::entry),
        static_cast<typename STE::action_id>(States::exit),
        static_cast<typename STE::state_id>(States::parent),
        static_cast<typename STE::state_id>(States::children_count),
        static_cast<uint16_t>(States::transition_count),
        static_cast<uint8_t>(kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::count_if<back::detail::normal_transition>>,
                                               typename States::transitions>::value),
        static_cast<back::state_flags>(States::flags),
    }...};
    return table;
}
template <typename TTE, typename... Transitions>
auto get_transition_table(kvasir::mpl::list<Transitions...>) noexcept
{
    static constexpr TTE table[] = {TTE{typename TTE::event_id(Transitions::event),                       //
                                        typename TTE::state_id(Transitions::dest),                        //
                                        static_cast<typename TTE::condition_id>(Transitions::condition),  //
                                        static_cast<typename TTE::action_id>(Transitions::action),        //
                                        static_cast<back::transition_flags>(Transitions::flags)}...};
    return table;
}
}  // namespace back
}  // namespace hsm

#endif
