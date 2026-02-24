#include "dmhaman.h"
#include "dmlist.h"
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Distinguishes generic (untyped) from typed handler entries.
 */
typedef enum
{
    DMHAMAN_ENTRY_GENERIC = 0,  /**< Plain function-pointer storage */
    DMHAMAN_ENTRY_TYPED,        /**< Typed handler with standard signature */
} dmhaman_entry_type_t;

/**
 * @brief Internal registry entry (heap-allocated, stored in the dmlist).
 */
typedef struct
{
    const char            *name;
    dmhaman_entry_type_t   type;
    void                  *generic_handler;
    dmhaman_handler_t      handler;
    void                  *user_ctx;
} dmhaman_entry_t;

static dmlist_context_t *g_handler_list = NULL;

/* ------------------------------------------------------------------ */
/*  Internal comparison helpers                                         */
/* ------------------------------------------------------------------ */

static int compare_by_ptr(const void *a, const void *b)
{
    return (a == b) ? 0 : -1;
}

static int compare_typed_entry(const void *a, const void *b)
{
    const dmhaman_entry_t *ea = (const dmhaman_entry_t *)a;
    const dmhaman_entry_t *eb = (const dmhaman_entry_t *)b;
    if (ea->type != DMHAMAN_ENTRY_TYPED || eb->type != DMHAMAN_ENTRY_TYPED) return -1;
    if (ea->name == NULL || eb->name == NULL) return -1;
    if (strcmp(ea->name, eb->name) != 0) return -1;
    if (ea->handler != eb->handler) return -1;
    return 0;
}

static int compare_generic_entry(const void *a, const void *b)
{
    const dmhaman_entry_t *ea = (const dmhaman_entry_t *)a;
    const dmhaman_entry_t *eb = (const dmhaman_entry_t *)b;
    if (ea->type != DMHAMAN_ENTRY_GENERIC || eb->type != DMHAMAN_ENTRY_GENERIC) return -1;
    if (ea->name == NULL || eb->name == NULL) return -1;
    if (strcmp(ea->name, eb->name) != 0) return -1;
    if (ea->generic_handler != eb->generic_handler) return -1;
    return 0;
}

/* ------------------------------------------------------------------ */

void dmhaman_init(void)
{
    if (g_handler_list != NULL)
    {
        /* Free all entry data before destroying the list nodes */
        void *entry;
        while ((entry = dmlist_pop_front(g_handler_list)) != NULL)
        {
            Dmod_Free(entry);
        }
        dmlist_destroy(g_handler_list);
        g_handler_list = NULL;
    }
    g_handler_list = dmlist_create("dmhaman");
}

/* ------------------------------------------------------------------ */

int dmhaman_register_handler_generic(const char *name, void *handler)
{
    if (name == NULL || handler == NULL)
    {
        DMOD_LOG_ERROR("Invalid arguments to dmhaman_register_handler_generic\n");
        return -EINVAL;
    }
    if (g_handler_list == NULL)
    {
        DMOD_LOG_ERROR("Registry not initialized - call dmhaman_init first\n");
        return -EINVAL;
    }

    dmhaman_entry_t *entry = (dmhaman_entry_t *)Dmod_MallocEx(sizeof(dmhaman_entry_t), "dmhaman");
    if (entry == NULL)
    {
        DMOD_LOG_ERROR("Failed to allocate handler entry\n");
        return -ENOMEM;
    }

    entry->name            = name;
    entry->type            = DMHAMAN_ENTRY_GENERIC;
    entry->generic_handler = handler;
    entry->handler         = NULL;
    entry->user_ctx        = NULL;

    if (!dmlist_push_back(g_handler_list, entry))
    {
        Dmod_Free(entry);
        DMOD_LOG_ERROR("Failed to add handler to registry\n");
        return -ENOMEM;
    }

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
    if (g_handler_list == NULL)
    {
        DMOD_LOG_ERROR("Registry not initialized - call dmhaman_init first\n");
        return -EINVAL;
    }

    dmhaman_entry_t *entry = (dmhaman_entry_t *)Dmod_MallocEx(sizeof(dmhaman_entry_t), "dmhaman");
    if (entry == NULL)
    {
        DMOD_LOG_ERROR("Failed to allocate handler entry\n");
        return -ENOMEM;
    }

    entry->name            = name;
    entry->type            = DMHAMAN_ENTRY_TYPED;
    entry->generic_handler = NULL;
    entry->handler         = handler;
    entry->user_ctx        = user_ctx;

    if (!dmlist_push_back(g_handler_list, entry))
    {
        Dmod_Free(entry);
        DMOD_LOG_ERROR("Failed to add handler to registry\n");
        return -ENOMEM;
    }

    DMOD_LOG_INFO("Registered handler '%s'\n", name);
    return 0;
}

/* ------------------------------------------------------------------ */

typedef struct
{
    const char *name;
    void       *parameters;
    int         called;
    int         last_error;
} dmhaman_call_ctx_t;

static bool call_iterator(void *data, void *user_data)
{
    dmhaman_entry_t    *entry = (dmhaman_entry_t *)data;
    dmhaman_call_ctx_t *ctx   = (dmhaman_call_ctx_t *)user_data;

    if (entry->type == DMHAMAN_ENTRY_TYPED &&
        entry->name != NULL &&
        strcmp(entry->name, ctx->name) == 0)
    {
        if (entry->handler == NULL)
        {
            DMOD_LOG_ERROR("Null handler pointer for '%s'\n", entry->name);
            return true;
        }
        int ret = entry->handler(ctx->parameters, entry->user_ctx);
        if (ret != 0)
        {
            ctx->last_error = ret;
        }
        ctx->called++;
    }
    return true;
}

int dmhaman_call_handler(const char *name, void *parameters)
{
    if (name == NULL || g_handler_list == NULL)
    {
        DMOD_LOG_ERROR("Invalid arguments to dmhaman_call_handler\n");
        return -EINVAL;
    }

    dmhaman_call_ctx_t ctx = { .name = name, .parameters = parameters,
                                .called = 0, .last_error = 0 };
    dmlist_foreach(g_handler_list, call_iterator, &ctx);

    if (ctx.called == 0)
    {
        DMOD_LOG_ERROR("No typed handler found for '%s'\n", name);
        return -ENOENT;
    }

    return ctx.last_error;
}

/* ------------------------------------------------------------------ */

typedef struct
{
    const char *name;
    void      **out;
    bool        found;
} dmhaman_get_ctx_t;

static bool get_iterator(void *data, void *user_data)
{
    dmhaman_entry_t   *entry = (dmhaman_entry_t *)data;
    dmhaman_get_ctx_t *ctx   = (dmhaman_get_ctx_t *)user_data;

    if (entry->name != NULL && strcmp(entry->name, ctx->name) == 0)
    {
        if (entry->type == DMHAMAN_ENTRY_GENERIC)
        {
            *ctx->out = entry->generic_handler;
        }
        else
        {
            /* Casting function pointer to void* is implementation-defined
             * but supported on all targeted platforms. */
            *ctx->out = (void *)(uintptr_t)entry->handler;
        }
        ctx->found = true;
        return false; /* stop at first match */
    }
    return true;
}

int dmhaman_get_handler(const char *name, void **handler)
{
    if (name == NULL || handler == NULL)
    {
        DMOD_LOG_ERROR("Invalid arguments to dmhaman_get_handler\n");
        return -EINVAL;
    }
    if (g_handler_list == NULL)
    {
        DMOD_LOG_ERROR("Registry not initialized\n");
        return -EINVAL;
    }

    dmhaman_get_ctx_t ctx = { .name = name, .out = handler, .found = false };
    dmlist_foreach(g_handler_list, get_iterator, &ctx);

    if (!ctx.found)
    {
        DMOD_LOG_ERROR("Handler '%s' not found\n", name);
        return -ENOENT;
    }
    return 0;
}

/* ------------------------------------------------------------------ */

int dmhaman_unregister_handler(const char *name, dmhaman_handler_t handler)
{
    if (name == NULL || handler == NULL)
    {
        DMOD_LOG_ERROR("Invalid arguments to dmhaman_unregister_handler\n");
        return -EINVAL;
    }
    if (g_handler_list == NULL)
    {
        DMOD_LOG_ERROR("Registry not initialized\n");
        return -EINVAL;
    }

    dmhaman_entry_t key  = { .name = name, .type = DMHAMAN_ENTRY_TYPED,
                              .handler = handler };
    int             count = 0;
    dmhaman_entry_t *found;

    while ((found = (dmhaman_entry_t *)dmlist_find(g_handler_list, &key,
                                                    compare_typed_entry)) != NULL)
    {
        dmlist_remove(g_handler_list, found, compare_by_ptr);
        Dmod_Free(found);
        count++;
    }

    return count;
}

/* ------------------------------------------------------------------ */

int dmhaman_unregister_handler_generic(const char *name, void *handler)
{
    if (name == NULL || handler == NULL)
    {
        DMOD_LOG_ERROR("Invalid arguments to dmhaman_unregister_handler_generic\n");
        return -EINVAL;
    }
    if (g_handler_list == NULL)
    {
        DMOD_LOG_ERROR("Registry not initialized\n");
        return -EINVAL;
    }

    dmhaman_entry_t key  = { .name = name, .type = DMHAMAN_ENTRY_GENERIC,
                              .generic_handler = handler };
    int             count = 0;
    dmhaman_entry_t *found;

    while ((found = (dmhaman_entry_t *)dmlist_find(g_handler_list, &key,
                                                    compare_generic_entry)) != NULL)
    {
        dmlist_remove(g_handler_list, found, compare_by_ptr);
        Dmod_Free(found);
        count++;
    }

    return count;
}

