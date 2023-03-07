/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef HSM_HSM_HPP_INCLUDED
#define HSM_HSM_HPP_INCLUDED
#include <array>
#include <vector>
#include <functional>
#include <algorithm>
#include <hsm/front.hpp>
#include <hsm/back.hpp>

#include <kvasir/mpl/algorithm/filter.hpp>
#include <kvasir/mpl/sequence/join.hpp>
#include <kvasir/mpl/sequence/make_sequence.hpp>
namespace hsm
{
template <typename Hsm, typename Context, typename Traits>
struct state_machine
{
    using raw_state_machine = Hsm;
    using state_id          = typename Traits::state_id;
    using event_id          = typename Traits::event_id;
    using condition_id      = typename Traits::condition_id;
    using action_id         = typename Traits::action_id;
    using history_id        = typename Traits::history_id;
    using transition_offset = typename Traits::transition_offset;
    using tt_entry          = typename Traits::tt_entry;
    using state_entry       = typename Traits::state_entry;

    using transition_range = hsm::detail::tt_entry_range<tt_entry const>;
    using condition_entry  = bool (*)(Context&);
    using action_entry     = void (*)(Context&);
    tt_entry const*        transition_table;
    state_entry const*     state_table;
    condition_entry const* conditions;
    action_entry const*    actions;
    state_id               current_state{0};

    std::array<state_id, Traits::history_count> history;

    constexpr state_machine(tt_entry const* ts, state_entry const* states, condition_entry const* cons, action_entry const* as,
                            std::array<state_id, Traits::history_count>&& a)
        : transition_table(ts), state_table(states), conditions(cons), actions(as), history{a}
    {
    }

   private:
    constexpr transition_range get_special_transitions(state_entry const& s) const
    {
        auto const* first = &transition_table[s.transition_table_offset];
        return transition_range{first, first + s.special_transition_count};
    }
    constexpr transition_range get_special_transitions(state_id id) const { return get_special_transitions(state_table[id]); }

    constexpr transition_range get_event_transitions(state_entry const& s) const
    {
        auto const* first = &transition_table[s.transition_table_offset];
        return transition_range{first + s.special_transition_count, first + s.transition_count};
    }
    constexpr transition_range get_event_transitions(state_id id) const { return get_event_transitions(state_table[id]); }

    void execute_enter_action(state_id id, Context& con)
    {
        auto const& compound = state_table[id];
        if ((compound.flags & back::state_flags::has_entry) == back::state_flags::has_entry) actions[compound.enter_action](con);
    }
    constexpr bool contains(state_id p, state_id test) const { return p <= test && test <= p + state_table[p].children_count; }

    void handle_transition(tt_entry const& trans, Context& con)
    {
        state_entry const* search_state = &state_table[current_state];

        if (current_state == trans.dest)
        {
            if constexpr (Traits::exit_count > 0)
            {
                if (search_state->has_exit()) actions[search_state->exit_action](con);
            }
            if (trans.has_action()) actions[trans.action_index](con);
            if constexpr (Traits::enter_count > 0)
            {
                if (search_state->has_entry()) actions[search_state->enter_action](con);
            }
            return;
        }

        if constexpr (Traits::enter_count > 0 || Traits::exit_count > 0 || Traits::history_count > 0)
        {
            auto previous_state = current_state;
            while (!((search_state - state_table) <= trans.dest &&  //
                     (search_state - state_table + search_state->children_count) >= trans.dest))
            {
                if constexpr (Traits::history_count > 0)
                {
                    if (search_state->has_deep_history())
                        store_history(search_state->history, current_state);
                    else if (search_state->has_history())
                        store_history(search_state->history, previous_state);
                }
                if constexpr (Traits::exit_count > 0)
                    if (search_state->has_exit()) actions[search_state->exit_action](con);
                if constexpr (Traits::history_count > 0) previous_state = search_state - state_table;
                search_state = &state_table[search_state->parent];
            }
        }

        if (trans.has_action()) actions[trans.action_index](con);

        // constexpr if (necessary state stack is not empty)
        if constexpr (Traits::enter_count > 0)
        {
            std::vector<action_id> enter_actions;
            for (state_entry const* state_to_enter_rec = &state_table[trans.dest]; state_to_enter_rec != search_state;
                 state_to_enter_rec                    = &state_table[state_to_enter_rec->parent])
                if (state_to_enter_rec->has_entry()) enter_actions.push_back(state_to_enter_rec->enter_action);
            std::for_each(enter_actions.rbegin(), enter_actions.rend(), [this, &con](auto a) { actions[a](con); });
        }
        current_state = trans.dest;
    }
    void            store_history(history_id id, state_id stored_history) { history[id] = stored_history; }
    state_id        restore_history(state_id id) { return history[state_table[id].history]; }
    tt_entry const* completion(state_id id, Context& con)
    {
        auto const&     compound        = state_table[id];
        tt_entry const* cont_transition = nullptr;
        if (compound.has_default())
        {
            for (auto const& ttentry : get_special_transitions(compound))
                if (ttentry.transition_type() == back::transition_flags::completion &&
                    (!ttentry.has_condition() || conditions[ttentry.condition_index](con)))
                {
                    cont_transition = &ttentry;
                    break;
                }
        }
        return cont_transition;
    }

    tt_entry const* initial_or_completion(state_id id, Context& con)
    {
        auto const&     compound        = state_table[id];
        tt_entry const* cont_transition = nullptr;
        if (compound.has_initial())
        {
            // TODO  this needs to be adapted to support orthogonal regions..
            for (auto const& ttentry : get_special_transitions(compound))
                if (ttentry.transition_type() == back::transition_flags::initial &&
                    (!ttentry.has_condition() || conditions[ttentry.condition_index](con)))
                {
                    cont_transition = &ttentry;
                    break;
                }
        }
        if (!cont_transition && compound.has_default())
        {
            for (auto const& ttentry : get_special_transitions(compound))
                if (ttentry.transition_type() == back::transition_flags::completion &&
                    (!ttentry.has_condition() || conditions[ttentry.condition_index](con)))
                {
                    cont_transition = &ttentry;
                    break;
                }
        }
        return cont_transition;
    }

    constexpr state_id parent_state(state_id state) const { return state_table[state].parent; }

    void process_transition_to_compound(tt_entry const* trans, Context& con)
    {
        while (trans)
        {
            if constexpr (Traits::history_count > 0)
            {
                tt_entry entry;
                if (trans->to_shallow_history() || trans->to_deep_history())
                {
                    (entry = *trans).dest = restore_history(trans->dest);
                    trans                 = &entry;
                }
                handle_transition(*trans, con);  // handle exit  and action enter cascade
            }
            else
            {
                handle_transition(*trans, con);  // handle exit  and action enter cascade
            }

            if (trans->to_final())
                trans = completion(current_state, con);
            else
                trans = initial_or_completion(current_state, con);
        }
    }

   public:
    void start(Context& con)
    {
        if constexpr (Traits::enter_count > 0) { execute_enter_action(state_id{0}, con); }
        auto trans = initial_or_completion(state_id{0}, con);
        process_transition_to_compound(trans, con);
    }

    bool process_event(event_id event, Context& con)
    {
        tt_entry const* trans = nullptr;
        for (state_id search_state = current_state; trans == nullptr; search_state = parent_state(search_state))
        {
            for (auto const& transition : get_event_transitions(search_state))
                if ((transition.event == event || transition.event == back::any_event_id) &&
                    (!transition.has_condition() || conditions[transition.condition_index](con)))
                {
                    trans = &transition;
                    break;
                }
            if (search_state == parent_state(search_state)) break;
        }

        if (!trans) return false;
        if (trans->transition_type() == back::transition_flags::internal)
        {
            if (trans->has_action()) actions[trans->action_index](con);
            return true;
        }

        process_transition_to_compound(trans, con);
        return true;
    }

    template <typename EventId>
    requires requires { back::get_event_id<Hsm, std::decay_t<EventId>>::value; }
    bool process_event(EventId&&, Context& con)
    {
        return process_event(back::get_event_id<Hsm, std::decay_t<EventId>>::value, con);
    }
    constexpr state_id current_state_id() const { return current_state; }
    template <typename Key>
    constexpr state_id get_state_id(Key const&) const
    {
        return static_cast<state_id>(tiny_tuple::value_type<back::unpack<Key>, Hsm>::type::id);
    }
};

template <typename Context, typename... Ts>
constexpr auto create_state_machine(Ts&&...)
{
    namespace km           = kvasir::mpl;
    namespace tt           = tiny_tuple;
    using input_expression = hsm::state<root_state, std::decay_t<Ts>...>;
    using sm_stats         = km::call<                                                                             //
        km::push_front<                                                                                    //
            hsm::back::assembly_status<tt::map<tt::detail::item<no_event, km::uint_<back::any_event_id>>,  //
                                               tt::detail::item<back::history_table, std::integer_sequence<size_t>>>,
                                       0, 0, 1, 0, 0, 0>,
            km::fold_left<hsm::back::assemble_state_machine>>,
        input_expression>;
    using sm               = typename sm_stats::type;

    using sm_res =                                                   //
        typename km::call<                                           //
            km::push_front<                                          //
                hsm::back::attach_transition_state<sm, root_state>,  //
                km::fold_left<hsm::back::attach_transitions>>,
            input_expression>;
    using final_sm = typename sm_res::type;

    using state_id_type     = get_id_type<sm_stats::count>;
    using action_id_type    = get_id_type<sm_res::action_count>;
    using condition_id_type = get_id_type<sm_res::condition_count>;
    using event_id_type     = get_id_type<sm_stats::event_count>;
    using history_table           = typename tiny_tuple::value_type<back::history_table, typename sm_res::type>::type::value;
    using history_id_type   = get_id_type<history_table::size()>;
    using tt_entry          = detail::tt_entry<event_id_type, state_id_type, condition_id_type, action_id_type>;
    using traits            = back::sm_traits<sm_stats::count, state_id_type, sm_stats::event_count, event_id_type, sm_res::action_count,
                                   action_id_type, sm_res::condition_count, condition_id_type, sm_stats::transition_count,
                                   get_id_type<sm_stats::transition_count * sizeof(tt_entry)>, sm_stats::enter_count, sm_stats::exit_count,
                                   history_table::size(), history_id_type>;
    using sm_type           = state_machine<final_sm, Context, traits>;
    using state_entry       = typename sm_type::state_entry;
    using states            = back::extract_backend_states<final_sm>;
    using transitions       = back::extract_and_sort_transitions<states>;
    using conditions        = back::extract_conditions<input_expression>;
    using actions           = back::extract_actions<input_expression>;

    return sm_type(back::get_transition_table<tt_entry>(transitions{}), back::get_state_table<state_entry>(states{}),
                   back::get_conditions<Context>(conditions{}), back::get_actions<Context>(actions{}),
                   back::get_history<state_id_type>(history_table{}));
}

template <typename SM, typename E>
auto process_event(SM& sm, E const& e)
{
    sm.process_event(e);
}

}  // namespace hsm

#endif
