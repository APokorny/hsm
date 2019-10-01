/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#ifndef HSM_HSM_HPP_INCLUDED
#define HSM_HSM_HPP_INCLUDED
#include <array>
#include <functional>
#include "front.hpp"
#include "back.hpp"

namespace hsm
{
template <typename Hsm, typename Traits>
struct state_machine
{
    using raw_state_machine = Hsm;
    using state_id          = Traits::state_id;
    using event_id          = Traits::event_id;
    using condition_id      = Traits::condition_id;
    using action_id         = Traits::action_id;
    using transition_offset = Traits::transition_offset;
    using tt_entry          = Traits::tt_entry;
    using state_entry       = Traits::state_entry;

    using transition_range = hsm::detail::tt_entry_range<tt_entry const>;
    using transition_array = std::array<tt_entry, Traits::transition_count>;
    using state_array      = std::array<state_entry, Traits::count>;
    using condition_array  = std::array<std::function<bool()>, Traits::condition_count>;
    using action_array     = std::array<std::function<void()>, Traits::action_count>;
    const transition_array transition_table;
    const state_array      state_table;
    const condition_array  condition_table;
    const action_array     action_table;
    state_id               current_state{0};

    constexpr state_machine(transition_array&& ts, state_array&& states, condition_array&& conditions, action_array&& actions)
        : transition_table(std::move(ts)),
          state_table(std::move(states)),
          condition_table(std::move(conditions)),
          action_table(std::move(actions))
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

    void execute_enter_action(state_id id)
    {
        auto const& compound = state_table[id];
        if ((compound.flags & back::state_flags::has_entry) == back::state_flags::has_entry) action_table[compound.enter_action]();
    }
    constexpr bool contains(state_id p, state_id test) const { return p <= test && test <= p + state_table[p].children_count; }

    void handle_transition(tt_entry const& trans)
    {
        state_entry const* search_state = &state_table[current_state];
        auto               contains     = [trans, &search_state, this]() {
            return (search_state - state_table.data()) <= trans.dest &&
                   (search_state - state_table.data() + search_state->children_count) >= trans.dest;
        };
        auto execute_exit = [this](state_entry const* s) {
            // store state in history if nexesssary .. current_state or recently exited state
            if (s->has_exit()) action_table[s->exit_action]();
        };
        auto execute_enter = [this](state_entry const* s) {
            // alternatively update the history on upwards --- this ought to be a policy
            if (s->has_entry()) action_table[s->enter_action]();
        };
        auto to_parent = [this](state_entry const*& s) { s = &state_table[s->parent]; };

        // constexpr if history || enter actions
        if (current_state == trans.dest)
        {
            execute_exit(search_state);
            if (trans.has_action()) action_table[trans.action_index]();
            execute_enter(search_state);
            return;
        }

        while (!contains())
        {
            execute_exit(search_state);
            to_parent(search_state);
        }

        if (trans.has_action()) action_table[trans.action_index]();

        // constexpr if (necessary state stack is not empty)
        std::vector<action_id> actions;
        for (state_entry const* state_to_enter_rec = &state_table[trans.dest]; state_to_enter_rec != search_state;
             to_parent(state_to_enter_rec))
        {
            if (state_to_enter_rec->has_entry()) actions.push_back(state_to_enter_rec->enter_action);
        }

        std::for_each(actions.rbegin(), actions.rend(), [this](auto a) { action_table[a](); });
        current_state = trans.dest;
    }
    tt_entry const* restore_history(state_id id)
    {
        // if history is set for id
        //    create transition to history (no cond no action) return ptr;
        // else return history restore transition
        return nullptr;
    }
    tt_entry const* completion(state_id id)
    {
        auto const&     compound        = state_table[id];
        tt_entry const* cont_transition = nullptr;
        if (compound.has_default())
        {
            for (auto const& ttentry : get_special_transitions(compound))
                if (ttentry.transition_type() == back::transition_flags::completion &&
                    (!ttentry.has_condition() || condition_table[ttentry.condition_index]()))
                {
                    cont_transition = &ttentry;
                    break;
                }
        }
        return cont_transition;
    }

    tt_entry const* initial_or_completion(state_id id)
    {
        auto const&     compound        = state_table[id];
        tt_entry const* cont_transition = nullptr;
        if (compound.has_initial())
        {
            // TODO  this needs to be adapted to support orthogonal regions..
            for (auto const& ttentry : get_special_transitions(compound))
                if (ttentry.transition_type() == back::transition_flags::initial &&
                    (!ttentry.has_condition() || condition_table[ttentry.condition_index]()))
                {
                    cont_transition = &ttentry;
                    break;
                }
        }
        if (!cont_transition && compound.has_default())
        {
            for (auto const& ttentry : get_special_transitions(compound))
                if (ttentry.transition_type() == back::transition_flags::completion &&
                    (!ttentry.has_condition() || condition_table[ttentry.condition_index]()))
                {
                    cont_transition = &ttentry;
                    break;
                }
        }
        return cont_transition;
    }

    constexpr state_id parent_state(state_id state) const { return state_table[state].parent; }

    void process_transition_to_compound(tt_entry const* trans)
    {
        while (trans)
        {
            handle_transition(*trans);  // handle exit  and action enter cascade
            if (trans->to_history())
                trans = restore_history(current_state);
            else if (trans->to_final())
                trans = completion(current_state);
            else
                trans = initial_or_completion(current_state);
        }
    }

   public:
    void start()
    {
        execute_enter_action(state_id{0});
        auto trans = initial_or_completion(state_id{0});
        process_transition_to_compound(trans);
    }

    bool process_event(event_id event)
    {
        tt_entry const* trans = nullptr;
        for (state_id search_state = current_state; trans == nullptr; search_state = parent_state(search_state))
        {
            for (auto const& transition : get_event_transitions(search_state))
                if ((transition.event == event || transition.event == back::any_event_id) &&
                    (!transition.has_condition() || condition_table[transition.condition_index]()))
                {
                    trans = &transition;
                    break;
                }
            if (search_state == parent_state(search_state)) break;
        }

        if (!trans) return false;
        if (trans->transition_type() == back::transition_flags::internal)
        {
            if (trans->has_action()) action_table[trans->action_index]();
            return true;
        }

        process_transition_to_compound(trans);
        return true;
    }

    template <typename Key>
    bool process_event(Key const& key)
    {
        using event_type = tiny_tuple::value_type<back::unpack<Key>, Hsm>::type;
        return process_event(static_cast<event_id>(event_type::value));
    }
    constexpr state_id current_state_id() const { return current_state; }
    template <typename Key>
    constexpr state_id get_state_id(Key const& key) const
    {
        return static_cast<state_id>(tiny_tuple::value_type<back::unpack<Key>, Hsm>::type::id);
    }
};

template <typename... Ts>
constexpr auto create_state_machine(Ts&&... state_parts)
{
    auto RootState  = back::enumerate_state_elements(back::counters<0, 0>{}, hsm::state<hsm::root_state>{}, std::move(state_parts)...);
    namespace km    = kvasir::mpl;
    using ca        = std::decay_t<decltype(tiny_tuple::get<0>(RootState))>;
    using rootstate = std::decay_t<decltype(tiny_tuple::get<1>(RootState))>;
    using sm_stats  = km::call<  //
        km::unpack<             //
            km::push_front<     //
                hsm::back::assembly_status<tiny_tuple::map<tiny_tuple::detail::item<no_event, km::uint_<back::any_event_id>>>, 0, 1, 1>,
                km::fold_left<hsm::back::assemble_state_machine>>>,
        rootstate>;
    using sm = km::call<km::unpack<km::push_front<tiny_tuple::detail::item<root_state, hsm::back::state<0, 0, sm_stats::count, 0, 0, 0>>,
                                                  km::cfe<tiny_tuple::map>>>,
                        typename sm_stats::type>;
    using final_sm =                                                     //
        typename km::call<                                               //
            km::unpack<                                                  //
                km::push_front<                                          //
                    hsm::back::attach_transition_state<sm, root_state>,  //
                    km::fold_left<hsm::back::attach_transitions>>>,
            rootstate>::type;

    using state_id_type     = get_id_type<sm_stats::count>;
    using action_id_type    = get_id_type<ca::a_counter>;
    using condition_id_type = get_id_type<ca::c_counter>;
    using event_id_type     = get_id_type<sm_stats::event_count>;
    using tt_entry          = detail::tt_entry<event_id_type, state_id_type, condition_id_type, action_id_type>;
    using traits =
        back::sm_traits<sm_stats::count, state_id_type, sm_stats::event_count, event_id_type, ca::a_counter, action_id_type, ca::c_counter,
                        condition_id_type, sm_stats::transition_count, get_id_type<sm_stats::transition_count * sizeof(tt_entry)>>;
    using sm_type          = state_machine<final_sm, traits>;
    using transition_array = sm_type::transition_array;
    using state_array      = sm_type::state_array;
    using condition_array  = sm_type::condition_array;
    using action_array     = sm_type::action_array;

    condition_array conditions;
    action_array    actions;
    back::initialize_ca_array(std::move(tiny_tuple::get<1>(RootState)), conditions, actions);

    transition_array transitions;
    state_array      states;
    back::initialize_states<final_sm>(transitions, states);

    return sm_type(std::move(transitions), std::move(states), std::move(conditions), std::move(actions));
}

template <typename SM, typename E>
auto process_event(SM& sm, E const& e)
{
    sm.process_event(e);
}

}  // namespace hsm

#endif
