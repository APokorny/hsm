#include <iostream>
#include "hsm/hsm.hpp"

template <typename... Ts>
void front_test(Ts...)
{
}
bool test_guard() { return true; }
int  main()
{
    using namespace hsm;
    front_test(initial = "foo"_state  //
               ,
               "bar"_state(                //
                   initial = "blub"_state  //
                   ,
                   "blub"_state  //
                   ,
                   "evas"_ev[([]() { return test_guard(); })] = internal  //
                   ,
                   enter / []() { std::cout << " foo bar \n"; }  //
                   )                                             //
               ,
               "orthogonal_example"_state(         //
                   initial = "first_region"_state  //
                   ,
                   "first_region"_state + "evterm1"_ev = X  //
                   // ----------------------------------------------------------------------------
                   ,
                   initial = "other_region"_state  //
                   ,
                   "other_region"_state(        //
                       initial = "gully"_state  //
                       ,
                       "gully"_state + "terminate2"_ev = X  //
                       ,
                       hsm::exit / []() { std::cout << "exit other region\n"; }  //
                       )                                                         //
                   ,
                   "other_region"_state = X  //
                   ));
}
