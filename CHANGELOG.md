
# Changelog

# [Unreleased]
In this version hsm comes with a few more bug fixes and faster state machine execution.
So if you have no need for enter or exit actions the execution is now noticeable faster.
Not having enter actions skips one expensive step when entering a deep hierachy.
Not having both enter and exit actions skips the parent state entirely.

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
