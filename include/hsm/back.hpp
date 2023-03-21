/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#pragma once

#include <hsm/hsm_fwd.hpp>
#include <hsm/front.hpp>
#include <hsm/detail/hsm.hpp>
#include <hsm/detail/back.hpp>

#include <kvasir/mpl/algorithm/fold_left.hpp>
#include <kvasir/mpl/algorithm/transform.hpp>
#include <kvasir/mpl/algorithm/count_if.hpp>
#include <kvasir/mpl/algorithm/replace_if.hpp>
#include <kvasir/mpl/functions/arithmetic/plus.hpp>
#include <kvasir/mpl/sequence/take.hpp>
#include <kvasir/mpl/sequence/push_back.hpp>
#include <kvasir/mpl/sequence/size.hpp>

namespace hsm
{
namespace back
{
template <size_t StateCount, typename StateId, size_t EventCount, typename EventId, size_t ActionCount, typename ActionId,
          size_t ConditionCount, typename ConditionId, size_t TransitionCount, typename TransitionOffset, size_t EnterCount,
          size_t ExitCount, size_t HistoryCount, typename HistoryId>
struct sm_traits
{
    using state_id                                = StateId;
    using event_id                                = EventId;
    using action_id                               = ActionId;
    using condition_id                            = ConditionId;
    using history_id                              = HistoryId;
    using transition_offset                       = TransitionOffset;
    using tt_entry                                = hsm::detail::tt_entry<event_id, state_id, condition_id, action_id>;
    using state_entry                             = hsm::detail::state_entry<state_id, transition_offset, action_id, history_id>;
    static constexpr state_id    count            = StateCount;
    static constexpr ActionId    action_count     = ActionCount;
    static constexpr ConditionId condition_count  = ConditionCount;
    static constexpr size_t      transition_count = TransitionCount;
    static constexpr size_t      enter_count      = EnterCount;
    static constexpr size_t      exit_count       = ExitCount;
    static constexpr size_t      history_count    = HistoryCount;
};

struct history_table
{
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
struct condition_flag : kvasir::mpl::uint_<static_cast<uint8_t>(transition_flags::has_condition)>
{
};
template <>
struct condition_flag<no_cond> : kvasir::mpl::uint_<0>
{
};

template <typename T>
struct action_flag : kvasir::mpl::uint_<static_cast<uint8_t>(transition_flags::has_action)>
{
};
template <>
struct action_flag<no_action> : kvasir::mpl::uint_<0>
{
};

template <uint32_t Flags, size_t Id, size_t Size, size_t ParentId, size_t Entry, size_t Exit, size_t H, typename... Transitions>
struct state
{
    using transitions                                   = kvasir::mpl::list<Transitions...>;
    static constexpr size_t            id               = Id;
    static constexpr size_t            parent           = ParentId;
    static constexpr size_t            children_count   = Size;
    static constexpr size_t            entry            = Entry;
    static constexpr size_t            exit             = Exit;
    static constexpr size_t            history          = H;
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

template <typename SM, size_t Parent_id, size_t Count, size_t EvIds, size_t TsCount, size_t EnterCount, size_t ExitCount>
struct assembly_status
{
    using type                               = SM;
    static constexpr size_t count            = Count;
    static constexpr size_t event_count      = EvIds;
    static constexpr size_t transition_count = TsCount;
    static constexpr size_t enter_count      = EnterCount;
    static constexpr size_t exit_count       = ExitCount;
};
template <typename W>
struct wrap
{
    using type = W;
};
template <bool b>
struct if_
{
    template <typename T, typename E>
    using f = typename E::type;
};

template <>
struct if_<true>
{
    template <typename T, typename E>
    using f = typename T::type;
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

template <typename T>
struct get_history_flag;
template <typename T>
struct get_history_flag<history_state<T>> : kvasir::mpl::uint_<static_cast<uint32_t>(state_flags::has_history)>
{
};
template <typename T>
struct get_history_flag<deep_history_state<T>> : kvasir::mpl::uint_<static_cast<uint32_t>(state_flags::has_deep_history)>
{
};

template <uint32_t Flags, typename... Ps>
struct combine_flags : kvasir::mpl::uint_<Flags>
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

namespace km  = kvasir::mpl;
namespace hbd = hsm::back::detail;
struct assemble_state_machine
{
    template <typename CS, typename Part>
    struct f_impl
    {
        using type = CS;
    };

    template <typename... Items, size_t P, size_t SC, size_t EC, size_t TC, size_t EnterC, size_t ExitC, typename TT, typename S,
              typename E, typename C, typename A, typename D>
    struct f_impl<assembly_status<tiny_tuple::map<Items...>, P, SC, EC, TC, EnterC, ExitC>, hsm::transition<TT, S, E, C, A, D>>
    {
        using sm   = tiny_tuple::map<Items...>;
        using type = typename if_<std::is_same_v<no_event, E> || tiny_tuple::has_key<unpack<E>, sm>::value>::template f<
            wrap<assembly_status<sm, P, SC, EC, TC + 1, EnterC, ExitC>>,
            wrap<assembly_status<tiny_tuple::map<Items..., tiny_tuple::detail::item<unpack<E>, km::uint_<EC>>>, P, SC, EC + 1, TC + 1,
                                 EnterC, ExitC>>>;
    };

    template <typename SM, size_t P, size_t SC, size_t EC, size_t TC, size_t EnterC, size_t ExitC, typename A>
    struct f_impl<assembly_status<SM, P, SC, EC, TC, EnterC, ExitC>, hsm::entry_action<A>>
    {
        using type = assembly_status<SM, P, SC, EC, TC, EnterC + 1, ExitC>;
    };

    template <typename... Items, size_t P, size_t SC, size_t EC, size_t TC, size_t EnterC, size_t ExitC, typename A>
    struct f_impl<assembly_status<tiny_tuple::map<Items...>, P, SC, EC, TC, EnterC, ExitC>, hsm::exit_action<A>>
    {
        using type = assembly_status<tiny_tuple::map<Items...>, P, SC, EC, TC, EnterC, ExitC + 1>;
    };

    template <typename... Items, size_t P, size_t SC, size_t EC, size_t TC, size_t EnterC, size_t ExitC, typename K>
    struct f_impl<assembly_status<tiny_tuple::map<Items...>, P, SC, EC, TC, EnterC, ExitC>, hsm::state_ref<K>>
    {
        using sm   = tiny_tuple::map<Items...>;
        using type = typename if_<tiny_tuple::has_key<K, sm>::value>::template f<
            wrap<assembly_status<sm, P, SC, EC, TC, EnterC, ExitC>>,
            wrap<assembly_status<tiny_tuple::map<Items..., tiny_tuple::detail::item<unpack<K>, back::state<0, SC, 0, P, 0, 0, 0>>>, P,
                                 SC + 1, EC, TC, EnterC, ExitC>>>;
    };
    template <typename... Items, size_t P, size_t SC, size_t EC, size_t TC, size_t EnterC, size_t ExitC, typename K>
    struct f_impl<assembly_status<tiny_tuple::map<Items...>, P, SC, EC, TC, EnterC, ExitC>, hsm::event<K>>
    {
        using sm   = tiny_tuple::map<Items...>;
        using type = typename if_<tiny_tuple::has_key<K, sm>::value>::template f<
            wrap<assembly_status<sm, P, SC, EC, TC, EnterC, ExitC>>,
            wrap<assembly_status<tiny_tuple::map<Items..., tiny_tuple::detail::item<unpack<K>, km::uint_<EC>>>, P, SC, EC + 1, TC, EnterC,
                                 ExitC>>>;
    };

    template <typename... Items, size_t P, size_t SC, size_t EC, size_t TC, size_t EnterC, size_t ExitC, typename K, typename... Elements>
    struct f_impl<assembly_status<tiny_tuple::map<Items...>, P, SC, EC, TC, EnterC, ExitC>, hsm::state<K, Elements...>>
    {
        constexpr static uint32_t id = SC;
        using back_state = km::call<km::push_front<assembly_status<tiny_tuple::map<Items...>, id, SC + 1, EC, TC, EnterC, ExitC>,
                                                   km::fold_left<assemble_state_machine>>,
                                    Elements...>;
        constexpr static uint32_t size = back_state::count - id - 1;
        using final_table              = km::call<
            km::unpack<km::push_back<tiny_tuple::detail::item<unpack<K>, back::state<0, id, size, P, 0, 0, 0>>, km::cfe<tiny_tuple::map>>>,
            typename back_state::type>;

        using type = assembly_status<final_table, P, back_state::count, back_state::event_count, back_state::transition_count,
                                     back_state::enter_count, back_state::exit_count>;
    };

    template <typename State, typename Param>
    using f = typename f_impl<State, Param>::type;
};

template <typename SM, typename CurrentState, size_t ACount = 0, size_t CCount = 0>
struct attach_transition_state
{
    using type                              = SM;
    static constexpr size_t action_count    = ACount;
    static constexpr size_t condition_count = CCount;
};

template <typename C, typename T>
using get_state = typename detail::get_state_impl<C, T>::type;

template <typename SM, typename E>
using get_event_id = std::decay_t<decltype(tiny_tuple::get<unpack<E>>(std::declval<SM>()))>;

template <typename SM, typename S>
using get_state_id = kvasir::mpl::uint_<std::decay_t<decltype(tiny_tuple::get<unpack<S>>(std::declval<SM>()))>::id>;

template <typename Source, typename Destination, typename FrontTransition, typename Transition>
struct apply_transition
{
    template <typename T>
    struct f_impl
    {
        using type = T;
    };

    template <uint32_t Flags, size_t Id, size_t Size, size_t ParentId, size_t Entry, size_t Exit, size_t H, typename... Transitions, bool F>
    struct f_impl<tiny_tuple::detail::item<Source, hsm::back::state<Flags, Id, Size, ParentId, Entry, Exit, H, Transitions...>, F>>
    {
        static constexpr uint32_t combined_flags =
            combine_flags<Flags, typename FrontTransition::source_type, typename FrontTransition::transition_type>::value;
        using type =
            tiny_tuple::detail::item<Source, state<combined_flags, Id, Size, ParentId, Entry, Exit, H, Transitions..., Transition>>;
    };
    template <typename T>
    using f = typename f_impl<T>::type;
};

template <typename Source, size_t history_flag, size_t DestId, size_t HistoryId>
struct apply_history
{
    template <typename T>
    struct f_impl
    {
        using type = T;
    };

    template <uint32_t Flags, size_t Id, size_t Size, size_t ParentId, size_t Entry, size_t Exit, typename... Transitions, bool F>
    struct f_impl<tiny_tuple::detail::item<Source, hsm::back::state<Flags, Id, Size, ParentId, Entry, Exit, 0, Transitions...>, F>>
    {
        using type =
            tiny_tuple::detail::item<Source, state<Flags | history_flag, Id, Size, ParentId, Entry, Exit, HistoryId, Transitions...>>;
    };
    template <size_t... Is, bool F>
    struct f_impl<tiny_tuple::detail::item<history_table, std::integer_sequence<size_t, Is...>, F>>
    {
        using type = tiny_tuple::detail::item<history_table, std::integer_sequence<size_t, Is..., DestId>>;
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

    template <uint32_t Flags, size_t Id, size_t Size, size_t ParentId, size_t Exit, size_t H, typename... Transitions, bool F>
    struct f_impl<tiny_tuple::detail::item<Source, back::state<Flags, Id, Size, ParentId, 0, Exit, H, Transitions...>, F>>
    {
        using type = tiny_tuple::detail::item<
            Source, state<Flags | static_cast<uint32_t>(state_flags::has_entry), Id, Size, ParentId, ActionId, Exit, H, Transitions...>>;
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

    template <uint32_t Flags, size_t Id, size_t Size, size_t ParentId, size_t Entry, size_t H, typename... Transitions, bool F>
    struct f_impl<tiny_tuple::detail::item<Source, back::state<Flags, Id, Size, ParentId, Entry, 0, H, Transitions...>, F>>
    {
        using type = tiny_tuple::detail::item<
            Source, state<Flags | static_cast<uint32_t>(state_flags::has_exit), Id, Size, ParentId, Entry, ActionId, H, Transitions...>>;
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

    template <typename... Items, typename Current, size_t AC, size_t CC, typename A>
    struct f_impl<attach_transition_state<tiny_tuple::map<Items...>, Current, AC, CC>, hsm::entry_action<A>>
    {
        using type =
            attach_transition_state<km::call<km::transform<apply_entry_action<unpack<Current>, AC>, km::cfe<tiny_tuple::map>>, Items...>,
                                    Current, AC + 1, CC>;
    };

    template <typename... Items, typename Current, size_t AC, size_t CC, typename A>
    struct f_impl<attach_transition_state<tiny_tuple::map<Items...>, Current, AC, CC>, hsm::exit_action<A>>
    {
        using type =
            attach_transition_state<km::call<km::transform<apply_exit_action<unpack<Current>, AC>, km::cfe<tiny_tuple::map>>, Items...>,
                                    Current, AC + 1, CC>;
    };

    template <typename... Items, typename Current, size_t AC, size_t CC, typename S, typename D>
    struct f_impl<attach_transition_state<tiny_tuple::map<Items...>, Current, AC, CC>,
                  hsm::transition<empty_history, S, no_event, no_cond, no_action, D>>
    {
        using sm                        = tiny_tuple::map<Items...>;
        using source_state              = get_state<Current, S>;
        using dest_state                = get_state<Current, D>;
        constexpr static size_t dest_id = get_state_id<sm, dest_state>::value;
        using history                   = typename tiny_tuple::value_type<history_table, sm>::type::value;
        using type                      = attach_transition_state<
            typename km::call<km::transform<apply_history<unpack<source_state>, get_history_flag<S>::value, dest_id, history::size()>,
                                            km::cfe<tiny_tuple::map>>,
                              Items...>,
            Current, AC, CC>;
    };

    template <typename... Items, typename Current, size_t AC, size_t CC, typename TT, typename S, typename E, typename C, typename A,
              typename D>
    struct f_impl<attach_transition_state<tiny_tuple::map<Items...>, Current, AC, CC>, hsm::transition<TT, S, E, C, A, D>>
    {
        using sm                          = tiny_tuple::map<Items...>;
        using source_state                = get_state<Current, S>;
        using dest_state                  = get_state<Current, D>;
        constexpr static size_t event_id  = get_event_id<sm, E>::value;
        constexpr static size_t source_id = get_state_id<sm, source_state>::value;
        constexpr static size_t dest_id   = get_state_id<sm, dest_state>::value;
        using transition_entry            = back::transition<to_flag<TT>::value | source_flag<S>::value | dest_flag<D>::value |
                                                      condition_flag<C>::value | action_flag<A>::value,
                                                  event_id, dest_id, detail::count<C>::value ? CC : +0, detail::count<A>::value ? AC : +0>;
        using front_transition            = hsm::transition<TT, S, E, C, A, D>;
        using type                        = attach_transition_state<
            typename km::call<
                km::transform<apply_transition<unpack<source_state>, D, front_transition, transition_entry>, km::cfe<tiny_tuple::map>>,
                Items...>,
            Current, AC + detail::count<A>::value, CC + detail::count<C>::value>;
    };

    template <typename Map, typename P, size_t AC, size_t CC, typename K, typename... Elements>
    struct f_impl<attach_transition_state<Map, P, AC, CC>, ::hsm::state<K, Elements...>>
    {
        using recurse = km::call<km::push_front<attach_transition_state<Map, K, AC, CC>, km::fold_left<attach_transitions>>, Elements...>;
        using type    = attach_transition_state<typename recurse::type, P, recurse::action_count, recurse::condition_count>;
    };

    template <typename State, typename Param>
    using f = typename f_impl<State, Param>::type;
};

template <typename C = kvasir::mpl::listify>
struct flatten_state_machine
{
    template <typename... Ts>
    using f = typename kvasir::mpl::join<C>::template f<typename detail::flatten_state_machine<C, Ts>::type...>::type;
};

template <typename C = kvasir::mpl::listify>
struct flatten_transition_actions
{
    template <typename... Ts>
    using f = typename kvasir::mpl::join<C>::template f<typename detail::flatten_transition_actions<Ts>::type...>;
};

template <typename C = kvasir::mpl::listify>
struct flatten_transition_conditions
{
    template <typename... Ts>
    using f = typename kvasir::mpl::join<C>::template f<typename detail::flatten_transition_conditions<Ts>::type...>;
};

template <typename SM>
using extract_actions =
    km::call<hsm::back::flatten_transition_actions<km::filter<back::detail::is_action, km::transform<back::detail::function_type>>>, SM>;

template <typename SM>
using extract_conditions =
    km::call<flatten_transition_conditions<km::filter<back::detail::is_condition, km::transform<back::detail::function_type>>>, SM>;

template <typename SM>
using extract_backend_states = km::call<  //
    km::unpack<                           //
        km::filter<back::detail::is_any_state,
                   km::stable_sort<back::detail::by_state_id,                                  //
                                   km::transform<km::cfe<tiny_tuple::detail::value_type>>>>>,  //
    SM>;

template <typename StateList>
using extract_and_sort_transitions = km::call<                    //
    km::unpack<                                                   //
        km::transform<                                            //
            back::detail::unpack_transitions<                     //
                km::stable_sort<back::detail::sort_transition>>,  //
            km::join<>>>,                                         //
    StateList>;

#if (!defined(__GNUC__)) || (__GNUC__ > 5)
#define HSM_USE_DIRECT_ARRAY_INITIALIZATION
#endif
#ifdef HSM_USE_DIRECT_ARRAY_INITIALIZATION
template <typename STE>
struct state_table
{
    template <typename... States>
    struct array
    {
        static constexpr STE table[] = {STE{
            static_cast<typename STE::transition_table_offset_type>(  //
                km::call<                                             //
                    km::take<                                         //
                        km::uint_<States::id>,                        //
                        km::transform<                                //
                            hbd::extract_transition_count,            //
                            km::push_front<                           //
                                km::uint_<0>,                         //
                                km::fold_left<km::plus<>>             //
                                >                                     //
                            >                                         //
                        >,
                    States...>::value),
            static_cast<typename STE::action_id>(States::entry),
            static_cast<typename STE::action_id>(States::exit),
            static_cast<typename STE::state_id>(States::parent),
            static_cast<typename STE::state_id>(States::children_count),
            static_cast<uint16_t>(States::transition_count),
            static_cast<uint8_t>(States::transition_count -
                                 kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::count_if<back::detail::normal_transition>>,
                                                   typename States::transitions>::value),
            static_cast<back::state_flags>(States::flags),
            static_cast<typename STE::history_id>(States::history),
        }...};
    };
};

template <typename STE, typename... States>
auto get_state_table(kvasir::mpl::list<States...>) noexcept
{
    return state_table<STE>::template array<States...>::table;
}
#else

template <typename STE, typename... States>
auto get_state_table(kvasir::mpl::list<States...>) noexcept
{
    static constexpr STE table[] = {STE{
        static_cast<typename STE::transition_table_offset_type>(  //
            km::call<                                             //
                km::take<                                         //
                    km::uint_<States::id>,                        //
                    km::transform<                                //
                        hbd::extract_transition_count,            //
                        km::push_front<                           //
                            km::uint_<0>,                         //
                            km::fold_left<km::plus<>>             //
                            >                                     //
                        >                                         //
                    >,
                States...>::value),
        static_cast<typename STE::action_id>(States::entry),
        static_cast<typename STE::action_id>(States::exit),
        static_cast<typename STE::state_id>(States::parent),
        static_cast<typename STE::state_id>(States::children_count),
        static_cast<uint16_t>(States::transition_count),
        static_cast<uint8_t>(States::transition_count -
                             kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::count_if<back::detail::normal_transition>>,
                                               typename States::transitions>::value),
        static_cast<back::state_flags>(States::flags),
        static_cast<typename STE::history_id>(States::history),
    }...};

    return table;
}
#endif

#ifdef HSM_USE_DIRECT_ARRAY_INITIALIZATION
template <typename TTE>
struct transition_table
{
    template <typename... Ts>
    struct array
    {
        static constexpr TTE table[sizeof...(Ts)] = {TTE{typename TTE::event_id(Ts::event),                       //
                                                         typename TTE::state_id(Ts::dest),                        //
                                                         static_cast<typename TTE::condition_id>(Ts::condition),  //
                                                         static_cast<typename TTE::action_id>(Ts::action),        //
                                                         static_cast<back::transition_flags>(Ts::flags)}...};
    };
};
template <typename TTE, typename... Transitions>
auto get_transition_table(kvasir::mpl::list<Transitions...>) noexcept
{
    return transition_table<TTE>::template array<Transitions...>::table;
}
#else
template <typename TTE, typename... Ts>
auto get_transition_table(kvasir::mpl::list<Ts...>) noexcept
{
    static constexpr TTE table[sizeof...(Ts)] = {TTE{typename TTE::event_id(Ts::event),                       //
                                                     typename TTE::state_id(Ts::dest),                        //
                                                     static_cast<typename TTE::condition_id>(Ts::condition),  //
                                                     static_cast<typename TTE::action_id>(Ts::action),        //
                                                     static_cast<back::transition_flags>(Ts::flags)}...};
    return table;
}
#endif

namespace detail
{
template <typename T>
struct empty_object
{
    struct T1
    {
        char m;
    };
    struct T2 : T
    {
        char m;
    };
    union Storage
    {
        Storage() noexcept : t1{} {}
        T1 t1;
        T2 t2;
    };
    static T get() noexcept
    {
        Storage     storage{};
        char const* m  = &storage.t2.m;
        T2 const*   t2 = reinterpret_cast<T2 const*>(m);
        T const*    t  = static_cast<T const*>(t2);
        return *t;
    }
};
}  // namespace detail

template <typename Context, typename... Cs>
auto get_conditions(kvasir::mpl::list<Cs...>) noexcept
{
    using C                       = bool (*)(Context&);
    static C table[sizeof...(Cs)] = {detail::empty_object<Cs>::get()...};
    return table;
}

template <typename Context, typename... As>
auto get_actions(kvasir::mpl::list<As...>) noexcept
{
    using C                       = void (*)(Context&);
    static C table[sizeof...(As)] = {detail::empty_object<As>::get()...};
    return table;
}
template <typename StateIdType, typename T, T... Is>
constexpr auto get_history(std::integer_sequence<T, Is...>) -> std::array<StateIdType, sizeof...(Is)>
{
    return {static_cast<StateIdType>(Is)...};
}

}  // namespace back
}  // namespace hsm
