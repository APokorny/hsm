/* ==========================================================================
 Copyright (c) 2019 Andreas Pokorny
 Distributed under the Boost Software License, Version 1.0. (See accompanying
 file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
========================================================================== */

#pragma once

#include <type_traits>

namespace hsm
{
namespace detail
{
template <size_t Count, typename T = void>
struct get_id_type_impl
{
    using type = uint32_t;
};

template <size_t Count>
struct get_id_type_impl<Count, std::enable_if_t<(Count >= 255 && Count < 65535)>>
{
    using type = uint16_t;
};

template <size_t Count>
struct get_id_type_impl<Count, std::enable_if_t<(Count >= 0 && Count < 255)>>
{
    using type = uint8_t;
};

template <typename StateId, typename TTOffset, typename ActionId, typename HistoryId>
struct state_entry
{
    using action_id                    = ActionId;
    using state_id                     = StateId;
    using history_id                   = HistoryId;
    using transition_table_offset_type = TTOffset;
    TTOffset          transition_table_offset{0};
    action_id         enter_action{0};
    action_id         exit_action{0};
    state_id          parent{0};
    state_id          children_count{0};
    uint16_t          transition_count{0};
    uint8_t           special_transition_count{0};
    back::state_flags flags{back::state_flags::none};
    history_id        history{0};

    constexpr bool has_default() const { return is_set(flags, back::state_flags::has_default_transition); }

    constexpr bool has_initial() const { return is_set(flags, back::state_flags::has_initial_transition); }
    constexpr bool has_any_history() const { return is_set(flags, back::state_flags::has_history | back::state_flags::has_deep_history); }
    constexpr bool has_history() const { return is_set(flags, back::state_flags::has_history); }
    constexpr bool has_deep_history() const { return is_set(flags, back::state_flags::has_deep_history); }
    constexpr bool has_entry() const { return is_set(flags, back::state_flags::has_entry); }
    constexpr bool has_exit() const { return is_set(flags, back::state_flags::has_exit); }
};

template <typename EventId, typename StateId, typename ConditionId, typename ActionId>
struct tt_entry
{
    using event_id     = EventId;
    using state_id     = StateId;
    using condition_id = ConditionId;
    using action_id    = ActionId;
    EventId                event{0};
    StateId                dest{0};
    ConditionId            condition_index{0};
    ActionId               action_index{0};
    back::transition_flags flags{back::transition_flags::initial};

    constexpr back::transition_flags transition_type() const { return flags & back::transition_flags::transition_type_mask; }

    constexpr back::transition_flags destination_type() const { return flags & back::transition_flags::dest_mask; }
    constexpr bool to_shallow_history() const { return is_set(destination_type(), back::transition_flags::to_shallow_history); }
    constexpr bool to_deep_history() const { return is_set(destination_type(), back::transition_flags::to_shallow_history); }
    constexpr bool to_final() const { return is_set(destination_type(), back::transition_flags::to_final); }
    constexpr bool has_action() const { return back::transition_flags::has_action == (flags & back::transition_flags::has_action); }
    constexpr bool has_condition() const
    {
        return back::transition_flags::has_condition == (flags & back::transition_flags::has_condition);
    }
};

template <typename T>
struct tt_entry_range
{
    T*           b;
    T*           e;
    constexpr T* begin() const { return b; }
    constexpr T* end() const { return e; }
};

}  // namespace detail

template <size_t Count>
using get_id_type = typename detail::get_id_type_impl<Count>::type;

}  // namespace hsm
