# dmhaman API Reference

## Overview

`dmhaman` is a DMOD library module providing a central name→function-pointer
registry.  It supports two handler modes and one-to-many dispatch.

---

## Types

### `dmhaman_handler_t`

```c
typedef int (*dmhaman_handler_t)(void *parameters, void *user_ctx);
```

Standard handler function signature used for typed registration and dispatch.

| Parameter    | Description |
|---|---|
| `parameters` | Caller-supplied parameters forwarded on each dispatch. |
| `user_ctx`   | User context supplied at registration time. |

**Returns** `0` on success, non-zero on failure.

---

## Functions

### `dmhaman_init`

```c
void dmhaman_init(void);
```

Initialises (or resets) the handler registry.

Called automatically by `dmod_init` when the DMOD runtime enables the module.
Can be called again to wipe all registered entries (e.g., during tests).

---

### `dmhaman_register_handler_generic`

```c
int dmhaman_register_handler_generic(const char *name, void *handler);
```

Registers an arbitrary function pointer under `name`.

The pointer can be retrieved later with `dmhaman_get_handler()`.
Multiple entries may share the same name.

| Parameter | Description |
|---|---|
| `name`    | Registration key. Must not be `NULL`. |
| `handler` | Function pointer to store. Must not be `NULL`. |

**Returns** `0` on success, `-EINVAL` if arguments are invalid, `-ENOMEM` if
allocation fails.

---

### `dmhaman_register_handler`

```c
int dmhaman_register_handler(const char *name,
                              dmhaman_handler_t handler,
                              void *user_ctx);
```

Registers a typed handler together with a user context.

Multiple handlers may share the same `name` (one-to-many). All typed handlers
registered under the same name are invoked by `dmhaman_call_handler()`.

| Parameter  | Description |
|---|---|
| `name`     | Registration key. Must not be `NULL`. |
| `handler`  | Handler function. Must not be `NULL`. |
| `user_ctx` | Opaque pointer forwarded to the handler on every call. |

**Returns** `0` on success, `-EINVAL` if arguments are invalid, `-ENOMEM` if
allocation fails.

---

### `dmhaman_call_handler`

```c
int dmhaman_call_handler(const char *name, void *parameters);
```

Calls **all** typed handlers registered under `name` in registration order
(one-to-many dispatch).

| Parameter    | Description |
|---|---|
| `name`       | Name of the handler(s) to invoke. Must not be `NULL`. |
| `parameters` | Forwarded to each handler as the first argument. |

**Returns**
- `0` if all handlers return `0`.
- The last non-zero return value if any handler fails.
- `-ENOENT` if no typed handler is registered under `name`.
- `-EINVAL` if `name` is `NULL` or the registry is not initialised.

> **Note** Generic entries registered via `dmhaman_register_handler_generic`
> are **not** invoked by this function.

---

### `dmhaman_get_handler`

```c
int dmhaman_get_handler(const char *name, void **handler);
```

Returns the function pointer stored for the **first** entry whose name
matches `name`, regardless of entry type (generic or typed).

| Parameter | Description |
|---|---|
| `name`    | Name to look up. Must not be `NULL`. |
| `handler` | Output: receives the function pointer. Must not be `NULL`. |

**Returns** `0` on success, `-ENOENT` if not found, `-EINVAL` on bad
arguments.

---

### `dmhaman_unregister_handler`

```c
int dmhaman_unregister_handler(const char *name,
                                dmhaman_handler_t handler);
```

Removes all typed entries that match both `name` **and** `handler`.

This allows precise removal even when multiple handlers share the same name.

| Parameter | Description |
|---|---|
| `name`    | Name the handler was registered under. Must not be `NULL`. |
| `handler` | Handler pointer to remove. Must not be `NULL`. |

**Returns** number of entries removed (≥ 0), or `-EINVAL` on bad arguments.

---

### `dmhaman_unregister_handler_generic`

```c
int dmhaman_unregister_handler_generic(const char *name, void *handler);
```

Removes all generic entries that match both `name` **and** `handler`.

| Parameter | Description |
|---|---|
| `name`    | Name the handler was registered under. Must not be `NULL`. |
| `handler` | Generic function pointer to remove. Must not be `NULL`. |

**Returns** number of entries removed (≥ 0), or `-EINVAL` on bad arguments.

---

## DMOD Lifecycle

| Function       | Called by DMOD when |
|---|---|
| `dmod_init`    | Module is enabled — initialises the registry. |
| `dmod_deinit`  | Module is disabled — frees all entries and destroys the list. |

---

## Error Codes

| Code       | Meaning |
|---|---|
| `0`        | Success |
| `-EINVAL`  | Invalid argument (NULL pointer or registry not initialised) |
| `-ENOMEM`  | Memory allocation failed |
| `-ENOENT`  | No matching entry found |

---

## See Also

- [Main README](../README.md)
- [`dmlist`](https://github.com/choco-technologies/dmlist) — underlying
  doubly-linked list used by the registry
