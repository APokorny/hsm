#include <iostream>
#include <hsm/hsm.hpp>

bool test_guard() { return true; }
template <typename T>
struct print;
template <typename T, typename V>
using item = tiny_tuple::detail::item<T, V>;
int main()
{
    using namespace hsm;
    using namespace tiny_tuple;
    namespace km = kvasir::mpl;
    using namespace std;
    struct Empty
    {
    };
    {
        auto a1   = [](Empty&) {};
        auto a2   = [](Empty&) {};
        auto expr = "a"_state("e"_ev / a1 = "b"_state, "b"_state("e"_ev / a2 = "a"_state));
        static_assert(std::is_same<hsm::back::extract_actions<decltype(expr)>, km::list<decltype(a1), decltype(a2)>>::value,
                      "action list does not match list<a1, a2>");
    }
    {
        using test_sm = decltype(create_state_machine<Empty>("a"_state, "e"_ev, "a"_state + "e"_ev = "a"_state));
        static_assert(std::is_same<map<item<no_event, km::uint_<0>>,                                                          //
                                       item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0, back::transition<4, 1, 1, 0, 0>>>,  //
                                       item<hsm::elit<'e'>, km::uint_<1>>,                                                    //
                                       item<root_state, back::state<0, 0, 1, 0, 0, 0>>>,                                      //
                                   typename test_sm::raw_state_machine>::value,
                      "error");
    }
    {
        using front_test = decltype("a"_state + "e"_ev = hsm::internal);
    }
    {
        using front_test = decltype(("a"_state + "e"_ev) = hsm::internal);
    }

    {
        using front_test = decltype("a"_state + ("e"_ev = hsm::internal));
    }

    {
        using test_sm = decltype(create_state_machine<Empty>("a"_state, "e"_ev, "a"_state + "e"_ev = hsm::internal));
        static_assert(
            std::is_same<
                map<item<no_event, km::uint_<0>>, item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0, back::transition<3, 1, 1, 0, 0>>>,
                    item<hsm::elit<'e'>, km::uint_<1>>, item<root_state, back::state<0, 0, 1, 0, 0, 0>>>,
                typename test_sm::raw_state_machine>::value,
            "error");
    }
    {
        using test_sm = decltype(create_state_machine<Empty>("a"_state, "e"_ev, "e"_ev = hsm::internal));
        static_assert(std::is_same<map<item<no_event, km::uint_<0>>,                                                       //
                                       item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0>>,                                //
                                       item<hsm::elit<'e'>, km::uint_<1>>,                                                 //
                                       item<root_state, back::state<0, 0, 1, 0, 0, 0, back::transition<3, 1, 0, 0, 0>>>>,  //
                                   typename test_sm::raw_state_machine>::value,
                      "invalid");
    }
    {
        using test_sm = decltype(create_state_machine<Empty>("a"_state, "e"_ev, "a"_state + "e"_ev = hsm::X));
        static_assert(std::is_same<map<item<no_event, km::uint_<0>>,                                                           //
                                       item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0, back::transition<12, 1, 0, 0, 0>>>,  //
                                       item<hsm::elit<'e'>, km::uint_<1>>,                                                     //
                                       item<root_state, back::state<0, 0, 1, 0, 0, 0>>>,                                       //
                                   typename test_sm::raw_state_machine>::value,
                      "invalid");
    }
    {
        using test_sm = decltype(create_state_machine<Empty>("a"_state, "e1"_ev, "e2"_ev));
        static_assert(std::is_same<map<item<no_event, km::uint_<0>>,                    //
                                       item<slit<'a'>, back::state<0, 1, 0, 0, 0, 0>>,  //
                                       item<hsm::elit<'e', '1'>, km::uint_<1>>,         //
                                       item<hsm::elit<'e', '2'>, km::uint_<2>>,         //
                                       item<root_state, back::state<0, 0, 1, 0, 0, 0>>>,
                                   typename test_sm::raw_state_machine>::value,
                      "invalid");
    }

    {
        using test_sm = decltype(create_state_machine<Empty>("e1"_ev));
        static_assert(std::is_same<map<item<no_event, km::uint_<0>>,             //
                                       item<hsm::elit<'e', '1'>, km::uint_<1>>,  //
                                       item<root_state, back::state<0, 0, 0, 0, 0, 0>>>,
                                   typename test_sm::raw_state_machine>::value,
                      "invalid");
    }
    {
        auto a1       = [](Empty&) {};
        using test_sm = decltype(create_state_machine<Empty>("a"_state(), hsm::enter = a1));
        static_assert(
            std::is_same<map<item<no_event, km::uint_<0>>,  //
                             item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0>>,
                             item<root_state, back::state<static_cast<uint32_t>(back::state_flags::has_entry), 0, 1, 0, 0, 0>>>,
                         typename test_sm::raw_state_machine>::value,
            "invalid");
    }
    {
        auto a1       = [](Empty&) {};
        using test_sm = decltype(create_state_machine<Empty>("a"_state(), hsm::exit = a1, hsm::enter = a1));
        static_assert(
            std::is_same<map<item<no_event, km::uint_<0>>,  //
                             item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0>>,
                             item<root_state, back::state<static_cast<uint32_t>(back::state_flags::has_entry | back::state_flags::has_exit), 0, 1, 0, 1, 0>>>,
                         typename test_sm::raw_state_machine>::value,
            "invalid");
    }
    {
        auto a1       = [](Empty&) {};
        using test_sm = decltype(create_state_machine<Empty>("a"_state(hsm::enter = a1)));
        static_assert(
            std::is_same<map<item<no_event, km::uint_<0>>,  //
                             item<hsm::slit<'a'>, back::state<static_cast<uint32_t>(back::state_flags::has_entry), 1, 0, 0, 0, 0>>,
                             item<root_state, back::state<0, 0, 1, 0, 0, 0>>>,
                         typename test_sm::raw_state_machine>::value,
            "invalid");
    }
}
