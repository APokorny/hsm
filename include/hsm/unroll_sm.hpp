/* ==========================================================================
 Copyright (c) 2023 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef HSM_FLATTEN_SM_HPP_INCLUDED
#define HSM_FLATTEN_SM_HPP_INCLUDED

#include <array>
#include <kvasir/mpl/sequence/at.hpp>
#include <kvasir/mpl/algorithm/reverse.hpp>
#include <kvasir/mpl/algorithm/transform.hpp>
#include <hsm/hsm.hpp>
#include <hsm/back.hpp>

namespace hsm
{
namespace back
{

struct exit_count
{
};
struct entry_count
{
};

template <uint8_t Flags, size_t Id, size_t Size, size_t ParentId, typename Entry, typename Exit, size_t H, typename... Transitions>
struct u_state
{
    using transitions                          = kvasir::mpl::list<Transitions...>;
    static constexpr size_t            id      = Id;
    static constexpr size_t            parent  = ParentId;
    static constexpr size_t            size    = Size;
    static constexpr back::state_flags flags   = static_cast<back::state_flags>(Flags);
    static constexpr size_t            history = H;
    using entry                                = Entry;
    using exit                                 = Exit;
    template <typename Context>
    static constexpr void enter_state(Context&& c) noexcept
    {
        return entry{}(c);
    }
    template <typename Context>
    static constexpr void exit_state(Context&& c) noexcept
    {
        return exit{}(c);
    }
};

template <uint8_t Flags, size_t E, size_t D, typename C, typename A>
struct u_transition
{
    static constexpr back::transition_flags flags = static_cast<back::transition_flags>(Flags);
    static constexpr size_t                 dest  = D;
    static constexpr size_t                 event = E;
    using condition                               = C;
    using action                                  = A;
    template <typename Context>
    static constexpr bool eval(Context&& c) noexcept
    {
        return condition{}(c);
    }
    template <typename Context>
    static constexpr void exec(Context&& c) noexcept
    {
        action{}(c);
    }
    static constexpr bool to_history() noexcept
    {
        using enum back::transition_flags;
        return ((flags & dest_mask) == to_shallow_history || (flags & dest_mask) == to_deep_history);
    }
    static constexpr bool to_final() noexcept
    {
        using enum back::transition_flags;
        return (flags & dest_mask) == to_final;
    }
    static constexpr bool is_internal() noexcept
    {
        using enum back::transition_flags;
        return (flags & transition_type_mask) == internal;
    }

    static constexpr bool is_completion() noexcept
    {
        using enum back::transition_flags;
        return (flags & transition_type_mask) == completion;
    }

    static constexpr bool is_initial() noexcept
    {
        using enum back::transition_flags;
        return (flags & transition_type_mask) == initial;
    }
};

template <typename SM, size_t Parent_id, size_t Count, size_t EvIds>
struct u_assembly_status
{
    using type                          = SM;
    static constexpr size_t count       = Count;
    static constexpr size_t event_count = EvIds;
};

namespace km  = kvasir::mpl;
namespace hbd = hsm::back::detail;
struct u_assemble_state_machine
{
    template <typename CS, typename Part>
    struct f_impl
    {
        using type = CS;
    };

    template <typename... Items, size_t P, size_t SC, size_t EC, typename TT, typename S, typename E, typename C, typename A, typename D>
    struct f_impl<u_assembly_status<tiny_tuple::map<Items...>, P, SC, EC>, hsm::transition<TT, S, E, C, A, D>>
    {
        using sm   = tiny_tuple::map<Items...>;
        using type = typename if_<std::is_same_v<no_event, E> || tiny_tuple::has_key<unpack<E>, sm>::value>::template f<
            wrap<u_assembly_status<sm, P, SC, EC>>,
            wrap<u_assembly_status<tiny_tuple::map<Items..., tiny_tuple::detail::item<unpack<E>, km::uint_<EC>>>, P, SC, EC + 1>>>;
    };

    template <typename... Items, size_t P, size_t SC, size_t EC, typename K>
    struct f_impl<u_assembly_status<tiny_tuple::map<Items...>, P, SC, EC>, hsm::state_ref<K>>
    {
        using sm   = tiny_tuple::map<Items...>;
        using type = typename if_<tiny_tuple::has_key<K, sm>::value>::template f<
            wrap<u_assembly_status<sm, P, SC, EC>>,
            wrap<u_assembly_status<
                tiny_tuple::map<Items..., tiny_tuple::detail::item<unpack<K>, back::u_state<0, SC, 0, P, no_action, no_action, 0>>>, P,
                SC + 1, EC>>>;
    };
    template <typename... Items, size_t P, size_t SC, size_t EC, typename K>
    struct f_impl<u_assembly_status<tiny_tuple::map<Items...>, P, SC, EC>, hsm::event<K>>
    {
        using sm   = tiny_tuple::map<Items...>;
        using type = typename if_<tiny_tuple::has_key<K, sm>::value>::template f<
            wrap<u_assembly_status<sm, P, SC, EC>>,
            wrap<u_assembly_status<tiny_tuple::map<Items..., tiny_tuple::detail::item<unpack<K>, km::uint_<EC>>>, P, SC, EC + 1>>>;
    };

    template <typename... Items, size_t P, size_t SC, size_t EC, typename K, typename... Elements>
    struct f_impl<u_assembly_status<tiny_tuple::map<Items...>, P, SC, EC>, hsm::state<K, Elements...>>
    {
        constexpr static uint32_t id = SC;
        using back_state =
            km::call<km::push_front<u_assembly_status<tiny_tuple::map<Items...>, id, SC + 1, EC>, km::fold_left<u_assemble_state_machine>>,
                     Elements...>;
        constexpr static uint32_t size = back_state::count - id - 1;
        using final_table =
            km::call<km::unpack<km::push_back<tiny_tuple::detail::item<unpack<K>, back::u_state<0, id, size, P, no_action, no_action, 0>>,
                                              km::cfe<tiny_tuple::map>>>,
                     typename back_state::type>;

        using type = u_assembly_status<final_table, P, back_state::count, back_state::event_count>;
    };

    template <typename State, typename Param>
    using f = typename f_impl<State, Param>::type;
};

template <typename SM, typename CurrentState>
struct u_attach_transition_state
{
    using type = SM;
};

template <typename Source, typename Destination, typename FrontTransition, typename Transition>
struct u_apply_transition
{
    template <typename T>
    struct f_impl
    {
        using type = T;
    };

    template <uint8_t Flags, size_t Id, size_t S, size_t ParentId, typename Entry, typename Exit, size_t H, typename... Transitions, bool F>
    struct f_impl<tiny_tuple::detail::item<Source, hsm::back::u_state<Flags, Id, S, ParentId, Entry, Exit, H, Transitions...>, F>>
    {
        static constexpr uint8_t combined_flags =
            combine_flags<Flags, typename FrontTransition::source_type, typename FrontTransition::transition_type>::value;
        using type = tiny_tuple::detail::item<Source, u_state<combined_flags, Id, S, ParentId, Entry, Exit, H, Transitions..., Transition>>;
    };
    template <typename T>
    using f = typename f_impl<T>::type;
};

template <typename Source, size_t history_flag, size_t DestId, size_t HistoryId>
struct u_apply_history
{
    template <typename T>
    struct f_impl
    {
        using type = T;
    };

    template <uint8_t Flags, size_t Id, size_t S, size_t ParentId, typename Entry, typename Exit, typename... Transitions, bool F>
    struct f_impl<tiny_tuple::detail::item<Source, hsm::back::u_state<Flags, Id, S, ParentId, Entry, Exit, 0, Transitions...>, F>>
    {
        using type =
            tiny_tuple::detail::item<Source, u_state<Flags | history_flag, Id, S, ParentId, Entry, Exit, HistoryId, Transitions...>>;
    };
    template <size_t... Is, bool F>
    struct f_impl<tiny_tuple::detail::item<history_table, std::integer_sequence<size_t, Is...>, F>>
    {
        using type = tiny_tuple::detail::item<history_table, std::integer_sequence<size_t, Is..., DestId>>;
    };

    template <typename T>
    using f = typename f_impl<T>::type;
};

template <typename Source, typename Action>
struct u_apply_entry_action
{
    template <typename T>
    struct f_impl
    {
        using type = T;
    };

    template <unsigned long long X>
    struct f_impl<tiny_tuple::detail::item<entry_count, km::uint_<X>>>
    {
        using type = tiny_tuple::detail::item<entry_count, km::uint_<X + 1>>;
    };

    template <uint8_t Flags, size_t Id, size_t S, size_t ParentId, typename Exit, size_t H, typename... Transitions, bool F>
    struct f_impl<tiny_tuple::detail::item<Source, back::u_state<Flags, Id, S, ParentId, no_action, Exit, H, Transitions...>, F>>
    {
        using type = tiny_tuple::detail::item<
            Source, u_state<Flags | static_cast<uint8_t>(state_flags::has_entry), Id, S, ParentId, Action, Exit, H, Transitions...>>;
    };

    template <typename T>
    using f = typename f_impl<T>::type;
};

template <typename Source, typename Action>
struct u_apply_exit_action
{
    template <typename T>
    struct f_impl
    {
        using type = T;
    };

    template <unsigned long long X>
    struct f_impl<tiny_tuple::detail::item<exit_count, km::uint_<X>>>
    {
        using type = tiny_tuple::detail::item<exit_count, km::uint_<X + 1>>;
    };

    template <uint8_t Flags, size_t Id, size_t S, size_t P, typename Entry, size_t H, typename... Transitions, bool F>
    struct f_impl<tiny_tuple::detail::item<Source, back::u_state<Flags, Id, S, P, Entry, no_action, H, Transitions...>, F>>
    {
        using type = tiny_tuple::detail::item<
            Source, u_state<Flags | static_cast<uint8_t>(state_flags::has_exit), Id, S, P, Entry, Action, H, Transitions...>>;
    };
    template <typename T>
    using f = typename f_impl<T>::type;
};

struct u_attach_transitions
{
    template <typename L, typename R>
    struct f_impl
    {
        using type = L;
    };

    template <typename... Items, typename Current, typename A>
    struct f_impl<u_attach_transition_state<tiny_tuple::map<Items...>, Current>, hsm::entry_action<A>>
    {
        using type =
            u_attach_transition_state<km::call<km::transform<u_apply_entry_action<unpack<Current>, A>, km::cfe<tiny_tuple::map>>, Items...>,
                                      Current>;
    };

    template <typename... Items, typename Current, typename A>
    struct f_impl<u_attach_transition_state<tiny_tuple::map<Items...>, Current>, hsm::exit_action<A>>
    {
        using type =
            u_attach_transition_state<km::call<km::transform<u_apply_exit_action<unpack<Current>, A>, km::cfe<tiny_tuple::map>>, Items...>,
                                      Current>;
    };

    template <typename... Items, typename Current, typename S, typename D>
    struct f_impl<u_attach_transition_state<tiny_tuple::map<Items...>, Current>,
                  hsm::transition<empty_history, S, no_event, no_cond, no_action, D>>
    {
        using sm                        = tiny_tuple::map<Items...>;
        using source_state              = get_state<Current, S>;
        using dest_state                = get_state<Current, D>;
        constexpr static size_t dest_id = get_state_id<sm, dest_state>::value;
        using history                   = typename tiny_tuple::value_type<history_table, sm>::type::value;
        using type                      = u_attach_transition_state<
            typename km::call<km::transform<u_apply_history<unpack<source_state>, get_history_flag<S>::value, dest_id, history::size()>,
                                            km::cfe<tiny_tuple::map>>,
                              Items...>,
            Current>;
    };

    template <typename... Items, typename Current, typename TT, typename S, typename E, typename C, typename A, typename D>
    struct f_impl<u_attach_transition_state<tiny_tuple::map<Items...>, Current>, hsm::transition<TT, S, E, C, A, D>>
    {
        using sm                          = tiny_tuple::map<Items...>;
        using source_state                = get_state<Current, S>;
        using dest_state                  = get_state<Current, D>;
        constexpr static size_t event_id  = get_event_id<sm, E>::value;
        constexpr static size_t source_id = get_state_id<sm, source_state>::value;
        constexpr static size_t dest_id   = get_state_id<sm, dest_state>::value;
        using transition_entry            = back::u_transition<to_flag<TT>::value | source_flag<S>::value | dest_flag<D>::value |
                                                        condition_flag<C>::value | action_flag<A>::value,
                                                    event_id, dest_id, C, A>;
        using front_transition            = hsm::transition<TT, S, E, C, A, D>;
        using type                        = u_attach_transition_state<
            typename km::call<
                km::transform<u_apply_transition<unpack<source_state>, D, front_transition, transition_entry>, km::cfe<tiny_tuple::map>>,
                Items...>,
            Current>;
    };

    template <typename Map, typename P, typename K, typename... Elements>
    struct f_impl<u_attach_transition_state<Map, P>, ::hsm::state<K, Elements...>>
    {
        using recurse = km::call<km::push_front<u_attach_transition_state<Map, K>, km::fold_left<u_attach_transitions>>, Elements...>;
        using type    = u_attach_transition_state<typename recurse::type, P>;
    };

    template <typename State, typename Param>
    using f = typename f_impl<State, Param>::type;
};

struct internal_and_normal
{
    template <typename T>
    struct f_impl : kvasir::mpl::false_
    {
    };

    template <uint8_t F, size_t E, size_t D, typename C, typename A>
    struct f_impl<back::u_transition<F, E, D, C, A>>
        : kvasir::mpl::bool_<(F & transition_flags::transition_type_mask) == transition_flags::internal ||
                             (F & transition_flags::transition_type_mask) == transition_flags::normal>
    {
    };

    template <typename T>
    using f = f_impl<T>;
};

struct event_transitions
{
    template <typename Param>
    using f = typename kvasir::mpl::call<kvasir::mpl::unpack<kvasir::mpl::filter<internal_and_normal>>, Param>;
};

template <size_t StateId>
struct parent_transitions
{
    template <typename R>
    struct f_impl
    {
        using type = kvasir::mpl::list<>;
    };
    template <uint8_t F, size_t Id, size_t S, size_t P, typename E, typename Ex, size_t H, typename... Ts>
    struct f_impl<back::u_state<F, Id, S, P, E, Ex, H, Ts...>>
    {
        using type = if_<(Id <= StateId) && (StateId <= Id + S)>::template f<wrap<kvasir::mpl::list<Ts...>>, wrap<kvasir::mpl::list<>>>;
    };

    template <typename Param>
    using f = typename f_impl<Param>::type;
};

template <size_t EventId>
struct only_event
{
    template <typename Param>
    using f = typename if_<Param::event == 0 || Param::event == EventId>::template f<wrap<km::true_>, wrap<km::false_>>;
};

template <typename States, size_t EventId, size_t StateId>
using grab_event_transitions_sorted = kvasir::mpl::call<  //
    km::unpack<                                           //
        km::transform<                                    //
            parent_transitions<StateId>,                  //
            km::transform<                                //
                event_transitions,                        //
                km::reverse<                              //
                    km::join<                             //
                        km::filter<                       //
                            only_event<EventId>>          //
                        >                                 //
                    >                                     //
                >                                         //
            >                                             //
        >,                                                //
    States>;

template <typename States, size_t StateId>
using grab_transitions_sorted =
    kvasir::mpl::call<km::unpack<km::transform<parent_transitions<StateId>, km::transform<event_transitions, km::reverse<km::join<>>>>>,
                      States>;

template <typename StateList>
using extract_and_sort_transitions = km::call<                    //
    km::unpack<                                                   //
        km::transform<                                            //
            back::detail::unpack_transitions<                     //
                km::stable_sort<back::detail::sort_transition>>,  //
            km::join<>>>,                                         //
    StateList>;

template <size_t StateCount, typename StateId, size_t EventCount, typename EventId, size_t HistoryCount, typename HistoryId,
          size_t EnterCount, size_t ExitCount>
struct u_sm_traits
{
    using state_id                          = StateId;
    using event_id                          = EventId;
    using history_id                        = HistoryId;
    static constexpr state_id count         = StateCount;
    static constexpr size_t   history_count = HistoryCount;
    static constexpr size_t   enter_count   = EnterCount;
    static constexpr size_t   exit_count    = ExitCount;
};

template <typename States, typename Result, typename Source, typename Dest>
struct calc_static_parent_impl;
template <typename States, typename... Ts, typename Source, typename Dest>
struct calc_static_parent_impl<States, km::list<Ts...>, Source, Dest>
{
    using type = if_<(Source::id <= Dest::id) && (Dest::id <= Source::id + Source::size)>::template f<
        wrap<km::list<Source, Ts...>>,
        calc_static_parent_impl<States, km::list<Source, Ts...>, km::eager::at<States, Source::parent>, Dest>>;
};

template <typename States, size_t Source, size_t Dest>
using calc_static_parent =
    typename calc_static_parent_impl<States, km::list<>, km::eager::at<States, Source>, km::eager::at<States, Dest>>::type;

template <typename States, typename Result, typename Source, typename Dest>
struct get_parent_impl;
template <typename States, typename... Ts, typename Source, typename Dest>
struct get_parent_impl<States, km::list<Ts...>, Source, Dest>
{
    using type = typename get_parent_impl<States, km::list<Dest, Ts...>, Source, km::eager::at<States, Dest::parent>>::type;
};

template <typename States, typename... Ts, typename Same>
struct get_parent_impl<States, km::list<Ts...>, Same, Same>
{
    using type = km::list<Ts...>;
};

template <typename States, size_t Source, size_t Dest>
using get_parents = typename get_parent_impl<States, km::list<>, km::eager::at<States, Source>, km::eager::at<States, Dest>>::type;

struct no_state
{
};
template <size_t History>
struct shallow_history_state
{
    static constexpr size_t first = History;
};
template <size_t History, size_t Stateid>
struct history_setting
{
    static constexpr size_t first  = History;
    static constexpr size_t second = Stateid;
};

template <typename List, typename LastElementState>
struct history_sequence
{
    using type = List;
};
template <size_t CurrentStateId>
struct apply_history_state
{
    template <typename L, typename R>
    struct f_impl;
    template <typename... Ts, size_t H, typename R>
    struct f_impl<history_sequence<km::list<Ts...>, shallow_history_state<H>>, R>
    {
        using setting     = history_setting<H, R::id>;
        using next_status = if_<(static_cast<uint8_t>(R::flags & state_flags::has_history)) ==
                                0>::template f<wrap<no_state>, wrap<shallow_history_state<R::history>>>;
        using type =
            if_<(static_cast<uint8_t>(R::flags & state_flags::has_deep_history) !=
                 0)>::template f<wrap<history_sequence<km::list<Ts..., setting, history_setting<R::history, CurrentStateId>>, next_status>>,
                                 wrap<history_sequence<km::list<Ts..., setting>, next_status>>>;
    };

    template <typename... Ts, typename R>
    struct f_impl<history_sequence<km::list<Ts...>, no_state>, R>
    {
        using next_status = if_<(static_cast<uint8_t>(R::flags & state_flags::has_history)) ==
                                0>::template f<wrap<no_state>, wrap<shallow_history_state<R::history>>>;
        using type        = if_<(static_cast<uint8_t>(R::flags & state_flags::has_deep_history)) !=
                         0>::template f<wrap<history_sequence<km::list<Ts..., history_setting<R::history, CurrentStateId>>, next_status>>,
                                        wrap<history_sequence<km::list<Ts...>, next_status>>>;
    };

    template <typename L, typename R>
    using f = typename f_impl<L, R>::type;
};
}  // namespace back

template <typename FinalSM, typename Context, typename Traits>
struct unrolled_sm
{
    using sm         = FinalSM;
    using states     = back::extract_backend_states<sm>;
    using state_id   = typename Traits::state_id;
    using event_id   = typename Traits::event_id;
    using history_id = typename Traits::history_id;

    template <state_id id>
    using get_state = kvasir::mpl::eager::template at<states, id>;

    state_id current_state{0};

    std::array<state_id, Traits::history_count> history;

    enum dispatch_result
    {
        internal,
        normal,
        to_final,
        no_consume
    };

    constexpr unrolled_sm(std::array<state_id, Traits::history_count>&& a) : history{a} {}

    template <typename... Ts>
    inline void execute_exit_actions(Context& con, kvasir::mpl::list<Ts...>)
    {
        auto foo = {Ts::exit_state(con)...};
    }

    template <typename T, typename State>
    inline auto handle_transition(Context& con, T, State, State)
    {
        namespace km = kvasir::mpl;
        if constexpr (Traits::exit_count > 0) State::exit_state(con);
        T::exec(con);
        if constexpr (Traits::enter_count > 0) State::enter_state(con);
        return true;
    }

    template <typename T, typename Source, typename Dest>
    inline auto handle_transition(Context& con, T t, Source, Dest)
    {
        namespace km    = kvasir::mpl;
        using to_parent = back::calc_static_parent<states, Source::id, Dest::id>;
        if constexpr (Traits::exit_count > 0)
            [this]<typename... Ts>(Context& con, km::list<Ts...>)
            { (Ts::exit_state(con),...); }(con, km::eager::drop<to_parent, 1>());
        if constexpr (Traits::history_count > 0)
        {
            using history_update = km::call<                                     //
                km::unpack<                                                      //
                    km::push_front<                                              //
                        back::history_sequence<km::list<>, back::no_state>,      //
                        km::fold_left<back::apply_history_state<Source::id>>>>,  //
                to_parent>;
            [this]<typename... Ts>(kvasir::mpl::list<Ts...>)
            { ((history[Ts::first] = Ts::second),...); }(typename history_update::type());
        }
        t.exec(con);
        if constexpr (Traits::enter_count > 0)
        {
            using parent = km::call<km::unpack<km::front<km::identity>>, to_parent>;
            [this]<typename... Ts>(Context& con, km::list<Ts...>)
            { (Ts::enter_state(con),...); }(con, back::get_parents<states, parent::id, Dest::id>());
        }
        return true;
    }

    template <typename Dest, typename Trans, typename Source>
    inline bool execute_transition(Context& con, dispatch_result& result, Dest dest, Trans trans, Source source)
    {
        namespace km = kvasir::mpl;
        using enum hsm::back::transition_flags;
        if constexpr (Trans::is_internal())
        {
            trans.exec(con);
            result = dispatch_result::internal;
            return true;
        }
        current_state = Dest::id;
        result        = dispatch_result::normal;  // this seems wrong.. because we can always take the initial transition??c

        if constexpr (Trans::to_history())
            [this]<size_t... Is>(Context & con, Trans t, state_id dest_id, Source source, Dest dest, std::integer_sequence<size_t, Is...>)
            {
                return ((dest_id == (Dest::id + Is) && handle_transition(con, t, source, get_state<Dest::id + Is>())) || ...);
            }
        (con, trans, current_state = history[Dest::history], source, dest, std::make_index_sequence<Dest::size + 1>{});
        else if constexpr (Traits::exit_count > 0 || Traits::enter_count > 0 || Traits::history_count > 0)
            handle_transition(con, trans, source, dest);
        else Trans::exec(con);

        if constexpr (Trans::to_final()) result = dispatch_result::to_final;
        return true;
    }

    template <typename... States>
    inline dispatch_result dispatch(Context& con, event_id id, kvasir::mpl::list<States...>)
    {
        dispatch_result ret = no_consume;
        namespace km        = kvasir::mpl;
        const auto s        = current_state;
        bool       val =
            ((s == States::id &&
              [this]<typename Source, typename... Ts>(Context& con, event_id id, dispatch_result& result, Source state, km::list<Ts...>)
              {
                  return (((Ts::event == id || Ts::event == back::any_event_id) && Ts::eval(con) &&
                           execute_transition(con, result, get_state<Ts::dest>{}, Ts{}, state)) ||
                          ...);
              }(con, id, ret, States{}, back::grab_transitions_sorted<states, States::id>{})) ||
             ...);
        return ret;
    }

    template <uint8_t Flags, size_t Id, size_t S, size_t ParentId, typename Entry, typename Exit, size_t H, typename... Ts>
    inline bool execute_initial_or_completion(Context& con, dispatch_result& ret,
                                              back::u_state<Flags, Id, S, ParentId, Entry, Exit, H, Ts...>) noexcept
    {
        using source = back::u_state<Flags, Id, S, ParentId, Entry, Exit, H, Ts...>;
        using enum hsm::back::state_flags;
        using enum hsm::back::transition_flags;
        ret = no_consume;
        if constexpr (static_cast<bool>(Flags & has_initial_transition) || static_cast<bool>(Flags & has_default_transition))
        {
            return (((Ts::is_initial() || Ts::is_completion()) && Ts::eval(con) &&
                     execute_transition(con, ret, get_state<Ts::dest>{}, Ts{}, get_state<0>{})) ||
                    ...);
        }
        return true;
    }

    template <uint8_t Flags, size_t Id, size_t S, size_t ParentId, typename Entry, typename Exit, size_t H, typename... Ts>
    inline bool execute_completion(Context& con, dispatch_result& ret,
                                   back::u_state<Flags, Id, S, ParentId, Entry, Exit, H, Ts...>) noexcept
    {
        using source = back::u_state<Flags, Id, S, ParentId, Entry, Exit, H, Ts...>;
        using enum hsm::back::state_flags;
        using enum hsm::back::transition_flags;
        ret = no_consume;
        if constexpr (static_cast<bool>(Flags & has_default_transition))
        {
            return ((Ts::is_completion() && Ts::eval(con) && execute_transition(con, ret, get_state<Ts::dest>{}, Ts{}, get_state<0>{})) ||
                    ...);
        }
        return true;
    }

    bool process_event(event_id ev, Context& con)
    {
        auto result = dispatch(con, ev, states{});
        if (result == no_consume) return false;
        if (result == internal) return true;
        do {
            if (result == normal)
                [this]<typename... Ts>(Context& con, dispatch_result& result, state_id c, kvasir::mpl::list<Ts...>) {
                    auto find_state = ((c == Ts::id && (execute_initial_or_completion(con, result, Ts{}))) || ...);
                }(con, result, current_state, states{});
            else if (result == to_final)
                [this]<typename... Ts>(Context& con, dispatch_result& result, state_id c, kvasir::mpl::list<Ts...>) {
                    auto find_state = ((c == Ts::id && (execute_completion(con, result, Ts{}))) || ...);
                }(con, result, current_state, states{});
        } while (result != no_consume);
        return true;
    }

    template <typename EventId>
        requires requires { back::get_event_id<sm, std::decay_t<EventId>>::value; } bool
    process_event(EventId&& ev, Context& con)
    {
        return process_event(back::get_event_id<sm, std::decay_t<EventId>>::value, con);
    }

    inline void start(Context& con) noexcept
    {
        namespace km     = kvasir::mpl;
        using root_state = get_state<0>;
        if constexpr (Traits::enter_count > 0 && static_cast<uint8_t>(root_state::flags & hsm::back::state_flags::has_entry) != 0)
        {
            typename root_state::entry{}(con);
        }

        dispatch_result result = no_consume;
        this->template execute_initial_or_completion(con, result, root_state{});
        do {
            if (result == normal)
                [this]<typename... Ts>(Context& con, dispatch_result& result, state_id c, kvasir::mpl::list<Ts...>) {
                    auto find_state = ((c == Ts::id && (execute_initial_or_completion(con, result, Ts{}))) || ...);
                }(con, result, current_state, states{});
            else if (result == to_final)
                [this]<typename... Ts>(Context& con, dispatch_result& result, state_id c, kvasir::mpl::list<Ts...>) {
                    auto find_state = ((c == Ts::id && (execute_completion(con, result, Ts{}))) || ...);
                }(con, result, current_state, states{});
        } while (result != no_consume);
    }

    inline constexpr state_id current_state_id() const noexcept { return current_state; }
    template <typename Key>
    constexpr state_id get_state_id(Key const&) const
    {
        return static_cast<state_id>(tiny_tuple::value_type<back::unpack<Key>, FinalSM>::type::id);
    }
};

template <typename Context, typename... Ts>
inline constexpr auto create_unrolled_sm(Ts&&...) noexcept
{
    namespace km           = kvasir::mpl;
    namespace tt           = tiny_tuple;
    using input_expression = hsm::state<root_state, std::decay_t<Ts>...>;
    using sm_stats         = km::call<  //
        km::push_front<         //
            hsm::back::u_assembly_status<
                tt::map<tt::detail::item<no_event, km::uint_<back::any_event_id>>,  //
                        tt::detail::item<back::history_table, std::integer_sequence<size_t>>,
                        tt::detail::item<back::entry_count, km::uint_<0>>, tt::detail::item<back::exit_count, km::uint_<0>>>,
                0, 0, 1>,
            km::fold_left<hsm::back::u_assemble_state_machine>>,
        input_expression>;
    using sm               = typename sm_stats::type;

    using sm_res =                                                     //
        typename km::call<                                             //
            km::push_front<                                            //
                hsm::back::u_attach_transition_state<sm, root_state>,  //
                km::fold_left<hsm::back::u_attach_transitions>>,
            input_expression>;
    using final_sm = typename sm_res::type;

    using state_id_type   = get_id_type<sm_stats::count>;
    using event_id_type   = get_id_type<sm_stats::event_count>;
    using history         = typename tiny_tuple::value_type<back::history_table, typename sm_res::type>::type::value;
    using entry_c         = typename tiny_tuple::value_type<back::entry_count, typename sm_res::type>::type::value;
    using exit_c          = typename tiny_tuple::value_type<back::exit_count, typename sm_res::type>::type::value;
    using history_id_type = get_id_type<history::size()>;
    using traits = back::u_sm_traits<sm_stats::count, state_id_type, sm_stats::event_count, event_id_type, history::size(), history_id_type,
                                     entry_c::value, exit_c::value>;
    return unrolled_sm<final_sm, Context, traits>(back::get_history<state_id_type>(history{}));
}
}  // namespace hsm
#endif
