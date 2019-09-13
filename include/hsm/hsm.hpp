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
    using state_id          = Traits::state_id;
    using event_id          = Traits::event_id;
    using condition_id      = Traits::condition_id;
    using action_id         = Traits::action_id;
    using transition_offset = Traits::transition_offset;
    using tt_entry          = Traits::tt_entry;
    using state_entry       = Traits::state_entry;

    using transition_array = std::array<tt_entry, Traits::transition_count>;
    using state_array      = std::array<state_entry, Traits::count>;
    using condition_array  = std::array<std::function<bool()>, Traits::condition_count>;
    using action_array     = std::array<std::function<void()>, Traits::action_count>;
    const transition_array transition_table;
    const state_array      state_table;
    const condition_array  condition_table;
    const action_array     action_table;

    state_machine(transition_array&& ts, state_array&& states, condition_array&& conditions, action_array&& actions)
        : transition_table(std::move(ts)),
          state_table(std::move(states)),
          condition_table(std::move(conditions)),
          action_table(std::move(actions))
    {
    }
    void process_event(event_id event);
};

template <typename... Ts>
constexpr auto create_state_machine(Ts&&... state_parts)
{
    auto RootState = back::enumerate_state_elements(back::counters<0, 0>{}, hsm::state<hsm::root_state>{}, std::move(state_parts)...);
    namespace km   = kvasir::mpl;
    using ca       = std::decay_t<decltype(tiny_tuple::get<0>(RootState))>;
    using sm_stats = km::call<                                                                                             //
        km::push_front<                                                                                                    //
            hsm::back::assembly_status<tiny_tuple::map<tiny_tuple::detail::item<no_event, km::uint_<0>, true>>, 0, 1, 1>,  //
            km::fold_left<hsm::back::assemble_state_machine>>,
        Ts...>;
    using sm       = km::call<km::unpack<km::push_front<tiny_tuple::detail::item<root_state, hsm::back::state<0, 0, sm_stats::count, 0>>,
                                                  km::cfe<tiny_tuple::map>>>,
                        typename sm_stats::type>;
    using final_sm =                                                 //
        typename km::call<                                           //
            km::push_front<                                          //
                hsm::back::attach_transition_state<sm, root_state>,  //
                km::fold_left<hsm::back::attach_transitions>>,
            Ts...>::type;

    // std::cout << final_sm{} << std::endl;

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
