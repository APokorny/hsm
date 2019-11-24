# HSM - Hierarchical statemachine library

A library for defining hierachical state machines within C++. This library is heavily inspired by the frontend of boost-experimental.sml

## Example

```C++
#include <hsm/hsm.hpp>
auto sm = hsm::create_state_machine(...watch this place..);
```

## Key differences to SML

* Deep hierarchies can be defined within a single expression
* States are globally unique so transitions to sub-substates are possible.
* Table based state machine algorithm - the library creates four constant global arrays containing the states, transitions, conditions and actions.
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
sm.process_event(context);
```

## Missing features:

* History tracking and restore is not yet implemented: There are two reasonable ways to handle deep and shallow histories, by updateing on enter or on exit of states. Those two models have behavior implications - so it should be implemented via a policy
* make enter tracking optional / this is currently the only source for dynamic memory -> it could be improved to use static memory and or disable if there is no enter action at all
* No active state object support yet - The idea is to add the concept of special objects that exist within the state machine hierachy and may track external state and get triggers on enter/on exit/on histry restore - allowing the object to influence the state machine. I.e. this can be used to implement timers
* support for orthogonal states
* alternative event transitiion search dispatch mechanisms - currently only table based

## Use Clang Format!

Use // to break expression onto multiple lines and let clang-format align operators to keep your state machine definition readable.
```
AlignConsecutiveAssignments: true
AlignOperands: true
```

## License

Boost Software License.

## Dependencies

* Kvasir MPL
* C++ type traits and utility header
* tiny-tuple: github.com/APokorny/tiny-tuple
