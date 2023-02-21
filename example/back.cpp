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
        static_assert(std::is_same<map<item<no_event, km::uint_<0>>,  //
                                       item<back::history_table, std::integer_sequence<size_t>>,
                                       item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0, 0, back::transition<4, 1, 1, 0, 0>>>,  //
                                       item<hsm::elit<'e'>, km::uint_<1>>,                                                       //
                                       item<root_state, back::state<0, 0, 1, 0, 0, 0, 0>>>,                                      //
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
        static_assert(std::is_same<map<item<no_event, km::uint_<0>>,                              //
                                       item<back::history_table, std::integer_sequence<size_t>>,  //
                                       item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0, 0, back::transition<3, 1, 1, 0, 0>>>,
                                       item<hsm::elit<'e'>, km::uint_<1>>, item<root_state, back::state<0, 0, 1, 0, 0, 0, 0>>>,
                                   typename test_sm::raw_state_machine>::value,
                      "error");
    }
    {
        using test_sm = decltype(create_state_machine<Empty>("a"_state, "e"_ev, "e"_ev = hsm::internal));
        static_assert(std::is_same<map<item<no_event, km::uint_<0>>,                                                          //
                                       item<back::history_table, std::integer_sequence<size_t>>,                              //
                                       item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0, 0>>,                                //
                                       item<hsm::elit<'e'>, km::uint_<1>>,                                                    //
                                       item<root_state, back::state<0, 0, 1, 0, 0, 0, 0, back::transition<3, 1, 0, 0, 0>>>>,  //
                                   typename test_sm::raw_state_machine>::value,
                      "invalid");
    }
    {
        using test_sm = decltype(create_state_machine<Empty>("a"_state, "e"_ev, "a"_state + "e"_ev = hsm::X));
        static_assert(std::is_same<map<item<no_event, km::uint_<0>>,                                                              //
                                       item<back::history_table, std::integer_sequence<size_t>>,                                  //
                                       item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0, 0, back::transition<12, 1, 0, 0, 0>>>,  //
                                       item<hsm::elit<'e'>, km::uint_<1>>,                                                        //
                                       item<root_state, back::state<0, 0, 1, 0, 0, 0, 0>>>,                                       //
                                   typename test_sm::raw_state_machine>::value,
                      "invalid");
    }
    {
        using test_sm = decltype(create_state_machine<Empty>("a"_state, "e1"_ev, "e2"_ev));
        static_assert(std::is_same<map<item<no_event, km::uint_<0>>,                              //
                                       item<back::history_table, std::integer_sequence<size_t>>,  //
                                       item<slit<'a'>, back::state<0, 1, 0, 0, 0, 0, 0>>,         //
                                       item<hsm::elit<'e', '1'>, km::uint_<1>>,                   //
                                       item<hsm::elit<'e', '2'>, km::uint_<2>>,                   //
                                       item<root_state, back::state<0, 0, 1, 0, 0, 0, 0>>>,
                                   typename test_sm::raw_state_machine>::value,
                      "invalid");
    }

    {
        using test_sm = decltype(create_state_machine<Empty>("e1"_ev));
        static_assert(std::is_same<map<item<no_event, km::uint_<0>>,                              //
                                       item<back::history_table, std::integer_sequence<size_t>>,  //
                                       item<hsm::elit<'e', '1'>, km::uint_<1>>,                   //
                                       item<root_state, back::state<0, 0, 0, 0, 0, 0, 0>>>,
                                   typename test_sm::raw_state_machine>::value,
                      "invalid");
    }
    {
        auto a1       = [](Empty&) {};
        using test_sm = decltype(create_state_machine<Empty>("a"_state(), hsm::enter = a1));
        static_assert(
            std::is_same<map<item<no_event, km::uint_<0>>,                              //
                             item<back::history_table, std::integer_sequence<size_t>>,  //
                             item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0, 0>>,
                             item<root_state, back::state<static_cast<uint32_t>(back::state_flags::has_entry), 0, 1, 0, 0, 0, 0>>>,
                         typename test_sm::raw_state_machine>::value,
            "invalid");
    }
    {
        auto a1       = [](Empty&) {};
        using test_sm = decltype(create_state_machine<Empty>("a"_state(), hsm::exit = a1, hsm::enter = a1));
        static_assert(
            std::is_same<map<item<no_event, km::uint_<0>>,                              //
                             item<back::history_table, std::integer_sequence<size_t>>,  //
                             item<hsm::slit<'a'>, back::state<0, 1, 0, 0, 0, 0, 0>>,
                             item<root_state, back::state<static_cast<uint32_t>(back::state_flags::has_entry | back::state_flags::has_exit),
                                                          0, 1, 0, 1, 0, 0>>>,
                         typename test_sm::raw_state_machine>::value,
            "invalid");
    }
    {
        auto a1       = [](Empty&) {};
        using test_sm = decltype(create_state_machine<Empty>("a"_state(hsm::enter = a1)));
        static_assert(
            std::is_same<map<item<no_event, km::uint_<0>>,                              //
                             item<back::history_table, std::integer_sequence<size_t>>,  //
                             item<hsm::slit<'a'>, back::state<static_cast<uint32_t>(back::state_flags::has_entry), 1, 0, 0, 0, 0, 0>>,
                             item<root_state, back::state<0, 0, 1, 0, 0, 0, 0>>>,
                         typename test_sm::raw_state_machine>::value,
            "invalid");
    }
    {
        auto a1       = [](Empty&) {};
        using test_sm = decltype(create_state_machine<Empty>("a"_state(hsm::history = "b"_state, "b"_state)));
        static_assert(
            std::is_same<map<item<no_event, km::uint_<0>>,                                 //
                             item<back::history_table, std::integer_sequence<size_t, 2>>,  //
                             item<hsm::slit<'b'>, back::state<0, 2, 0, 1, 0, 0, 0>>,
                             item<hsm::slit<'a'>, back::state<static_cast<uint32_t>(back::state_flags::has_history), 1, 1, 0, 0, 0, 0>>,
                             item<root_state, back::state<0, 0, 2, 0, 0, 0, 0>>>,
                         typename test_sm::raw_state_machine>::value,
            "invalid");
    }
    {
        auto a1       = [](Empty&) {};
        using test_sm = decltype(create_state_machine<Empty>("a"_state(hsm::deep_history = "b"_state, "b"_state)));

        static_assert(
            std::is_same<
                map<item<no_event, km::uint_<0>>,                                 //
                    item<back::history_table, std::integer_sequence<size_t, 2>>,  //
                    item<hsm::slit<'b'>, back::state<0, 2, 0, 1, 0, 0, 0>>,
                    item<hsm::slit<'a'>, back::state<static_cast<uint32_t>(back::state_flags::has_deep_history), 1, 1, 0, 0, 0, 0>>,
                    item<root_state, back::state<0, 0, 2, 0, 0, 0, 0>>>,
                typename test_sm::raw_state_machine>::value,
            "invalid");
    }
    {
        using test_sm                   = decltype(create_state_machine<Empty>(  //
            "a"_state, "ee"_ev = deep_history_of("a"_state)));
        constexpr size_t state_a        = 1;
        constexpr size_t ee_event       = 1;
        constexpr size_t has_no_history = 0;  //! yes the state machine definition is broken
        static_assert(
            std::is_same<map<item<no_event, km::uint_<0>>,                                               //
                             item<back::history_table, std::integer_sequence<size_t>>,                   //
                             item<hsm::slit<'a'>, back::state<has_no_history, state_a, 0, 0, 0, 0, 0>>,  //
                             item<hsm::elit<'e', 'e'>, kvasir::mpl::uint_<ee_event>>,                    //
                             item<root_state, back::state<0, 0, 1, 0, 0, 0, 0,
                                                          back::transition<static_cast<uint8_t>(back::transition_flags::to_deep_history |
                                                                                                back::transition_flags::normal),
                                                                           ee_event, state_a, 0, 0>>>>,
                         typename test_sm::raw_state_machine>::value,
            "invalid");
    }
    {
        using test_sm                   = decltype(create_state_machine<Empty>(  //
            "a"_state, "ee"_ev = history_of("a"_state)));
        constexpr size_t state_a        = 1;
        constexpr size_t ee_event       = 1;
        constexpr size_t has_no_history = 0;  //! yes the state machine definition is broken
        static_assert(
            std::is_same<map<item<no_event, km::uint_<0>>,                                               //
                             item<back::history_table, std::integer_sequence<size_t>>,                   //
                             item<hsm::slit<'a'>, back::state<has_no_history, state_a, 0, 0, 0, 0, 0>>,  //
                             item<hsm::elit<'e', 'e'>, kvasir::mpl::uint_<ee_event>>,                    //
                             item<root_state, back::state<0, 0, 1, 0, 0, 0, 0,
                                                          back::transition<static_cast<uint8_t>(back::transition_flags::to_shallow_history |
                                                                                                back::transition_flags::normal),
                                                                           ee_event, state_a, 0, 0>>>>,
                         typename test_sm::raw_state_machine>::value,
            "invalid");
    }
    {
        using test_sm             = decltype(create_state_machine<Empty>(  //
            "a"_state("ee"_ev = hsm::X)));
        constexpr size_t state_a  = 1;
        constexpr size_t ee_event = 1;

        static_assert(
            std::is_same<map<item<no_event, km::uint_<0>>,                              //
                             item<back::history_table, std::integer_sequence<size_t>>,  //
                             item<hsm::elit<'e', 'e'>, kvasir::mpl::uint_<ee_event>>,   //
                             item<hsm::slit<'a'>, back::state<0, state_a, 0, 0, 0, 0, 0,
                                                              back::transition<static_cast<uint8_t>(back::transition_flags::to_final |
                                                                                                    back::transition_flags::normal),
                                                                               ee_event, state_a, 0, 0>>>,  //
                             item<root_state, back::state<0, 0, 1, 0, 0, 0, 0>>>,
                         typename test_sm::raw_state_machine>::value,
            "invalid");
    }
    {
        auto a1 = [](Empty&) {};
        using test_sm =
            decltype(create_state_machine<Empty>("a"_state(hsm::deep_history = "b"_state, "b"_state(hsm::history = "c"_state, "c"_state))));
        static_assert(
            std::is_same<
                map<item<no_event, km::uint_<0>>,                                    //
                    item<back::history_table, std::integer_sequence<size_t, 2, 3>>,  //
                    item<hsm::slit<'c'>, back::state<0, 3, 0, 2, 0, 0, 0>>,
                    item<hsm::slit<'b'>, back::state<static_cast<uint32_t>(back::state_flags::has_history), 2, 1, 1, 0, 0, 1>>,
                    item<hsm::slit<'a'>, back::state<static_cast<uint32_t>(back::state_flags::has_deep_history), 1, 2, 0, 0, 0, 0>>,
                    item<root_state, back::state<0, 0, 3, 0, 0, 0, 0>>>,
                typename test_sm::raw_state_machine>::value,
            "invalid");
    }
}
