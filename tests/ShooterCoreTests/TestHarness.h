#pragma once

#include <cstddef>
#include <cstdint>

#include <iostream>

namespace ShooterCore::Test {

inline std::uint32_t gFailureCount { 0U };

#define TEST_ASSERT(cond, msg)                                                                                         \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            std::cerr << "[FAIL] " << __FILE__ << ":" << __LINE__ << ": " << (msg) << "\n";                            \
            ++ShooterCore::Test::gFailureCount;                                                                        \
        }                                                                                                              \
    } while (false)

void runTypeTests() noexcept;
void runHitboxTests() noexcept;
void runTileMapTests() noexcept;
void runCollisionTests() noexcept;
void runTransformTests() noexcept;
void runWeaponTests() noexcept;
void runShieldTests() noexcept;

} // namespace ShooterCore::Test
