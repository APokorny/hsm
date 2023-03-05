# HSM - Hierarchical statemachine library

A library for defining hierarchical state machines within C++. This library is heavily inspired by the frontend of boost-experimental.sml

## Example

```C++
#include <hsm/hsm.hpp>

...
    struct my_context
    {
        int  stuff;
        void update(){};
        void do_this_when_done(){};
    };
    my_context con;

    using namespace hsm;
    auto sm = hsm::create_state_machine<my_context>(  //
        hsm::initial = "compute"_state,
        "compute"_state(
            hsm::enter                                                          = [](my_context& self) { std::cout << "computing\n"; },
            "update_received"_ev / [](my_context& self) { self.update(); }      = hsm::internal,
            "completed"_ev / [](my_context& self) { self.do_this_when_done(); } = "done"_state,   //
            hsm::any                                                            = "error"_state,  //
            hsm::exit = [](my_context& self) { std::cout << "done computing\n"; }                 //
            ),                                                                                    //
        "error"_state,                                                                            //
        "done"_state                                                                              //
    );
    sm.start(con);
    sm.process_event("update_received"_ev, con);
    sm.process_event("update_received"_ev, con);
    sm.process_event("completed"_ev, con);

```

## Key differences to SML

* Deep hierarchies can be defined within a single expression
* States are globally unique so transitions to sub-substates are possible.
* Table based state machine algorithm - the library creates four constant global arrays containing the states, transitions, conditions and actions. The parts of the algorithm to identical symbol elimation, as performed by mold.
* Later versions of this library should also contain the ability to attach active objects to compound states (see missing features)
* Limitation: Actions and conditions cannot store their own state directly or hold references to other objects - this simplifies the state machine construction and heavily reduces the RAM and ROM footprint of state machine instances - The environment of the state machine or additional state can still be accessed through a user defined context object.
```C++
struct SC
{
   int counter{0};
};

#include <hsm/hsm.hpp>
auto sm = hsm::create_state_machine<SC>( "foo"_state( hsm::enter = [](SC& c) { ++c.counter; } ));
SC context;
sm.start(context);
```

## Recently added features:

* History support for shallow and deep histories
* Automatic Opt-out for Enter/Exit and History tracking - disable expensive state machine algorithm parts when not used in the state machine. No enter actions makes a huge performance difference, not having history or exit also makes the state machine a bit faster. This is achieved now by using if constexpr in respective parts of the state machine interpreter.
* Compile-time unrolling of the hierarchical state machine as part of the state machine algorithm - trading ROM for performance. All you need to change is switching the call of `hsm::create_state_machine` to `hsm::create_unrolled_sm`

## Upcoming Features:
* Compile-time expansion of hierarchical state machine definitions into conventional mealy/more automata
  
## Missing features:

* No active state object support yet - The idea is to add the concept of special objects that exist within the state machine hierarchy and may track external state and get triggers on enter/on exit/on history restore - allowing the object to influence the state machine. I.e. this can be used to implement timers
* support for orthogonal states
* Alternative event transition search dispatch mechanisms - currently only table based
* Proper documentation - for now please refer to the tests in the examples directory

## Use Clang Format!

Use // to break expression onto multiple lines and let clang-format align operators to keep your state machine definition readable.
```
AlignConsecutiveAssignments: true
AlignOperands: true
```

## License

Boost Software License.

## Dependencies

* As of 0.2.0 C++20 is needed - c++17 for all previous releases
* Kvasir MPL
* C++ type traits and utility header
* tiny-tuple: github.com/APokorny/tiny-tuple
