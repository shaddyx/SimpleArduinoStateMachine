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
    // should ignore allowed transitions and force the state
    mainStateMachine.forceState(SomeState::Paused);
    TEST_ASSERT_EQUAL(SomeState::Paused, mainStateMachine.getState());
    // should ignore allowed transitions and force the state
    mainStateMachine.forceState(SomeState::Idle);
    TEST_ASSERT_EQUAL(SomeState::Idle, mainStateMachine.getState());
}

void simple_next_state_test(){
    enum class SomeState: uint8_t {
        Idle,
        Running,
        Paused
    };
    SimpleStateMachineCallback<SomeState> transitions = SimpleStateMachineCallbackStart(SomeState) {
        SimpleStateMachineAllowedTransition(SomeState::Idle, SomeState::Running);
        SimpleStateMachineAllowedTransition(SomeState::Running, SomeState::Paused);
        SimpleStateMachineAllowedTransition(SomeState::Paused, SomeState::Idle);
        SimpleStateMachineAllowedTransitionsEnd();
    };
    SimpleStateMachine<SomeState> mainStateMachine(SomeState::Idle, transitions, SimpleStateMachineNextStateCallbackStart(SomeState) {
        SimpleStateMachineNextState(SomeState::Idle, SomeState::Running);
        SimpleStateMachineNextState(SomeState::Running, SomeState::Paused);
        SimpleStateMachineNextState(SomeState::Paused, SomeState::Idle);
        SimpleStateMachineNextStateEnd();
    });
    TEST_ASSERT_TRUE(mainStateMachine.transitNext());
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());
    TEST_ASSERT_TRUE(mainStateMachine.transitNext());
    TEST_ASSERT_EQUAL(SomeState::Paused, mainStateMachine.getState());
    TEST_ASSERT_TRUE(mainStateMachine.transitNext());
    TEST_ASSERT_EQUAL(SomeState::Idle, mainStateMachine.getState());
    // Test transitNext with no next state callback defined
    SimpleStateMachine<SomeState> noNextStateMachine(SomeState::Idle);
    TEST_ASSERT_FALSE(noNextStateMachine.transitNext());
    TEST_ASSERT_EQUAL(SomeState::Idle, noNextStateMachine.getState());
}

// test state changed callback

// void state_changed_callback(SomeState oldState, SomeState newState){
//     lastOldState = oldState;
//     lastNewState = newState;
// }

enum class SomeStateForChangeTest: uint8_t {
    Idle,
    Running,
    Paused
};

SomeStateForChangeTest lastOldState = SomeStateForChangeTest::Idle;
SomeStateForChangeTest lastNewState = SomeStateForChangeTest::Idle;

void state_changed_callback(SomeStateForChangeTest oldState, SomeStateForChangeTest newState){
    lastOldState = oldState;
    lastNewState = newState;
}

void simple_state_changed_callback_test(){
    
    
    SimpleStateMachine<SomeStateForChangeTest> mainStateMachine(SomeStateForChangeTest::Idle, SimpleStateMachineCallbackStart(SomeStateForChangeTest) {
        SimpleStateMachineAllowedTransition(SomeStateForChangeTest::Idle, SomeStateForChangeTest::Running);
        SimpleStateMachineAllowedTransition(SomeStateForChangeTest::Running, SomeStateForChangeTest::Paused);
        SimpleStateMachineAllowedTransitionsEnd();
    }, nullptr, state_changed_callback);
    mainStateMachine.transit(SomeStateForChangeTest::Running);
    TEST_ASSERT_EQUAL(SomeStateForChangeTest::Idle, lastOldState);
    TEST_ASSERT_EQUAL(SomeStateForChangeTest::Running, lastNewState);
    mainStateMachine.transit(SomeStateForChangeTest::Paused);
    TEST_ASSERT_EQUAL(SomeStateForChangeTest::Running, lastOldState);
    TEST_ASSERT_EQUAL(SomeStateForChangeTest::Paused, lastNewState);
}

void simple_state_transit_if_state_test(){
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
    // Should transit because current state is Idle
    TEST_ASSERT_TRUE(mainStateMachine.transitIfState(SomeState::Idle, SomeState::Running));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());
    // Should not transit because current state is Running, not Idle
    TEST_ASSERT_FALSE(mainStateMachine.transitIfState(SomeState::Idle, SomeState::Paused));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());
    // Should transit because current state is Running
    TEST_ASSERT_TRUE(mainStateMachine.transitIfState(SomeState::Running, SomeState::Paused));
    TEST_ASSERT_EQUAL(SomeState::Paused, mainStateMachine.getState());
}

void simple_state_transit_if_condition_test(){
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

    // Condition true, valid transition
    TEST_ASSERT_TRUE(mainStateMachine.transitIfCondition(true, SomeState::Running));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());

    // Condition false, transition must not happen
    TEST_ASSERT_FALSE(mainStateMachine.transitIfCondition(false, SomeState::Paused));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());

    // Condition true but transition invalid by callback rules
    TEST_ASSERT_FALSE(mainStateMachine.transitIfCondition(true, SomeState::Idle));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());
}

void simple_state_transit_if_condition_and_state_test(){
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

    // Both checks pass
    TEST_ASSERT_TRUE(mainStateMachine.transitIfConditionAndState(true, SomeState::Idle, SomeState::Running));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());

    // Condition false
    TEST_ASSERT_FALSE(mainStateMachine.transitIfConditionAndState(false, SomeState::Running, SomeState::Paused));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());

    // State mismatch
    TEST_ASSERT_FALSE(mainStateMachine.transitIfConditionAndState(true, SomeState::Idle, SomeState::Paused));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());

    // Condition and state pass, allowed transition succeeds
    TEST_ASSERT_TRUE(mainStateMachine.transitIfConditionAndState(true, SomeState::Running, SomeState::Paused));
    TEST_ASSERT_EQUAL(SomeState::Paused, mainStateMachine.getState());
}

void simple_state_transit_next_if_condition_test(){
    enum class SomeState: uint8_t {
        Idle,
        Running,
        Paused
    };
    SimpleStateMachineCallback<SomeState> transitions = SimpleStateMachineCallbackStart(SomeState) {
        SimpleStateMachineAllowedTransition(SomeState::Idle, SomeState::Running);
        SimpleStateMachineAllowedTransition(SomeState::Running, SomeState::Paused);
        SimpleStateMachineAllowedTransitionsEnd();
    };
    SimpleStateMachine<SomeState> mainStateMachine(SomeState::Idle, transitions, SimpleStateMachineNextStateCallbackStart(SomeState) {
        SimpleStateMachineNextState(SomeState::Idle, SomeState::Running);
        SimpleStateMachineNextState(SomeState::Running, SomeState::Paused);
        SimpleStateMachineNextStateEnd();
    });

    // Condition true, next transition should happen
    TEST_ASSERT_TRUE(mainStateMachine.transitNextIfCondition(true));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());

    // Condition false, transition blocked
    TEST_ASSERT_FALSE(mainStateMachine.transitNextIfCondition(false));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());

    // Without nextStateCallback, should return false
    SimpleStateMachine<SomeState> noNextStateMachine(SomeState::Idle, transitions);
    TEST_ASSERT_FALSE(noNextStateMachine.transitNextIfCondition(true));
    TEST_ASSERT_EQUAL(SomeState::Idle, noNextStateMachine.getState());
}

void simple_state_transit_next_if_state_test(){
    enum class SomeState: uint8_t {
        Idle,
        Running,
        Paused
    };
    SimpleStateMachineCallback<SomeState> transitions = SimpleStateMachineCallbackStart(SomeState) {
        SimpleStateMachineAllowedTransition(SomeState::Idle, SomeState::Running);
        SimpleStateMachineAllowedTransition(SomeState::Running, SomeState::Paused);
        SimpleStateMachineAllowedTransitionsEnd();
    };
    SimpleStateMachine<SomeState> mainStateMachine(SomeState::Idle, transitions, SimpleStateMachineNextStateCallbackStart(SomeState) {
        SimpleStateMachineNextState(SomeState::Idle, SomeState::Running);
        SimpleStateMachineNextState(SomeState::Running, SomeState::Paused);
        SimpleStateMachineNextStateEnd();
    });

    // Matching state, should transit to Running
    TEST_ASSERT_TRUE(mainStateMachine.transitNextIfState(SomeState::Idle));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());

    // Mismatching expected state, should fail
    TEST_ASSERT_FALSE(mainStateMachine.transitNextIfState(SomeState::Idle));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());

    // Matching current state, should transit to Paused
    TEST_ASSERT_TRUE(mainStateMachine.transitNextIfState(SomeState::Running));
    TEST_ASSERT_EQUAL(SomeState::Paused, mainStateMachine.getState());

    // Without nextStateCallback, should return false
    SimpleStateMachine<SomeState> noNextStateMachine(SomeState::Idle, transitions);
    TEST_ASSERT_FALSE(noNextStateMachine.transitNextIfState(SomeState::Idle));
    TEST_ASSERT_EQUAL(SomeState::Idle, noNextStateMachine.getState());
}

void simple_state_transit_next_if_condition_and_state_test(){
    enum class SomeState: uint8_t {
        Idle,
        Running,
        Paused
    };
    SimpleStateMachineCallback<SomeState> transitions = SimpleStateMachineCallbackStart(SomeState) {
        SimpleStateMachineAllowedTransition(SomeState::Idle, SomeState::Running);
        SimpleStateMachineAllowedTransition(SomeState::Running, SomeState::Paused);
        SimpleStateMachineAllowedTransitionsEnd();
    };
    SimpleStateMachine<SomeState> mainStateMachine(SomeState::Idle, transitions, SimpleStateMachineNextStateCallbackStart(SomeState) {
        SimpleStateMachineNextState(SomeState::Idle, SomeState::Running);
        SimpleStateMachineNextState(SomeState::Running, SomeState::Paused);
        SimpleStateMachineNextStateEnd();
    });

    // All checks pass
    TEST_ASSERT_TRUE(mainStateMachine.transitNextIfConditionAndState(true, SomeState::Idle));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());

    // Condition false
    TEST_ASSERT_FALSE(mainStateMachine.transitNextIfConditionAndState(false, SomeState::Running));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());

    // State mismatch
    TEST_ASSERT_FALSE(mainStateMachine.transitNextIfConditionAndState(true, SomeState::Idle));
    TEST_ASSERT_EQUAL(SomeState::Running, mainStateMachine.getState());

    // Condition and state match
    TEST_ASSERT_TRUE(mainStateMachine.transitNextIfConditionAndState(true, SomeState::Running));
    TEST_ASSERT_EQUAL(SomeState::Paused, mainStateMachine.getState());

    // Without nextStateCallback, should return false
    SimpleStateMachine<SomeState> noNextStateMachine(SomeState::Idle, transitions);
    TEST_ASSERT_FALSE(noNextStateMachine.transitNextIfConditionAndState(true, SomeState::Idle));
    TEST_ASSERT_EQUAL(SomeState::Idle, noNextStateMachine.getState());
}

void setUp(void)
{
    ArduinoFakeReset();
}

int main(){
    UNITY_BEGIN();
    RUN_TEST(simple_state_machine_test);
    RUN_TEST(simple_force_state_test);
    RUN_TEST(simple_next_state_test);
    RUN_TEST(simple_state_changed_callback_test);
    RUN_TEST(simple_state_transit_if_state_test);
    RUN_TEST(simple_state_transit_if_condition_test);
    RUN_TEST(simple_state_transit_if_condition_and_state_test);
    RUN_TEST(simple_state_transit_next_if_condition_test);
    RUN_TEST(simple_state_transit_next_if_state_test);
    RUN_TEST(simple_state_transit_next_if_condition_and_state_test);
    UNITY_END();
    return 0;
}