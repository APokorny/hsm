# HSM - Hierarchical statemachine library

A library for defining hierachical state machines within C++. This library is heavily inspired by the frontend of boost-experimental.sml

## Key differences to SML

Deep hierarchies can be defined within a single expression. States are globally unique. Transitions to sub-substates are possible. 

## Dependencies

* Kvasir MPL
* C++ type traits and utility header
* tiny-tuple: github.com/APokorny/tiny-tuple
