#ifndef HSM_HSM_FWD_HPP_INCLUDED
#define HSM_HSM_FWD_HPP_INCLUDED

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
template <uint32_t Flags, size_t Id, size_t Size, size_t ParentId, typename... Transitions>
struct state;

template <typename TT, size_t E, size_t D, size_t C, size_t A>
struct transition;
}  // namespace back
}  // namespace hsm

#endif
