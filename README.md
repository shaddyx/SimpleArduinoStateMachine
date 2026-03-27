# SimpleArduinoStateMachine

A small, header-only state machine helper for Arduino and PlatformIO projects.

The library gives you:

- A generic `SimpleStateMachine<T>` class
- Optional transition validation callback
- Optional next-state callback for deterministic state flow
- Optional state-changed callback when transition succeeds
- Helper macros to keep transition rules readable

## Features

- Header-only implementation (`src/state_machine/simple_state_machine.h`)
- Works with strongly-typed enums (`enum class`)
- Optional guard function to allow/reject transitions
- Optional next-state resolver used by `transitNext()`
- Optional state-changed notification callback
- Force state override API for recovery/manual control
- Lightweight and suitable for embedded projects

## Installation

### PlatformIO library dependency

If you package this as a library, add it to your project and include the header:

```cpp
#include "state_machine/simple_state_machine.h"
```

### Current repository setup

This repository already contains the source under:

- `src/state_machine/simple_state_machine.h`

No additional build step is required for the library itself.

## Quick Start

```cpp
#include <Arduino.h>
#include "state_machine/simple_state_machine.h"

enum class DeviceState : uint8_t {
		Idle,
		Running,
		Paused
};

SimpleStateMachine<DeviceState> sm(
		DeviceState::Idle,
		SimpleStateMachineCallbackStart(DeviceState) {
				SimpleStateMachineAllowedTransition(DeviceState::Idle, DeviceState::Running);
				SimpleStateMachineAllowedTransition(DeviceState::Running, DeviceState::Paused);
				SimpleStateMachineAllowedTransition(DeviceState::Paused, DeviceState::Idle);
				SimpleStateMachineAllowedTransitionsEnd();
		}
);

void setup() {
		sm.transit(DeviceState::Running);  // true
		sm.transit(DeviceState::Paused);   // true
		sm.transit(DeviceState::Running);  // false (not listed above)
}

void loop() {}
```

## Core API

### Type aliases

```cpp
template <typename T>
using SimpleStateMachineCallback = bool (*)(T oldState, T newState);

template <typename T>
using SimpleStateMachineNextStateCallback = T (*)(T oldState);

template <typename T>
using StateChangedCallback = void (*)(T oldState, T newState);
```

### Constructor

```cpp
SimpleStateMachine(
		T initialState,
		SimpleStateMachineCallback<T> callback = nullptr,
		SimpleStateMachineNextStateCallback<T> nextStateCallback = nullptr,
		StateChangedCallback<T> stateChangedCallback = nullptr
)
```

- `initialState`: initial current state
- `callback`: optional transition validator
- `nextStateCallback`: optional resolver used by `transitNext()`
- `stateChangedCallback`: optional callback invoked on successful transition (`oldState`, `newState`)

### Methods

- `bool transit(T newState)`
	- If a transition callback exists, it must return `true` for the state to change.
	- Returns `true` on success, `false` when rejected.

- `bool transitNext()`
	- Calls `nextStateCallback(currentState)` to get target state.
	- Then calls `transit(target)`.
	- Returns `false` if no next-state callback is set.

- `T getState() const`
	- Returns the current state.

- `void forceState(T newState)`
	- Sets the state directly, skipping transition validation.

- `void setCallback(SimpleStateMachineCallback<T> newCallback)`
	- Updates transition callback at runtime.

- `void setNextStateCallback(SimpleStateMachineNextStateCallback<T> newNextStateCallback)`
	- Updates next-state callback at runtime.

Note: there is currently no dedicated setter for `stateChangedCallback`; set it through the constructor.

## Helper Macros

These macros are intended to make callback bodies concise.

### Transition validation helpers

- `SimpleStateMachineCallbackStart(STATE_TYPE)`
	- Starts a lambda with signature `(STATE_TYPE oldState, STATE_TYPE newState)`

- `SimpleStateMachineAllowedTransition(OLD_STATE, NEW_STATE)`
	- Adds one allowed transition rule and returns `true` if matched

- `SimpleStateMachineAllowedTransitionsEnd()`
	- Ends callback and returns `false` if no rules matched

Example:

```cpp
SimpleStateMachineCallbackStart(MyState) {
		SimpleStateMachineAllowedTransition(MyState::Idle, MyState::Running);
		SimpleStateMachineAllowedTransition(MyState::Running, MyState::Idle);
		SimpleStateMachineAllowedTransitionsEnd();
}
```

### Next-state helpers

- `SimpleStateMachineNextStateCallbackStart(STATE_TYPE)`
	- Starts a lambda with signature `(STATE_TYPE oldState)`

- `SimpleStateMachineNextState(FROM_STATE, TO_STATE)`
	- Returns `TO_STATE` if current state is `FROM_STATE`

- `SimpleStateMachineNextStateEnd()`
	- Ends callback and returns `oldState` if no mapping matched

Example:

```cpp
SimpleStateMachineNextStateCallbackStart(MyState) {
		SimpleStateMachineNextState(MyState::Idle, MyState::Running);
		SimpleStateMachineNextState(MyState::Running, MyState::Paused);
		SimpleStateMachineNextState(MyState::Paused, MyState::Idle);
		SimpleStateMachineNextStateEnd();
}
```

### State-changed callback helper

- `SimpleStateMachineChangedCallbackStart(STATE_TYPE)`
	- Starts a lambda with signature `(STATE_TYPE oldState, STATE_TYPE newState)`

Example with a normal function:

```cpp
enum class MyState : uint8_t { Idle, Running, Paused };

MyState lastOldState = MyState::Idle;
MyState lastNewState = MyState::Idle;

void onStateChanged(MyState oldState, MyState newState) {
		lastOldState = oldState;
		lastNewState = newState;
}

SimpleStateMachine<MyState> sm(
		MyState::Idle,
		SimpleStateMachineCallbackStart(MyState) {
				SimpleStateMachineAllowedTransition(MyState::Idle, MyState::Running);
				SimpleStateMachineAllowedTransition(MyState::Running, MyState::Paused);
				SimpleStateMachineAllowedTransitionsEnd();
		},
		nullptr,
		onStateChanged
);
```

## Full Example with transitNext()

```cpp
enum class MyState : uint8_t { Idle, Running, Paused };

SimpleStateMachine<MyState> sm(
		MyState::Idle,
		SimpleStateMachineCallbackStart(MyState) {
				SimpleStateMachineAllowedTransition(MyState::Idle, MyState::Running);
				SimpleStateMachineAllowedTransition(MyState::Running, MyState::Paused);
				SimpleStateMachineAllowedTransition(MyState::Paused, MyState::Idle);
				SimpleStateMachineAllowedTransitionsEnd();
		},
		SimpleStateMachineNextStateCallbackStart(MyState) {
				SimpleStateMachineNextState(MyState::Idle, MyState::Running);
				SimpleStateMachineNextState(MyState::Running, MyState::Paused);
				SimpleStateMachineNextState(MyState::Paused, MyState::Idle);
				SimpleStateMachineNextStateEnd();
		}
);

void loop() {
		sm.transitNext();
}
```

## Testing

This repository includes Unity-based tests in:

- `test/test_simple/test_simple.cpp`

Run tests with PlatformIO:

```bash
pio test -e local
```

## Notes and Behavior Details

- If no transition callback is provided, `transit()` always updates state.
- If no next-state callback is provided, `transitNext()` returns `false` and does not modify state.
- `stateChangedCallback` uses a function pointer type, so capturing lambdas are not accepted.
- `stateChangedCallback` is invoked in the successful validated-transition path.
- `forceState()` bypasses transition checks and should be used carefully.
- Since the implementation is template-based in a header, `simple_state_machine.cpp` can remain empty.

## License

Add your preferred license here (for example, MIT).
