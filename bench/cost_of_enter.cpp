#define NONIUS_RUNNER
#include <nonius/nonius_single.h++>
#include <hsm/hsm.hpp>
#include <hsm/unroll_sm.hpp>
#include <ranges>
struct SC
{
    int counter{0};
};
static constexpr int blip_count = 10;

NONIUS_BENCHMARK("handle_events_with_enter",
                 []
                 {
                     using namespace hsm::literals;
                     auto sm = hsm::create_state_machine<SC>("blip"_ev,
                                                             "a"_state(
                                                                 hsm::enter = [](SC& c) {},  //
                                                                 "b"_state(
                                                                     hsm::enter = [](SC& c) {},  //
                                                                     "ping"_state(
                                                                         hsm::enter = [](SC& c) { ++c.counter; },  //
                                                                         "blip"_ev  = "pong"_state))),
                                                             "c"_state(
                                                                 hsm::enter = [](SC& c) {},  //
                                                                 "d"_state(
                                                                     hsm::enter = [](SC& c) {},  //
                                                                     "pong"_state(
                                                                         hsm::enter = [](SC& c) { ++c.counter; },  //
                                                                         "blip"_ev  = "ping"_state))),
                                                             "blip"_ev = "ping"_state);
                     SC   con;
                     sm.start(con);
                     for (int i : std::ranges::iota_view(1, blip_count)) sm.process_event("blip"_ev, con);
                 })
NONIUS_BENCHMARK("handle_events_with_action_single_enter_action",
                 []
                 {
                     using namespace hsm::literals;
                     auto sm = hsm::create_state_machine<SC>(
                         "blip"_ev,

                         hsm::enter = [](SC& c) {},
                         "a"_state("b"_state("ping"_state("blip"_ev / [](SC& c) { ++c.counter; } = "pong"_state))),
                         "c"_state("d"_state("pong"_state("blip"_ev / [](SC& c) { ++c.counter; } = "ping"_state))),
                         "blip"_ev = "ping"_state);
                     SC con;
                     sm.start(con);
                     for (int i : std::ranges::iota_view(1, blip_count)) sm.process_event("blip"_ev, con);
                 })

NONIUS_BENCHMARK("handle_events_with_action_single_exit_action",
                 []
                 {
                     using namespace hsm::literals;
                     auto sm = hsm::create_state_machine<SC>(
                         "blip"_ev,                 //
                         hsm::exit = [](SC& c) {},  //
                         "a"_state("b"_state("ping"_state("blip"_ev / [](SC& c) { ++c.counter; } = "pong"_state))),
                         "c"_state("d"_state("pong"_state("blip"_ev / [](SC& c) { ++c.counter; } = "ping"_state))),
                         "blip"_ev = "ping"_state);
                     SC con;
                     sm.start(con);
                     for (int i : std::ranges::iota_view(1, blip_count)) sm.process_event("blip"_ev, con);
                 })

NONIUS_BENCHMARK("handle_events_with_action",
                 []
                 {
                     using namespace hsm::literals;
                     auto sm = hsm::create_state_machine<SC>(
                         "blip"_ev,  //
                         "a"_state("b"_state("ping"_state("blip"_ev / [](SC& c) { ++c.counter; } = "pong"_state))),
                         "c"_state("d"_state("pong"_state("blip"_ev / [](SC& c) { ++c.counter; } = "ping"_state))),
                         "blip"_ev = "ping"_state);
                     SC con;
                     sm.start(con);
                     for (int i : std::ranges::iota_view(1, blip_count)) sm.process_event("blip"_ev, con);
                 })

NONIUS_BENCHMARK("handle_events_with_action_enter_exit_action",
                 []
                 {
                     using namespace hsm::literals;
                     auto sm = hsm::create_state_machine<SC>(
                         "blip"_ev,

                         hsm::exit = [](SC& c) {}, hsm::enter = [](SC& c) {},
                         "a"_state("b"_state("ping"_state("blip"_ev / [](SC& c) { ++c.counter; } = "pong"_state))),
                         "c"_state("d"_state("pong"_state("blip"_ev / [](SC& c) { ++c.counter; } = "ping"_state))),
                         "blip"_ev = "ping"_state);
                     SC con;
                     sm.start(con);
                     for (int i : std::ranges::iota_view(1, blip_count)) sm.process_event("blip"_ev, con);
                 })

NONIUS_BENCHMARK("handle_events_with_enter_unrolled",
                 []
                 {
                     using namespace hsm::literals;
                     auto sm = hsm::create_unrolled_sm<SC>("blip"_ev,
                                                           "a"_state(
                                                               hsm::enter = [](SC& c) {},  //
                                                               "b"_state(
                                                                   hsm::enter = [](SC& c) {},  //
                                                                   "ping"_state(
                                                                       hsm::enter = [](SC& c) { ++c.counter; },  //
                                                                       "blip"_ev  = "pong"_state))),
                                                           "c"_state(
                                                               hsm::enter = [](SC& c) {},  //
                                                               "d"_state(
                                                                   hsm::enter = [](SC& c) {},  //
                                                                   "pong"_state(
                                                                       hsm::enter = [](SC& c) { ++c.counter; },  //
                                                                       "blip"_ev  = "ping"_state))),
                                                           "blip"_ev = "ping"_state);
                     SC   con;
                     sm.start(con);
                     for (int i : std::ranges::iota_view(1, blip_count)) sm.process_event("blip"_ev, con);
                 })
NONIUS_BENCHMARK("handle_events_with_action_single_enter_action_unrolled",
                 []
                 {
                     using namespace hsm::literals;
                     auto sm = hsm::create_unrolled_sm<SC>(
                         "blip"_ev,

                         hsm::enter = [](SC& c) {},
                         "a"_state("b"_state("ping"_state("blip"_ev / [](SC& c) { ++c.counter; } = "pong"_state))),
                         "c"_state("d"_state("pong"_state("blip"_ev / [](SC& c) { ++c.counter; } = "ping"_state))),
                         "blip"_ev = "ping"_state);
                     SC con;
                     sm.start(con);
                     for (int i : std::ranges::iota_view(1, blip_count)) sm.process_event("blip"_ev, con);
                 })

NONIUS_BENCHMARK("handle_events_with_action_single_exit_action_unrolled",
                 []
                 {
                     using namespace hsm::literals;
                     auto sm = hsm::create_unrolled_sm<SC>(
                         "blip"_ev,                 //
                         hsm::exit = [](SC& c) {},  //
                         "a"_state("b"_state("ping"_state("blip"_ev / [](SC& c) { ++c.counter; } = "pong"_state))),
                         "c"_state("d"_state("pong"_state("blip"_ev / [](SC& c) { ++c.counter; } = "ping"_state))),
                         "blip"_ev = "ping"_state);
                     SC con;
                     sm.start(con);
                     for (int i : std::ranges::iota_view(1, blip_count)) sm.process_event("blip"_ev, con);
                 })

NONIUS_BENCHMARK("handle_events_with_action_unrolled",
                 []
                 {
                     using namespace hsm::literals;
                     auto sm = hsm::create_unrolled_sm<SC>(
                         "blip"_ev,  //
                         "a"_state("b"_state("ping"_state("blip"_ev / [](SC& c) { ++c.counter; } = "pong"_state))),
                         "c"_state("d"_state("pong"_state("blip"_ev / [](SC& c) { ++c.counter; } = "ping"_state))),
                         "blip"_ev = "ping"_state);
                     SC con;
                     sm.start(con);
                     for (int i : std::ranges::iota_view(1, blip_count)) sm.process_event("blip"_ev, con);
                 })

NONIUS_BENCHMARK("handle_events_with_action_enter_exit_action_unrolled",
                 []
                 {
                     using namespace hsm::literals;
                     auto sm = hsm::create_unrolled_sm<SC>(
                         "blip"_ev,

                         hsm::exit = [](SC& c) {}, hsm::enter = [](SC& c) {},
                         "a"_state("b"_state("ping"_state("blip"_ev / [](SC& c) { ++c.counter; } = "pong"_state))),
                         "c"_state("d"_state("pong"_state("blip"_ev / [](SC& c) { ++c.counter; } = "ping"_state))),
                         "blip"_ev = "ping"_state);
                     SC con;
                     sm.start(con);
                     for (int i : std::ranges::iota_view(1, blip_count)) sm.process_event("blip"_ev, con);
                 })
