#pragma once
#include <Arduino.h>

template <typename T>
bool _checkStateTransitionAllowed(T oldState, T newState, T allowedOldState, T allowedNewState)
{
    return (oldState == allowedOldState && newState == allowedNewState);
}

#define SimpleStateMachineAllowedTransition(OLD_STATE, NEW_STATE) \
    if (_checkStateTransitionAllowed(oldState, newState, OLD_STATE, NEW_STATE)) { \
        return true; \
    }

#define SimpleStateMachineAllowedTransitionsEnd() \
    return false;

#define SimpleStateMachineNextState(FROM_STATE, TO_STATE) \
    if (oldState == FROM_STATE) { \
        return TO_STATE; \
    }

#define SimpleStateMachineNextStateEnd() \
    return oldState; // No transition, return current state


#define SimpleStateMachineCallbackStart(STATE_TYPE) \
    [](STATE_TYPE oldState, STATE_TYPE newState)
#define SimpleStateMachineNextStateCallbackStart(STATE_TYPE) \
    [](STATE_TYPE oldState)

template <typename T>
using SimpleStateMachineCallback = bool (*)(T oldState, T newState);

template <typename T>
using SimpleStateMachineNextStateCallback = T (*)(T oldState);

template <typename T>
class SimpleStateMachine
{
public:
    SimpleStateMachine(
        T initialState, 
        SimpleStateMachineCallback<T> callback = nullptr, 
        SimpleStateMachineNextStateCallback<T> nextStateCallback = nullptr
    ) : state(initialState), 
        callback(callback), 
        nextStateCallback(nextStateCallback) {}
    /**
     * Attempts to transition to a new state. Returns true if the transition was successful, false if it was rejected by the callback.
     */
    bool transit(T newState)
    {
        if (callback)
        {
            if (callback(state, newState))
            {
                state = newState;
            }
            else
            {
                return false; // State transition rejected by callback
            }
        }
        else
        {
            state = newState;
        }
        return true;
    }

    bool transitNext()
    {
        if (nextStateCallback)
        {
            T newState = nextStateCallback(state);
            return transit(newState);
        }
        return false; // No next state callback defined
    }
    

    T getState() const
    {
        return state;
    }
    
    /**
     * Forces the state machine into a new state without calling the callback. Use with caution, as this can lead to inconsistent states if not used properly.
     */
    void forceState(T newState)
    {
        state = newState;
    }

    void setCallback(SimpleStateMachineCallback<T> newCallback)
    {
        callback = newCallback;
    }

    void setNextStateCallback(SimpleStateMachineNextStateCallback<T> newNextStateCallback)
    {
        nextStateCallback = newNextStateCallback;
    }

private:
    T state;
    SimpleStateMachineCallback<T> callback;
    SimpleStateMachineNextStateCallback<T> nextStateCallback;
};
