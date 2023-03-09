
# Changelog

## Version 0.3.3
Bugfix release for hsm::any transitions in unrolled state machine

**Bugfix**: Sort transitions in unrolled statemachine


## Version 0.3.2
Warninng related release

**Bugfix**: Additional warnings fixed

## Version 0.3.1
Minor bugfix release

**Feature**: Expanded micro benchmark
- To show the difference in performance, each state machine is now executed in the tabled and unrolled version
- Depending on the statemachine the performance increased by a factor of 500 or 40..

**Bugfix**: Fix templated process_event
- Fixes an error in the key look-up the event type.


## Version 0.3.0
This version sees the addition of a second backend whih unrolls the state machine
in place, not needing any tables besides the current history status.

**Feature**: Cleaner Frontend: Less Specializations and wrappers
- Removal of partial specialisation for transitions, replaced with concepts
- Removal of some wrappers like `action_node`
  
**Feature**: Unrolled: Second state machine backend 
- This unrolls the state machine logic, transition search and more 

**Bugfix**: Minor cleanup
- Since 0.2.0 the plain history flag at transitions is no longer needed.


## Version 0.2.0
In this version hsm comes with a few more bug fixes and faster state machine execution.
So if you have no need for enter or exit actions the execution is now noticeable faster.
Not having enter actions skips one expensive step when entering a deep hierachy.
Not having both enter and exit actions skips the parent state entirely.
Also history is now working - take a look at the unit test for usage. History
is declared by adding a history assignment to a state:
```
   "some"_state(
      hsm::deep_history = "first_entry"_state, // when no history is set history info will point to "first_entry"
      // or if the deep history is not needed:
      hsm::history = "first_entry"_state,
      // definitions like the above cannot contain conditions or actions
      )
      
   // history can be accessed via transitions, just wrap the target state with history:
   "trigger"_ev = hsm::deep_history_of("some"_state)
   
   // or in case of shallow history:
   "trigger"_ev = hsm::history_of("some"_state)
   
```

**Feature**: history support
- basic support is in place, error handling missing

**Feature**: build system
- move towards fetch content and simpler builds

**Feature**: Faster transition handling
- exit/enter cascades are disabled when state machines have no enter or exit actions
- there is now a micro benchmark to show the difference

**Bugfix**: Fix missing enter exit actions
- enter exit actions when attached to states with `_state`-literal type ids got dropped by an template matching error

## Version 0.1.0

Initial release of hsm as a support library to build a nearly allocation free event driven json parser.
With many outstanding optimizations and improvements.
