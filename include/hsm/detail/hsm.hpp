#ifndef HSM_DETAIL_HSM_HPP_INCLUDED
#define HSM_DETAIL_HSM_HPP_INCLUDED

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

template <typename StateId, typename TTOffset, typename ActionId>
struct state_entry
{
    using state_id = StateId;
    using transition_table_offset_type = TTOffset;
    TTOffset transition_table_offset{0};
    state_id parent{0};
    action_id enter_action;
    action_id exit_action;
    uint16_t transition_count{0};
    uint8_t  flags{0};
};

template <typename EventId, typename StateId, typename ConditionId, typename ActionId>
struct tt_entry
{
    using event_id = EventId;
    using state_id = StateId;
    using condition_id = ConditionId;
    using action_id = ActionId;
    EventId     event{0};
    StateId     dest{0};
    ConditionId condition_index{0};
    ActionId    action_index{0};
    uint8_t     flags{0};
};

}  // namespace detail

template <size_t Count>
using get_id_type = detail::get_id_type_impl<Count>::type;

}  // namespace hsm

#endif
