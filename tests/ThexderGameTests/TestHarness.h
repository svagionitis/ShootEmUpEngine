#pragma once

#include <cstddef>
#include <cstdint>

#include <glog/logging.h>
#include <gtest/gtest.h>

namespace ThexderGame::Test {

struct TestStats {
    std::uint32_t totalAssertions { 0U };
    std::uint32_t failedAssertions { 0U };
};

inline TestStats gTestStats { };
inline std::uint32_t gFailureCount { 0U };

#define TEST_ASSERT(cond, msg)                                                                                         \
    do {                                                                                                               \
        ++ThexderGame::Test::gTestStats.totalAssertions;                                                               \
        const bool condVal_ = static_cast<bool>(cond);                                                                 \
        EXPECT_TRUE(condVal_) << (msg);                                                                                \
        if (!condVal_) {                                                                                               \
            LOG(ERROR) << "[FAIL] " << (msg) << " (Condition: " << #cond << ")";                                       \
            ++ThexderGame::Test::gTestStats.failedAssertions;                                                          \
            ++ThexderGame::Test::gFailureCount;                                                                        \
        }                                                                                                              \
    } while (false)

#define TEST_ASSERT_EQ(actual, expected, msg)                                                                          \
    do {                                                                                                               \
        ++ThexderGame::Test::gTestStats.totalAssertions;                                                               \
        const auto actualVal_ = (actual);                                                                              \
        const auto expectedVal_ = (expected);                                                                          \
        EXPECT_EQ(actualVal_, expectedVal_) << (msg);                                                                  \
        if (!(actualVal_ == expectedVal_)) {                                                                           \
            LOG(ERROR) << "[FAIL] " << (msg) << " [Expected: " << expectedVal_ << ", Got: " << actualVal_ << "]";      \
            ++ThexderGame::Test::gTestStats.failedAssertions;                                                          \
            ++ThexderGame::Test::gFailureCount;                                                                        \
        }                                                                                                              \
    } while (false)

void runLevel1MapTests();
void runHudOverlayTests();
void runEntitySpawnerTests();
void runGameStateTests();

} // namespace ThexderGame::Test
