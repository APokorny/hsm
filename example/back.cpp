#include <iostream>
#include "hsm/hsm.hpp"

bool test_guard() { return true; }
int  main()
{
    using namespace hsm;
    auto foo = create_state_machine(  //
        "foo"_state,                  //
        initial = "foo"_state,
        "bar"_state(                                                    //
            "blub"_state,                                               //
            initial                                    = "blub"_state,  //
            "evas"_ev[([]() { return test_guard(); })] = internal,      //
            enter / []() { std::cout << " foo bar \n"; }),
        "orthogonal_example"_state(                                                                       //
            "first_region"_state,                                                                         //
            initial                                                              = "first_region"_state,  //
            "first_region"_state + "evterm1"_ev / []() { std::cout << "bubbl"; } = X,                     //
            // ----------------------------------------------------------------------------
            initial = "other_region"_state,                       //
            "other_region"_state(                                 //
                "gully"_state,                                    //
                initial                         = "gully"_state,  //
                "gully"_state + "terminate2"_ev = X,              //
                hsm::exit / []() { std::cout << "exit other region\n"; }),
            "other_region"_state = X));
   // std::cout << "blub:| " << foo << std::endl;
}
