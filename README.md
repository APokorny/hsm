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
* Table based state machine algorithm - optimized for small table sizes - hopefully resulting into small binary sizes
* Later versions of this library should also contain the ability to attach active objects to compound states (see missing features)

## Missing features:

* History tracking and restore is not yet implemented: There are two reasonable ways to handle deep and shallow histories, by updateing on enter or on exit of states. Those two models have behavior implications - so it should be implemented via a policy
* Optimize state machine creation code : analyze the binary created with enabled optimizations and look for alternative code structures to fill the arrays
* make enter tracking optional / this is currently the only source for dynamic memory -> it could be improved to use static memory and or disable if there is no enter action at all
* No active state object support yet - The idea is to add the concept of special objects that exist within the state machine hierachy and may track external state and get triggers on enter/on exit/on histry restore - allowing the object to influence the state machine. I.e. this can be used to implement timers
* support for orthogonal states
* alternative event transitiion search dispatch mechanisms - currently only table based

## License

Boost Software License.

## Dependencies

* Kvasir MPL
* C++ type traits and utility header
* tiny-tuple: github.com/APokorny/tiny-tuple
