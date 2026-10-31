#pragma once

#include <cstddef>
#include <cstdint>

#include <chrono>
#include <iostream>
#include <string_view>

#include <glog/logging.h>
#include <gtest/gtest.h>

namespace SdlAdapter::Test {

/// @brief Global test statistics accumulator.
struct TestStats {
    std::uint32_t totalAssertions { 0U };
    std::uint32_t failedAssertions { 0U };
    std::uint32_t passedSuites { 0U };
    std::uint32_t failedSuites { 0U };
};

inline TestStats gTestStats { };
inline std::uint32_t gFailureCount { 0U };

/// @brief Primary assertion macro backed by GoogleTest and glog.
#define TEST_ASSERT(cond, msg)                                                                                         \
    do {                                                                                                               \
        ++SdlAdapter::Test::gTestStats.totalAssertions;                                                                \
        const bool condVal_ = static_cast<bool>(cond);                                                                 \
        EXPECT_TRUE(condVal_) << (msg);                                                                                \
        if (!condVal_) {                                                                                               \
            LOG(ERROR) << "[FAIL] " << (msg) << " (Condition: " << #cond << ")";                                       \
            ++SdlAdapter::Test::gTestStats.failedAssertions;                                                           \
            ++SdlAdapter::Test::gFailureCount;                                                                         \
        }                                                                                                              \
    } while (false)

/// @brief Typed equality assertion macro backed by GoogleTest and glog.
#define TEST_ASSERT_EQ(actual, expected, msg)                                                                          \
    do {                                                                                                               \
        ++SdlAdapter::Test::gTestStats.totalAssertions;                                                                \
        const auto actualVal_ = (actual);                                                                              \
        const auto expectedVal_ = (expected);                                                                          \
        EXPECT_EQ(actualVal_, expectedVal_) << (msg);                                                                  \
        if (!(actualVal_ == expectedVal_)) {                                                                           \
            LOG(ERROR) << "[FAIL] " << (msg) << " [Expected: " << expectedVal_ << ", Got: " << actualVal_ << "]";      \
            ++SdlAdapter::Test::gTestStats.failedAssertions;                                                           \
            ++SdlAdapter::Test::gFailureCount;                                                                         \
        }                                                                                                              \
    } while (false)

/// @brief Floating-point tolerance assertion macro backed by GoogleTest and glog.
#define TEST_ASSERT_NEAR(actual, expected, epsilon, msg)                                                               \
    do {                                                                                                               \
        ++SdlAdapter::Test::gTestStats.totalAssertions;                                                                \
        const auto actualVal_ = (actual);                                                                              \
        const auto expectedVal_ = (expected);                                                                          \
        EXPECT_NEAR(actualVal_, expectedVal_, epsilon) << (msg);                                                       \
        const float diff_ = static_cast<float>(actualVal_ - expectedVal_);                                             \
        const float absDiff_ = (diff_ < 0.0f) ? -diff_ : diff_;                                                        \
        if (absDiff_ > static_cast<float>(epsilon)) {                                                                  \
            LOG(ERROR) << "[FAIL] " << (msg) << " [Diff: " << absDiff_ << " > Tol: " << (epsilon) << "]";              \
            ++SdlAdapter::Test::gTestStats.failedAssertions;                                                           \
            ++SdlAdapter::Test::gFailureCount;                                                                         \
        }                                                                                                              \
    } while (false)

#define TEST_ASSERT_TRUE(cond, msg) TEST_ASSERT((cond), msg)
#define TEST_ASSERT_FALSE(cond, msg) TEST_ASSERT(!(cond), msg)

void runSdlWindowTests() noexcept;
void runSdlInputTests() noexcept;
void runTimePacerTests() noexcept;
void runSdlRendererTests() noexcept;
void runSdlAudioTests() noexcept;
void runAdsrEnvelopeTests() noexcept;
void runMusicTrackerTests() noexcept;

} // namespace SdlAdapter::Test
