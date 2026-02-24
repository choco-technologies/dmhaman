# dmhaman
DMOD Handlers Manager

## Overview

`dmhaman` is a DMOD library module that provides a central registry for
mapping handler names to function pointers.  It is designed for embedded
systems where configuration files (e.g., `*.ini`) need to reference handlers
by name, allowing runtime binding of interrupt handlers or callback functions.

## Features

- **Generic storage** – register any function pointer under a named key and
  retrieve it later with `dmhaman_get_handler()`.
- **Typed handlers** – register handlers conforming to the standard
  `dmhaman_handler_t` signature together with a user context pointer.
- **One-to-many dispatch** – multiple handlers may share the same name;
  `dmhaman_call_handler()` invokes **all** typed handlers registered under
  that name in registration order.
- **Named lookup** – retrieve a registered handler pointer by name.

## API

```c
typedef int (*dmhaman_handler_t)(void *parameters, void *user_ctx);

void dmhaman_init(void);
int  dmhaman_register_handler_generic(const char *name, void *handler);
int  dmhaman_register_handler(const char *name, dmhaman_handler_t handler, void *user_ctx);
int  dmhaman_call_handler(const char *name, void *parameters);
int  dmhaman_get_handler(const char *name, void **handler);
int  dmhaman_unregister_handler(const char *name);
```

## Usage

### Generic handler (plain storage)

```c
void my_irq_handler(void) { /* ... */ }

dmhaman_init();
dmhaman_register_handler_generic("uart1_irq", (void *)my_irq_handler);

/* Later, look up and call the handler directly */
void (*fn)(void);
dmhaman_get_handler("uart1_irq", (void **)&fn);
fn();
```

### Typed handler (with call support)

```c
int my_uart_handler(void *parameters, void *user_ctx)
{
    /* handle event */
    return 0;
}

dmhaman_init();
dmhaman_register_handler("uart1_event", my_uart_handler, NULL);

/* Dispatch all handlers registered for "uart1_event" */
dmhaman_call_handler("uart1_event", parameters);
```

### One-to-many dispatch

```c
dmhaman_register_handler("event", handler_a, ctx_a);
dmhaman_register_handler("event", handler_b, ctx_b);

/* Both handler_a and handler_b are called */
dmhaman_call_handler("event", parameters);
```

## Building

```sh
mkdir build && cd build
cmake .. -DDMOD_MODE=DMOD_MODULE
cmake --build .
```

## Running Tests

```sh
mkdir build-tests && cd build-tests
cmake .. -DDMOD_MODE=DMOD_SYSTEM -DDMHAMAN_BUILD_TESTS=ON
cmake --build .
ctest --output-on-failure
```

## Configuration

The maximum number of registry entries is controlled by the compile-time
constant `DMHAMAN_MAX_HANDLERS` (default: `64`).  Override it as needed:

```cmake
target_compile_definitions(my_target PRIVATE DMHAMAN_MAX_HANDLERS=128)
```

