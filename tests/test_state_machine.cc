#include "state_machine.h"
#include "state_machine/containers.h"
#include "state_machine/state_machine.h"
#include "state_machine/transition/transition_table.h"

#include "gtest/gtest.h"
#include <type_traits>

namespace {
using ::state_machine::event;
using ::state_machine::state;
using ::state_machine::placeholder::_;

using ::state_machine::containers::list;
using ::state_machine::state_machine::StateMachine;
using ::state_machine::transition::make_table_from_transition_args;

struct s1 {};
struct s2 {};
struct s3 {
    constexpr s3(int x) : value{x} {}
    int value;
};

struct e1 {};
struct e2 {};
struct e3 {
    constexpr e3(int x) : value{x} {}
    int value;
};

struct always {
    constexpr always(){};
    constexpr auto operator()() const noexcept -> bool { return true; }
};

template <int N>
struct e3_greater_than {
    constexpr e3_greater_than(){};
    constexpr auto operator()(const e3& e) const noexcept -> bool { return e.value > N; }
};

template <int N>
struct e3_less_than {
    constexpr e3_less_than(){};
    constexpr auto operator()(const e3& e) const noexcept -> bool { return e.value < N; }
};

struct noaction {
    constexpr noaction(){};
    constexpr auto operator()() const noexcept -> void { return; }
};

struct return_s2 {
    constexpr return_s2(){};
    constexpr auto operator()() const noexcept -> s2 { return {}; }
};

template <int N>
struct return_s3 {
    constexpr return_s3(){};
    constexpr auto operator()() const noexcept -> s3 { return {N}; }
};

struct return_s3_e3 {
    constexpr return_s3_e3(){};
    constexpr auto operator()(const e3& e) const noexcept -> s3 { return {e.value}; }
};

constexpr auto generate_table() noexcept {
    // clang-format off
    return make_table_from_transition_args(
        state<s1>, event<e2>, always{}, return_s2{}, state<s2>,
        state<s1>, event<e1>, always{}, noaction{}, _,
        state<s1>, event<e2>, always{}, return_s3<3>{}, state<s3>,
        state<s1>, event<e3>, always{}, return_s3_e3{}, state<s3>,
        state<s3>, event<e3>, e3_greater_than<0>{}, return_s2{}, state<s2>,
        state<s3>, event<e3>, e3_less_than<1>{}, return_s3<0>{}, state<s3>);
    // clang-format on
}

} // namespace

TEST(state_machine, event_types) {
    StateMachine<decltype(generate_table())> sm{generate_table()};

    using SM = decltype(sm);

    // order follows from transition table row order
    static_assert(std::is_same<SM::event_types, list<e2, e1, e3>>::value, "");
}

TEST(state_machine, state_types) {
    StateMachine<decltype(generate_table())> sm{generate_table()};

    using SM = decltype(sm);

    // order follows from transition table row order (all sources, then all destinations).
    static_assert(std::is_same<SM::state_types, list<s1, s3, s2>>::value, "");
}