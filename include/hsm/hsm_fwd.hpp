#ifndef HSM_HSM_FWD_HPP_INCLUDED
#define HSM_HSM_FWD_HPP_INCLUDED

#include "hsm/detail/flag.hpp"

namespace hsm
{
template <typename T>
struct state_ref;
template <typename T>
struct event;
struct internal_transition;
struct root_state;
struct empty_history;
template <typename T>
struct final_state;
struct current_state;
struct completion;
struct start;
struct normal;
struct no_event;
struct no_cond;
struct no_dest;
struct no_action;

template <typename A, size_t Id = 0>
struct exit_action;
template <typename A, size_t Id = 0>
struct entry_action;
template <typename C, size_t Id = 0>
struct condition_node;
template <typename A, size_t Id = 0>
struct action_node;

namespace back
{
template <uint32_t Flags, size_t Id, size_t Size, size_t ParentId, size_t Entry, size_t Exit, typename... Transitions>
struct state;

template <uint32_t Flags, size_t E, size_t D, size_t C, size_t A>
struct transition;

enum class state_flags : uint8_t
{
    none                            = 0,
    has_initial_transition          = 1,
    has_default_transition          = 2,  // == completion
    has_history                     = 4,
    has_deep_history                = 8,
    has_entry                       = 16,
    has_exit                        = 32,
    has_mulitple_initial_transition = 64,  // == orthogonal regions
};

enum class transition_flags : uint8_t
{
    initial              = 0,
    history              = 1,
    completion           = 2,
    internal             = 3,
    normal               = 4,
    transition_type_mask = 0x7,

    to_final           = 0x8,
    to_shallow_history = 0x10,
    to_deep_history    = 0x18,
    dest_mask          = 0x18,

    has_action    = 0x20,
    has_condition = 0x40,
};

template <>
struct enable_bitmask_ops<transition_flags> : kvasir::mpl::bool_<true>
{
};
template <>
struct enable_bitmask_ops<state_flags> : kvasir::mpl::bool_<true>
{
};

}  // namespace back
}  // namespace hsm

#endif
