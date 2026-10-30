#pragma once

#include <cstddef>
#include <cstdint>

#include <chrono>
#include <iostream>
#include <string_view>

namespace ShooterCore::Test {

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
        ++ShooterCore::Test::gTestStats.totalAssertions;                                                               \
        if (!(cond)) {                                                                                                 \
            std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ << ": " << (msg) << " (Condition: " << #cond         \
                      << ")\n";                                                                                        \
            ++ShooterCore::Test::gTestStats.failedAssertions;                                                          \
            ++ShooterCore::Test::gFailureCount;                                                                        \
        }                                                                                                              \
    } while (false)

/// @brief Typed equality assertion macro.
#define TEST_ASSERT_EQ(actual, expected, msg)                                                                          \
    do {                                                                                                               \
        ++ShooterCore::Test::gTestStats.totalAssertions;                                                               \
        if (!((actual) == (expected))) {                                                                               \
            std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ << ": " << (msg) << " [Expected: " << (expected)     \
                      << ", Got: " << (actual) << "]\n";                                                               \
            ++ShooterCore::Test::gTestStats.failedAssertions;                                                          \
            ++ShooterCore::Test::gFailureCount;                                                                        \
        }                                                                                                              \
    } while (false)

/// @brief Floating-point tolerance assertion macro.
#define TEST_ASSERT_NEAR(actual, expected, epsilon, msg)                                                               \
    do {                                                                                                               \
        ++ShooterCore::Test::gTestStats.totalAssertions;                                                               \
        const float diff_ = static_cast<float>((actual) - (expected));                                                 \
        const float absDiff_ = (diff_ < 0.0f) ? -diff_ : diff_;                                                        \
        if (absDiff_ > static_cast<float>(epsilon)) {                                                                  \
            std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ << ": " << (msg) << " [Diff: " << absDiff_           \
                      << " > Tol: " << (epsilon) << "]\n";                                                             \
            ++ShooterCore::Test::gTestStats.failedAssertions;                                                          \
            ++ShooterCore::Test::gFailureCount;                                                                        \
        }                                                                                                              \
    } while (false)

#define TEST_ASSERT_TRUE(cond, msg) TEST_ASSERT((cond), msg)
#define TEST_ASSERT_FALSE(cond, msg) TEST_ASSERT(!(cond), msg)

/// @brief Benchmark timer for test execution.
class ScopedTimer {
public:
    explicit ScopedTimer(std::string_view name) noexcept
        : name_ { name }
        , start_ { std::chrono::high_resolution_clock::now() }
    {
    }

    ~ScopedTimer() noexcept
    {
        const auto end = std::chrono::high_resolution_clock::now();
        const auto durationUs = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        std::cout << "[BENCH] " << name_ << ": " << (static_cast<double>(durationUs) / 1000.0) << " ms\n";
    }

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer&&) = delete;
    ScopedTimer& operator=(ScopedTimer&&) = delete;

private:
    std::string_view name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

void runTypeTests() noexcept;
void runHitboxTests() noexcept;
void runTileMapTests() noexcept;
void runCollisionTests() noexcept;
void runTransformTests() noexcept;
void runWeaponTests() noexcept;
void runShieldTests() noexcept;
void runEngineInputTests() noexcept;
void runCommandQueueTests() noexcept;
void runEntityPoolTests() noexcept;
void runGameEngineTests() noexcept;
void runDeterminismSoakTests() noexcept;

} // namespace ShooterCore::Test
