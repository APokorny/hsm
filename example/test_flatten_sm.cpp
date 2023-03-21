#include <hsm/unroll_sm.hpp>
#include <hsm/hsm.hpp>
#include <iostream>

int example()
{
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
    return 0;
}

int main()
{
    struct empty
    {
    };
    auto always_false = [](empty& e) -> bool { return false; };
    using namespace hsm;
    auto  a = hsm::create_unrolled_sm<empty>(         //
        "aussen"_state(                              //
            "innen"_state("ev1"_ev = root),          //
            "ev1"_ev[always_false] = "innen"_state,  //
            any                    = root            //
            ),                                       //
        "ev1"_ev   = "aussen"_state,                 //
        hsm::enter = [](empty& e) {});
    empty context;
    a.start(context);
    a.process_event("ev1"_ev, context);
    a.process_event(static_cast<std::decay_t<decltype(a)>::event_id>(1), context);
    example();
}
