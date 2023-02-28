#include <hsm/unroll_sm.hpp>
#include <hsm/hsm.hpp>

int main()
{
    struct empty
    {
    };
    using namespace hsm;
    auto a       = hsm::create_unrolled_sm<empty>(  //
        "aussen"_state(                       //
            "innen"_state("ev1"_ev = root),   //
            "ev1"_ev = "innen"_state,         //
            any      = root                   //
            ),                                //
        "ev1"_ev   = "aussen"_state,          //
        hsm::enter = [](empty &e) {});
    empty context;
    a.start(context);
    a.process_event("ev1"_ev, context);
    a.process_event(static_cast<std::decay_t<decltype(a)>::event_id>(1), context);
}
