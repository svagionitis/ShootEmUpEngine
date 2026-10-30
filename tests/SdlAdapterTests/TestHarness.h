#pragma once

#include <cstddef>
#include <cstdint>

#include <chrono>
#include <iostream>
#include <string_view>

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

/// @brief Primary assertion macro.
#define TEST_ASSERT(cond, msg)                                                                                         \
    do {                                                                                                               \
        ++SdlAdapter::Test::gTestStats.totalAssertions;                                                                \
        if (!(cond)) {                                                                                                 \
            std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ << ": " << (msg) << " (Condition: " << #cond         \
                      << ")\n";                                                                                        \
            ++SdlAdapter::Test::gTestStats.failedAssertions;                                                           \
            ++SdlAdapter::Test::gFailureCount;                                                                         \
        }                                                                                                              \
    } while (false)

/// @brief Typed equality assertion macro.
#define TEST_ASSERT_EQ(actual, expected, msg)                                                                          \
    do {                                                                                                               \
        ++SdlAdapter::Test::gTestStats.totalAssertions;                                                                \
        if (!((actual) == (expected))) {                                                                               \
            std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ << ": " << (msg) << " [Expected: " << (expected)     \
                      << ", Got: " << (actual) << "]\n";                                                               \
            ++SdlAdapter::Test::gTestStats.failedAssertions;                                                           \
            ++SdlAdapter::Test::gFailureCount;                                                                         \
        }                                                                                                              \
    } while (false)

/// @brief Floating-point tolerance assertion macro.
#define TEST_ASSERT_NEAR(actual, expected, epsilon, msg)                                                               \
    do {                                                                                                               \
        ++SdlAdapter::Test::gTestStats.totalAssertions;                                                                \
        const float diff_ = static_cast<float>((actual) - (expected));                                                 \
        const float absDiff_ = (diff_ < 0.0f) ? -diff_ : diff_;                                                        \
        if (absDiff_ > static_cast<float>(epsilon)) {                                                                  \
            std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ << ": " << (msg) << " [Diff: " << absDiff_           \
                      << " > Tol: " << (epsilon) << "]\n";                                                             \
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

} // namespace SdlAdapter::Test
