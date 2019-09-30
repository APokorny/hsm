#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include <iostream>
#include "catch.hpp"
#include <hsm/hsm.hpp>

using hsm::literals::operator""_ev;
using hsm::literals::operator""_state;

constexpr auto sm_with_multiple_transitions()
{
    return hsm::create_state_machine(  //
        "e1"_ev,                       //
        "a"_state,                     //
        "b"_state,                     //
        "e1"_ev = "a"_state,           //
        "e1"_ev = "b"_state            //
    );
}

TEST_CASE("Pick first matching transition", "[algorithm][transition_search]")
{
    auto sm = sm_with_multiple_transitions();
    sm.start();
    sm.process_event("e1"_ev);
    REQUIRE(sm.current_state_id() == sm.get_state_id("a"_state));
}

constexpr auto sm_with_any()
{
    return hsm::create_state_machine(  //
        "e1"_ev, "e2"_ev, "e3"_ev,     //
        "a"_state,                     //
        "b"_state,                     //
        "e1"_ev  = "a"_state,          //
        hsm::any = "b"_state           //
    );
}
constexpr auto sm_with_any_internal()
{
    return hsm::create_state_machine(         //
        "e1"_ev, "e2"_ev, "e3"_ev,            //
        hsm::initial = "a"_state,             //
        "a"_state(hsm::any = hsm::internal),  //
        "b"_state,                            //
        hsm::any = "b"_state                  //
    );
}
TEST_CASE("Handle Any event at normal transition", "[algorithm][any][normal]")
{
    auto sm = sm_with_any();
    sm.start();
    sm.process_event("e3"_ev);
    REQUIRE(sm.current_state_id() == sm.get_state_id("b"_state));
}
TEST_CASE("Prefer specific transition over any event transition", "[algorithm][any][normal]")
{
    auto sm = sm_with_any();
    sm.start();
    sm.process_event("e1"_ev);
    REQUIRE(sm.current_state_id() == sm.get_state_id("a"_state));
}
TEST_CASE("Handle Any event at internal transition", "[algorithm][any][internal]")
{
    auto sm = sm_with_any_internal();
    sm.start();
    REQUIRE(sm.current_state_id() == sm.get_state_id("a"_state));
    REQUIRE(sm.process_event("e3"_ev));
    REQUIRE(sm.current_state_id() == sm.get_state_id("a"_state));
    REQUIRE(sm.process_event("e1"_ev));
    REQUIRE(sm.current_state_id() == sm.get_state_id("a"_state));
    REQUIRE(sm.process_event("e2"_ev));
    REQUIRE(sm.current_state_id() == sm.get_state_id("a"_state));
}

