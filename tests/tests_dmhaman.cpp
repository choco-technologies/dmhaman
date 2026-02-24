/**
 * @file tests_dmhaman.cpp
 * @brief Unit tests for the dmhaman handler registry
 */

#include <gtest/gtest.h>
#include <cstdint>

extern "C" {
#include "dmhaman.h"
}

// ------------------------------------------------------------------ //
//  Test fixtures and shared helpers
// ------------------------------------------------------------------ //

static int  g_call_count      = 0;
static void *g_last_parameters = nullptr;
static void *g_last_user_ctx   = nullptr;
static int   g_return_value    = 0;

static void reset_globals()
{
    g_call_count      = 0;
    g_last_parameters = nullptr;
    g_last_user_ctx   = nullptr;
    g_return_value    = 0;
}

static int test_handler_a(void *parameters, void *user_ctx)
{
    g_call_count++;
    g_last_parameters = parameters;
    g_last_user_ctx   = user_ctx;
    return g_return_value;
}

static int test_handler_b(void *parameters, void *user_ctx)
{
    g_call_count++;
    (void)parameters;
    (void)user_ctx;
    return g_return_value;
}

/**
 * @brief Base fixture: resets the global registry and counters before each test.
 */
class DmhamanTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        dmhaman_init();
        reset_globals();
    }
};

// ================================================================== //
//              dmhaman_init
// ================================================================== //

TEST_F(DmhamanTest, InitClearsRegistry)
{
    dmhaman_register_handler("evt", test_handler_a, nullptr);
    dmhaman_init();

    // After re-init, there should be nothing to call
    int result = dmhaman_call_handler("evt", nullptr);
    EXPECT_LT(result, 0);
}

// ================================================================== //
//              dmhaman_register_handler_generic
// ================================================================== //

TEST_F(DmhamanTest, RegisterGenericSuccess)
{
    int result = dmhaman_register_handler_generic("generic", (void *)test_handler_a);
    EXPECT_EQ(result, 0);
}

TEST_F(DmhamanTest, RegisterGenericNullName)
{
    int result = dmhaman_register_handler_generic(nullptr, (void *)test_handler_a);
    EXPECT_LT(result, 0);
}

TEST_F(DmhamanTest, RegisterGenericNullHandler)
{
    int result = dmhaman_register_handler_generic("generic", nullptr);
    EXPECT_LT(result, 0);
}

// ================================================================== //
//              dmhaman_register_handler
// ================================================================== //

TEST_F(DmhamanTest, RegisterTypedSuccess)
{
    int result = dmhaman_register_handler("evt", test_handler_a, nullptr);
    EXPECT_EQ(result, 0);
}

TEST_F(DmhamanTest, RegisterTypedNullName)
{
    int result = dmhaman_register_handler(nullptr, test_handler_a, nullptr);
    EXPECT_LT(result, 0);
}

TEST_F(DmhamanTest, RegisterTypedNullHandler)
{
    int result = dmhaman_register_handler("evt", nullptr, nullptr);
    EXPECT_LT(result, 0);
}

TEST_F(DmhamanTest, RegisterTypedWithUserCtx)
{
    int ctx    = 42;
    int result = dmhaman_register_handler("evt", test_handler_a, &ctx);
    EXPECT_EQ(result, 0);
}

// ================================================================== //
//              dmhaman_call_handler
// ================================================================== //

TEST_F(DmhamanTest, CallHandlerSuccess)
{
    dmhaman_register_handler("evt", test_handler_a, nullptr);
    int result = dmhaman_call_handler("evt", nullptr);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(g_call_count, 1);
}

TEST_F(DmhamanTest, CallHandlerPassesParameters)
{
    dmhaman_register_handler("evt", test_handler_a, nullptr);
    int params = 99;
    dmhaman_call_handler("evt", &params);
    EXPECT_EQ(g_last_parameters, &params);
}

TEST_F(DmhamanTest, CallHandlerPassesUserCtx)
{
    int ctx = 7;
    dmhaman_register_handler("evt", test_handler_a, &ctx);
    dmhaman_call_handler("evt", nullptr);
    EXPECT_EQ(g_last_user_ctx, &ctx);
}

TEST_F(DmhamanTest, CallHandlerNotFound)
{
    int result = dmhaman_call_handler("nonexistent", nullptr);
    EXPECT_LT(result, 0);
}

TEST_F(DmhamanTest, CallHandlerNullName)
{
    int result = dmhaman_call_handler(nullptr, nullptr);
    EXPECT_LT(result, 0);
}

TEST_F(DmhamanTest, CallHandlerOneToMany)
{
    dmhaman_register_handler("multi", test_handler_a, nullptr);
    dmhaman_register_handler("multi", test_handler_b, nullptr);

    int result = dmhaman_call_handler("multi", nullptr);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(g_call_count, 2);
}

TEST_F(DmhamanTest, CallHandlerReturnsLastError)
{
    g_return_value = -5;
    dmhaman_register_handler("evt", test_handler_a, nullptr);

    int result = dmhaman_call_handler("evt", nullptr);
    EXPECT_EQ(result, -5);
}

TEST_F(DmhamanTest, CallHandlerOneToManyPartialError)
{
    // handler_a returns error, handler_b succeeds (returns g_return_value = 0 after reset)
    g_return_value = -1;
    dmhaman_register_handler("multi", test_handler_a, nullptr);
    g_return_value = 0;
    dmhaman_register_handler("multi", test_handler_b, nullptr);

    // Both handlers are called despite partial failure
    g_return_value = -1; // both will return -1
    int result = dmhaman_call_handler("multi", nullptr);
    EXPECT_LT(result, 0);
    EXPECT_EQ(g_call_count, 2);
}

TEST_F(DmhamanTest, CallHandlerSkipsGenericEntries)
{
    // Generic entries should not be called via call_handler
    dmhaman_register_handler_generic("evt", (void *)test_handler_a);

    int result = dmhaman_call_handler("evt", nullptr);
    EXPECT_LT(result, 0);
    EXPECT_EQ(g_call_count, 0);
}

// ================================================================== //
//              dmhaman_get_handler
// ================================================================== //

TEST_F(DmhamanTest, GetHandlerGeneric)
{
    void *fn = (void *)(uintptr_t)test_handler_a;
    dmhaman_register_handler_generic("generic", fn);

    void *out = nullptr;
    int result = dmhaman_get_handler("generic", &out);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(out, fn);
}

TEST_F(DmhamanTest, GetHandlerTyped)
{
    dmhaman_register_handler("evt", test_handler_a, nullptr);

    void *out = nullptr;
    int result = dmhaman_get_handler("evt", &out);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(out, (void *)(uintptr_t)test_handler_a);
}

TEST_F(DmhamanTest, GetHandlerNotFound)
{
    void *out  = nullptr;
    int result = dmhaman_get_handler("nonexistent", &out);
    EXPECT_LT(result, 0);
}

TEST_F(DmhamanTest, GetHandlerNullName)
{
    void *out  = nullptr;
    int result = dmhaman_get_handler(nullptr, &out);
    EXPECT_LT(result, 0);
}

TEST_F(DmhamanTest, GetHandlerNullOutput)
{
    dmhaman_register_handler_generic("generic", (void *)test_handler_a);
    int result = dmhaman_get_handler("generic", nullptr);
    EXPECT_LT(result, 0);
}

TEST_F(DmhamanTest, GetHandlerReturnsFirst)
{
    // Register two handlers with the same name; get_handler should return the first
    dmhaman_register_handler("evt", test_handler_a, nullptr);
    dmhaman_register_handler("evt", test_handler_b, nullptr);

    void *out  = nullptr;
    int result = dmhaman_get_handler("evt", &out);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(out, (void *)(uintptr_t)test_handler_a);
}

// ================================================================== //
//              dmhaman_unregister_handler
// ================================================================== //

TEST_F(DmhamanTest, UnregisterSingleHandler)
{
    dmhaman_register_handler("evt", test_handler_a, nullptr);

    int count = dmhaman_unregister_handler("evt", test_handler_a);
    EXPECT_EQ(count, 1);

    // No longer callable
    EXPECT_LT(dmhaman_call_handler("evt", nullptr), 0);
}

TEST_F(DmhamanTest, UnregisterRemovesAllMatchingEntries)
{
    // Same handler registered twice under same name
    dmhaman_register_handler("multi", test_handler_a, nullptr);
    dmhaman_register_handler("multi", test_handler_a, nullptr);

    int count = dmhaman_unregister_handler("multi", test_handler_a);
    EXPECT_EQ(count, 2);

    EXPECT_LT(dmhaman_call_handler("multi", nullptr), 0);
}

TEST_F(DmhamanTest, UnregisterOnlyRemovesMatchingHandler)
{
    // Two different handlers under the same name
    dmhaman_register_handler("multi", test_handler_a, nullptr);
    dmhaman_register_handler("multi", test_handler_b, nullptr);

    // Unregister only handler_a; handler_b must still be callable
    int count = dmhaman_unregister_handler("multi", test_handler_a);
    EXPECT_EQ(count, 1);

    int result = dmhaman_call_handler("multi", nullptr);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(g_call_count, 1);
}

TEST_F(DmhamanTest, UnregisterNullName)
{
    int result = dmhaman_unregister_handler(nullptr, test_handler_a);
    EXPECT_LT(result, 0);
}

TEST_F(DmhamanTest, UnregisterNullHandler)
{
    int result = dmhaman_unregister_handler("evt", nullptr);
    EXPECT_LT(result, 0);
}

TEST_F(DmhamanTest, UnregisterNonexistentHandler)
{
    int count = dmhaman_unregister_handler("nonexistent", test_handler_a);
    EXPECT_EQ(count, 0);
}

TEST_F(DmhamanTest, UnregisterLeavesOtherHandlersIntact)
{
    dmhaman_register_handler("evtA", test_handler_a, nullptr);
    dmhaman_register_handler("evtB", test_handler_b, nullptr);

    dmhaman_unregister_handler("evtA", test_handler_a);

    // evtB must still work
    int result = dmhaman_call_handler("evtB", nullptr);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(g_call_count, 1);
}

// ================================================================== //
//              dmhaman_unregister_handler_generic
// ================================================================== //

TEST_F(DmhamanTest, UnregisterGenericHandler)
{
    dmhaman_register_handler_generic("generic", (void *)test_handler_a);

    int count = dmhaman_unregister_handler_generic("generic", (void *)test_handler_a);
    EXPECT_EQ(count, 1);

    void *out  = nullptr;
    int result = dmhaman_get_handler("generic", &out);
    EXPECT_LT(result, 0);
}

TEST_F(DmhamanTest, UnregisterGenericNullName)
{
    int result = dmhaman_unregister_handler_generic(nullptr, (void *)test_handler_a);
    EXPECT_LT(result, 0);
}

TEST_F(DmhamanTest, UnregisterGenericNullHandler)
{
    int result = dmhaman_unregister_handler_generic("generic", nullptr);
    EXPECT_LT(result, 0);
}

TEST_F(DmhamanTest, UnregisterGenericOnlyRemovesMatchingHandler)
{
    dmhaman_register_handler_generic("g1", (void *)test_handler_a);
    dmhaman_register_handler_generic("g2", (void *)test_handler_b);

    dmhaman_unregister_handler_generic("g1", (void *)test_handler_a);

    // g2 must still be retrievable
    void *out  = nullptr;
    int result = dmhaman_get_handler("g2", &out);
    EXPECT_EQ(result, 0);
    EXPECT_EQ(out, (void *)(uintptr_t)test_handler_b);
}
