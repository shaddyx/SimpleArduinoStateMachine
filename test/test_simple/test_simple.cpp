//#include <Arduino.h>
#include <ArduinoFake.h>
#include "state_machine/simple_state_machine.h"
#include <unity.h>

using namespace fakeit;

void simple_state_machine_test(){
    enum class SomeTestState: uint8_t {
        Idle,
        Running,
        Paused
    };

    SimpleStateMachine<SomeTestState> mainStateMachine(SomeTestState::Idle, SimpleStateMachineCallbackStart(SomeTestState) {
        SimpleStateMachineAllowedTransition(SomeTestState::Idle, SomeTestState::Running);
        SimpleStateMachineAllowedTransition(SomeTestState::Running, SomeTestState::Paused);
        SimpleStateMachineAllowedTransition(SomeTestState::Paused, SomeTestState::Idle);
        SimpleStateMachineAllowedTransition(SomeTestState::Paused, SomeTestState::Running);
        SimpleStateMachineAllowedTransitionsEnd();
    });

    TEST_ASSERT_TRUE(mainStateMachine.transit(SomeTestState::Running));
    TEST_ASSERT_EQUAL(SomeTestState::Running, mainStateMachine.getState());
    TEST_ASSERT_TRUE(mainStateMachine.transit(SomeTestState::Paused));
    TEST_ASSERT_EQUAL(SomeTestState::Paused, mainStateMachine.getState());
    TEST_ASSERT_TRUE(mainStateMachine.transit(SomeTestState::Idle));
    TEST_ASSERT_EQUAL(SomeTestState::Idle, mainStateMachine.getState());

    // Test invalid transition
    TEST_ASSERT_FALSE(mainStateMachine.transit(SomeTestState::Paused)); // Transition from Idle to Running is allowed, but from Idle to Paused is not allowed
    TEST_ASSERT_EQUAL(SomeTestState::Idle, mainStateMachine.getState()); // State should remain Idle

}

void simple_force_state_test(){
    enum class SomeState: uint8_t {
        Idle,
        Running,
        Paused
    };
    SimpleStateMachine<SomeState> mainStateMachine(SomeState::Idle, SimpleStateMachineCallbackStart(SomeState) {
        SimpleStateMachineAllowedTransition(SomeState::Idle, SomeState::Running);
        SimpleStateMachineAllowedTransition(SomeState::Running, SomeState::Paused);
        SimpleStateMachineAllowedTransitionsEnd();
    });
    mainStateMachine.forceState(SomeState::Paused);
    TEST_ASSERT_EQUAL(SomeState::Paused, mainStateMachine.getState());
    mainStateMachine.forceState(SomeState::Idle);
    TEST_ASSERT_EQUAL(SomeState::Idle, mainStateMachine.getState());
}

void setUp(void)
{
    ArduinoFakeReset();
}

int main(){
    UNITY_BEGIN();
    RUN_TEST(simple_state_machine_test);
    RUN_TEST(simple_force_state_test);
    UNITY_END();
    return 0;
}