#pragma once

#include <cstdint>
#include <string_view>

#include <glog/logging.h>

namespace ShooterCore {

/// @brief Severity level for engine diagnostics.
enum class LogLevel : std::uint8_t { Debug = 0U, Info = 1U, Warning = 2U, Error = 3U };

/// @class Logger
/// @brief Core logging utility providing structured diagnostics for ShooterCore subsystems.
class Logger final {
public:
    Logger() = delete;

    /// @brief Log an informational diagnostic message.
    /// @param[in] tag Subsystem or category identifier.
    /// @param[in] message Diagnostic message.
    static void info(std::string_view tag, std::string_view message) noexcept;

    /// @brief Log a warning diagnostic message.
    /// @param[in] tag Subsystem or category identifier.
    /// @param[in] message Warning message.
    static void warn(std::string_view tag, std::string_view message) noexcept;

    /// @brief Log an error diagnostic message.
    /// @param[in] tag Subsystem or category identifier.
    /// @param[in] message Error description.
    static void error(std::string_view tag, std::string_view message) noexcept;
};

} // namespace ShooterCore
