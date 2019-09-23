#include <iostream>
#include "hsm/hsm.hpp"

bool test_guard() { return true; }
int  main()
{
    using namespace hsm;
    auto debug_stuff = []() { std::cout << "Stay in CCC \n"; };
    auto go_to_bbb   = []() { std::cout << "go to bbb\n"; };
    auto foo         = create_state_machine(  //
        "jump_to_b"_ev,  //
        "stay_in_ccc"_ev,
        initial = "c"_state,  //
        "c"_state(            //
            initial = "cc"_state,
            "cc"_state(  //
                "ccc"_state("stay_in_ccc"_ev / debug_stuff = internal),
                "jump_to_b"_ev / go_to_bbb = "bbb"_state)  //
            ),                                             //
        "bb"_state(                                        //
            "bbb"_state(                                   //
                enter = []() { std::cout << "entering bbb\n"; })));
    foo.start();
    foo.process_event("stay_in_ccc"_ev);
    foo.process_event("jump_to_b"_ev);
}
