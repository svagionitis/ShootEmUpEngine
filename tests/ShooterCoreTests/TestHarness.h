#pragma once

#include <cstddef>
#include <cstdint>

#include <chrono>
#include <iostream>
#include <string_view>

#include <glog/logging.h>
#include <gtest/gtest.h>

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

/// @brief Primary assertion macro backed by GoogleTest and glog.
#define TEST_ASSERT(cond, msg)                                                                                         \
    do {                                                                                                               \
        ++ShooterCore::Test::gTestStats.totalAssertions;                                                               \
        const bool condVal_ = static_cast<bool>(cond);                                                                 \
        EXPECT_TRUE(condVal_) << (msg);                                                                                \
        if (!condVal_) {                                                                                               \
            LOG(ERROR) << "[FAIL] " << (msg) << " (Condition: " << #cond << ")";                                       \
            ++ShooterCore::Test::gTestStats.failedAssertions;                                                          \
            ++ShooterCore::Test::gFailureCount;                                                                        \
        }                                                                                                              \
    } while (false)

/// @brief Typed equality assertion macro backed by GoogleTest and glog.
#define TEST_ASSERT_EQ(actual, expected, msg)                                                                          \
    do {                                                                                                               \
        ++ShooterCore::Test::gTestStats.totalAssertions;                                                               \
        const auto actualVal_ = (actual);                                                                              \
        const auto expectedVal_ = (expected);                                                                          \
        EXPECT_EQ(actualVal_, expectedVal_) << (msg);                                                                  \
        if (!(actualVal_ == expectedVal_)) {                                                                           \
            LOG(ERROR) << "[FAIL] " << (msg) << " [Expected: " << expectedVal_ << ", Got: " << actualVal_ << "]";      \
            ++ShooterCore::Test::gTestStats.failedAssertions;                                                          \
            ++ShooterCore::Test::gFailureCount;                                                                        \
        }                                                                                                              \
    } while (false)

/// @brief Floating-point tolerance assertion macro backed by GoogleTest and glog.
#define TEST_ASSERT_NEAR(actual, expected, epsilon, msg)                                                               \
    do {                                                                                                               \
        ++ShooterCore::Test::gTestStats.totalAssertions;                                                               \
        const auto actualVal_ = (actual);                                                                              \
        const auto expectedVal_ = (expected);                                                                          \
        EXPECT_NEAR(actualVal_, expectedVal_, epsilon) << (msg);                                                       \
        const float diff_ = static_cast<float>(actualVal_ - expectedVal_);                                             \
        const float absDiff_ = (diff_ < 0.0f) ? -diff_ : diff_;                                                        \
        if (absDiff_ > static_cast<float>(epsilon)) {                                                                  \
            LOG(ERROR) << "[FAIL] " << (msg) << " [Diff: " << absDiff_ << " > Tol: " << (epsilon) << "]";              \
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
void runParticlePoolTests() noexcept;
void runGameEngineTests() noexcept;
void runAnimationTests() noexcept;
void runDeterminismSoakTests() noexcept;
void runSecondaryArsenalTests() noexcept;
void runEnemyAiTests() noexcept;
void runNestManagerTests() noexcept;
void runSupplyDropTests() noexcept;

} // namespace ShooterCore::Test
