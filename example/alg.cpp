#include <iostream>
#include "hsm/hsm.hpp"

bool test_guard() { return true; }
int  main()
{
    using namespace hsm;
    struct EmptyContext
    {
    };
    auto         debug_stuff = [](EmptyContext&) { std::cout << "Stay in CCC \n"; };
    auto         go_to_bbb   = [](EmptyContext&) { std::cout << "go to bbb\n"; };
    auto         foo         = create_state_machine<EmptyContext>(  //
        "jump_to_b"_ev,                             //
        "stay_in_ccc"_ev,
        initial    = "c"_state,                                                    //
        hsm::enter = [](EmptyContext&) { std::cout << "in main state\n"; },        //
        hsm::exit  = [](EmptyContext&) { std::cout << "leaving root state\n"; },   //
        "c"_state(                                                                 //
            hsm::enter = [](EmptyContext&) { std::cout << "in c state\n"; },       //
            hsm::exit  = [](EmptyContext&) { std::cout << "leaving c state\n"; },  //
            initial    = "cc"_state,
            "cc"_state(                                                                                         //
                enter     = [](EmptyContext&) { std::cout << "in cc state\n"; },                                //
                hsm::exit = [](EmptyContext&) { std::cout << "leaving cc state\n"; },                           //
                "ccc"_state(                                                                                    //
                    "stay_in_ccc"_ev / debug_stuff = internal,                                                  //
                    enter                          = [](EmptyContext&) { std::cout << "in ccc state\n"; },      //
                    hsm::exit                      = [](EmptyContext&) { std::cout << "leaving ccc state\n"; }  //
                    ),
                initial                    = "ccc"_state,
                "jump_to_b"_ev / go_to_bbb = "bbb"_state)  //
            ),                                             //
        "bb"_state(                                        //
            "bbb"_state(                                   //
                enter = [](EmptyContext&) { std::cout << "entering bbb\n"; })));
    EmptyContext con;
    foo.start(con);
    foo.process_event("stay_in_ccc"_ev, con);
    foo.process_event("jump_to_b"_ev, con);
}
