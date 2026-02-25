#ifndef DMHAMAN_H
#define DMHAMAN_H

#include <dmod.h>
#include "dmhaman_defs.h"

/**
 * @brief Standard handler function type.
 *
 * @param parameters Pointer to handler-specific parameters.
 * @param user_ctx   User context provided at registration time.
 * @return int 0 on success, non-zero on failure.
 */
typedef int (*dmhaman_handler_t)(void *parameters, void *user_ctx);

/**
 * @brief Initialise (or reset) the handler registry.
 *
 * Must be called before any other dmhaman function.  Safe to call again to
 * wipe all registered entries (e.g., during tests).
 */
dmod_dmhaman_api(1.0, void, _init, (void));

/**
 * @brief Register a generic (untyped) handler under a name.
 *
 * Stores an arbitrary function pointer associated with @p name.
 * The pointer can be retrieved later with dmhaman_get_handler().
 *
 * @param name    Name to register the handler under.
 * @param handler Pointer to the handler function.
 * @return int 0 on success, negative error code on failure.
 */
dmod_dmhaman_api(1.0, int, _register_handler_generic, (const char *name, void *handler));

/**
 * @brief Register a typed handler with a user context.
 *
 * Multiple handlers may share the same @p name (one-to-many).
 * All typed handlers registered under the same name are invoked by
 * dmhaman_call_handler().
 *
 * @param name     Name to register the handler under.
 * @param handler  Handler function matching dmhaman_handler_t.
 * @param user_ctx User context forwarded to the handler on each call.
 * @return int 0 on success, negative error code on failure.
 */
dmod_dmhaman_api(1.0, int, _register_handler, (const char *name, dmhaman_handler_t handler, void *user_ctx));

/**
 * @brief Call all typed handlers registered under @p name.
 *
 * Implements the one-to-many dispatch pattern: every typed handler whose
 * registered name matches @p name is called in registration order.
 *
 * @param name       Name of the handler(s) to invoke.
 * @param parameters Parameters forwarded to each handler.
 * @return int 0 if all handlers succeed; the last non-zero return value
 *         if any handler fails; negative error code if no handler found.
 */
dmod_dmhaman_api(1.0, int, _call_handler, (const char *name, void *parameters));

/**
 * @brief Look up the function pointer registered under @p name.
 *
 * Returns the pointer stored for the first matching entry regardless of
 * whether it was registered as generic or typed.
 *
 * @note Retrieving a typed (dmhaman_handler_t) handler as @c void* requires
 *       a function-to-object-pointer cast that is technically implementation-
 *       defined in ISO C.  On all supported embedded and host platforms this
 *       is well-defined.
 *
 * @param name Name to look up.
 * @return void* Function pointer on success, NULL if not found or @p name is NULL.
 */
dmod_dmhaman_api(1.0, void *, _get_handler, (const char *name));

/**
 * @brief Remove all typed entries registered under @p name with @p handler.
 *
 * Only entries that match both @p name **and** @p handler are removed.
 * This allows precise removal even when multiple handlers share the same name
 * (one-to-many registration).
 *
 * @param name    Name the handler was registered under.
 * @param handler Handler function pointer to remove.
 * @return int Number of entries removed on success, negative error code on
 *         failure.
 */
dmod_dmhaman_api(1.0, int, _unregister_handler, (const char *name, dmhaman_handler_t handler));

/**
 * @brief Remove all generic entries registered under @p name with @p handler.
 *
 * Only entries that match both @p name **and** @p handler are removed.
 *
 * @param name    Name the handler was registered under.
 * @param handler Generic (untyped) function pointer to remove.
 * @return int Number of entries removed on success, negative error code on
 *         failure.
 */
dmod_dmhaman_api(1.0, int, _unregister_handler_generic, (const char *name, void *handler));

#endif /* DMHAMAN_H */
