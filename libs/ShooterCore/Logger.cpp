#include "Logger.h"

#include <cstdint>
#include <string_view>

#include <glog/logging.h>

namespace ShooterCore {

void Logger::info(std::string_view tag, std::string_view message) noexcept
{
    LOG(INFO) << "[" << tag << "] " << message;
}

void Logger::warn(std::string_view tag, std::string_view message) noexcept
{
    LOG(WARNING) << "[" << tag << "] " << message;
}

void Logger::error(std::string_view tag, std::string_view message) noexcept
{
    LOG(ERROR) << "[" << tag << "] " << message;
}

} // namespace ShooterCore
