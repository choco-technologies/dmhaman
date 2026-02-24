#include "dmhaman.h"
#include <string.h>
#include <errno.h>

#ifndef DMHAMAN_MAX_HANDLERS
/**
 * @brief Maximum number of handler entries in the registry.
 *
 * Can be overridden at compile time, e.g.:
 *   target_compile_definitions(my_target PRIVATE DMHAMAN_MAX_HANDLERS=128)
 */
#define DMHAMAN_MAX_HANDLERS    64
#endif

/**
 * @brief Distinguishes generic (untyped) from typed handler entries.
 */
typedef enum
{
    DMHAMAN_ENTRY_GENERIC = 0,  /**< Plain function-pointer storage */
    DMHAMAN_ENTRY_TYPED,        /**< Typed handler with standard signature */
} dmhaman_entry_type_t;

/**
 * @brief Internal registry entry.
 */
typedef struct
{
    const char            *name;
    dmhaman_entry_type_t   type;
    void                  *generic_handler;
    dmhaman_handler_t      handler;
    void                  *user_ctx;
} dmhaman_entry_t;

static dmhaman_entry_t g_handlers[DMHAMAN_MAX_HANDLERS];
static int             g_handler_count = 0;

/* ------------------------------------------------------------------ */

void dmhaman_init(void)
{
    memset(g_handlers, 0, sizeof(g_handlers));
    g_handler_count = 0;
}

/* ------------------------------------------------------------------ */

int dmhaman_register_handler_generic(const char *name, void *handler)
{
    if (name == NULL || handler == NULL)
    {
        DMOD_LOG_ERROR("Invalid arguments to dmhaman_register_handler_generic\n");
        return -EINVAL;
    }

    if (g_handler_count >= DMHAMAN_MAX_HANDLERS)
    {
        DMOD_LOG_ERROR("Handler registry is full\n");
        return -ENOMEM;
    }

    g_handlers[g_handler_count].name            = name;
    g_handlers[g_handler_count].type            = DMHAMAN_ENTRY_GENERIC;
    g_handlers[g_handler_count].generic_handler = handler;
    g_handlers[g_handler_count].handler         = NULL;
    g_handlers[g_handler_count].user_ctx        = NULL;
    g_handler_count++;

    DMOD_LOG_INFO("Registered generic handler '%s'\n", name);
    return 0;
}

/* ------------------------------------------------------------------ */

int dmhaman_register_handler(const char *name, dmhaman_handler_t handler, void *user_ctx)
{
    if (name == NULL || handler == NULL)
    {
        DMOD_LOG_ERROR("Invalid arguments to dmhaman_register_handler\n");
        return -EINVAL;
    }

    if (g_handler_count >= DMHAMAN_MAX_HANDLERS)
    {
        DMOD_LOG_ERROR("Handler registry is full\n");
        return -ENOMEM;
    }

    g_handlers[g_handler_count].name            = name;
    g_handlers[g_handler_count].type            = DMHAMAN_ENTRY_TYPED;
    g_handlers[g_handler_count].generic_handler = NULL;
    g_handlers[g_handler_count].handler         = handler;
    g_handlers[g_handler_count].user_ctx        = user_ctx;
    g_handler_count++;

    DMOD_LOG_INFO("Registered handler '%s'\n", name);
    return 0;
}

/* ------------------------------------------------------------------ */

int dmhaman_call_handler(const char *name, void *parameters)
{
    if (name == NULL)
    {
        DMOD_LOG_ERROR("Invalid arguments to dmhaman_call_handler\n");
        return -EINVAL;
    }

    int called     = 0;
    int last_error = 0;

    for (int i = 0; i < g_handler_count; i++)
    {
        if (g_handlers[i].name != NULL &&
            g_handlers[i].type == DMHAMAN_ENTRY_TYPED &&
            strcmp(g_handlers[i].name, name) == 0)
        {
            int ret = g_handlers[i].handler(parameters, g_handlers[i].user_ctx);
            if (ret != 0)
            {
                last_error = ret;
            }
            called++;
        }
    }

    if (called == 0)
    {
        DMOD_LOG_ERROR("No typed handler found for '%s'\n", name);
        return -ENOENT;
    }

    return last_error;
}

/* ------------------------------------------------------------------ */

int dmhaman_get_handler(const char *name, void **handler)
{
    if (name == NULL || handler == NULL)
    {
        DMOD_LOG_ERROR("Invalid arguments to dmhaman_get_handler\n");
        return -EINVAL;
    }

    for (int i = 0; i < g_handler_count; i++)
    {
        if (g_handlers[i].name != NULL && strcmp(g_handlers[i].name, name) == 0)
        {
            if (g_handlers[i].type == DMHAMAN_ENTRY_GENERIC)
            {
                *handler = g_handlers[i].generic_handler;
            }
            else
            {
                /* Casting function pointer to void* is implementation-defined
                 * but supported on all targeted platforms. */
                *handler = (void *)(uintptr_t)g_handlers[i].handler;
            }
            return 0;
        }
    }

    DMOD_LOG_ERROR("Handler '%s' not found\n", name);
    return -ENOENT;
}

/* ------------------------------------------------------------------ */

int dmhaman_unregister_handler(const char *name)
{
    if (name == NULL)
    {
        DMOD_LOG_ERROR("Invalid arguments to dmhaman_unregister_handler\n");
        return -EINVAL;
    }

    int count = 0;
    int i     = 0;

    while (i < g_handler_count)
    {
        if (g_handlers[i].name != NULL && strcmp(g_handlers[i].name, name) == 0)
        {
            /* Shift remaining entries down to close the gap. */
            for (int j = i; j < g_handler_count - 1; j++)
            {
                g_handlers[j] = g_handlers[j + 1];
            }
            memset(&g_handlers[g_handler_count - 1], 0, sizeof(dmhaman_entry_t));
            g_handler_count--;
            count++;
            /* Do not advance i: re-check the same slot after the shift. */
        }
        else
        {
            i++;
        }
    }

    return count;
}
