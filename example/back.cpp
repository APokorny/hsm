#include <iostream>
#include "hsm/hsm.hpp"

bool test_guard() { return true; }
template <typename T, typename V>
using item = tiny_tuple::detail::item<T, V>;
int main()
{
    using namespace hsm;
    using namespace tiny_tuple;
    namespace km = kvasir::mpl;
    using namespace std;
    {
        using test_sm = decltype(create_state_machine("a"_state, "e"_ev, "a"_state + "e"_ev = "a"_state));
        static_assert(std::is_same<map<item<root_state, back::state<0, 0, 2, 0, 0, 0>>,                                       //
                                       item<no_event, km::uint_<0>>,                                                          //
                                       item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0, back::transition<4, 1, 1, 0, 0>>>,  //
                                       item<hsm::elit<'e'>, km::uint_<1>>>,
                                   typename test_sm::raw_state_machine>::value);
    }
    {

        using front_test = decltype("a"_state + "e"_ev = hsm::internal);
    }

    {
        using test_sm = decltype(create_state_machine("a"_state, "e"_ev, "a"_state + "e"_ev = hsm::internal));
        static_assert(std::is_same<map<item<root_state, back::state<0, 0, 2, 0, 0, 0>>, item<no_event, km::uint_<0>>,
                                       item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0, back::transition<3, 1, 1, 0, 0>>>,
                                       item<hsm::elit<'e'>, km::uint_<1>>>,
                                   typename test_sm::raw_state_machine>::value);
    }
    {
        using test_sm = decltype(create_state_machine("a"_state, "e"_ev, "e"_ev = hsm::internal));
        static_assert(std::is_same<map<item<root_state, back::state<0, 0, 2, 0, 0, 0, back::transition<3, 1, 0, 0, 0>>>, item<no_event, km::uint_<0>>,
                                       item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0>>,
                                       item<hsm::elit<'e'>, km::uint_<1>>>,
                                   typename test_sm::raw_state_machine>::value);
    }

    {
        using test_sm = decltype(create_state_machine("a"_state, "e"_ev, "a"_state + "e"_ev = hsm::X));
        static_assert(std::is_same<map<item<root_state, back::state<0, 0, 2, 0, 0, 0>>, item<no_event, km::uint_<0>>,
                                       item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0, back::transition<12, 1, 0, 0, 0>>>,
                                       item<hsm::elit<'e'>, km::uint_<1>>>,
                                   typename test_sm::raw_state_machine>::value);
    }
    auto foo = create_state_machine(  //
        "foo"_state,                  //
        initial = "foo"_state,
        "bar"_state(                                                     //
            "blub"_state,                                                //
            initial                                    = "blub"_state,   //
            "evas"_ev[([]() { return test_guard(); })] = hsm::internal,  //
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
