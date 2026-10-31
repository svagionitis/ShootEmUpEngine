#include "SarDecompressor.h"

#include <cstddef>
#include <cstdint>

#include <array>

namespace SdlAdapter {

namespace {

    class BitReader final {
    public:
        constexpr BitReader(const std::uint8_t* data, std::size_t len) noexcept
            : data_ { data }
            , len_ { len }
        {
        }

        [[nodiscard]] bool readBits(std::size_t n, std::uint16_t& outVal) noexcept
        {
            outVal = 0U;
            for (std::size_t i { 0U }; i < n; ++i) {
                const std::size_t byteIdx { (bitPos_ + i) / 8U };
                const std::size_t bitIdx { (bitPos_ + i) % 8U };
                if (byteIdx >= len_) {
                    return false;
                }
                const auto bit { static_cast<std::uint16_t>((data_[byteIdx] >> bitIdx) & 1U) };
                outVal = static_cast<std::uint16_t>(outVal | static_cast<std::uint16_t>(bit << i));
            }
            bitPos_ += n;
            return true;
        }

    private:
        const std::uint8_t* data_ { nullptr };
        std::size_t len_ { 0U };
        std::size_t bitPos_ { 0U };
    };

} // namespace

bool SarDecompressor::decompress(const std::uint8_t* inData, std::size_t inLen, std::uint8_t* outBuf,
    std::size_t outMax, std::size_t& outWritten) noexcept
{
    outWritten = 0U;
    if (inData == nullptr || inLen == 0U || outBuf == nullptr || outMax == 0U) {
        return false;
    }

    BitReader reader(inData, inLen);
    std::array<DictEntry, kMaxDictEntries> dict { };
    std::array<std::uint8_t, kMaxDictEntries> stack { };

    auto resetDict = [&]() noexcept -> std::size_t {
        for (std::size_t i { 0U }; i < 256U; ++i) {
            dict[i].prefix = static_cast<std::uint16_t>(kMaxDictEntries);
            dict[i].suffix = static_cast<std::uint8_t>(i);
        }
        return kFirstCode;
    };

    std::size_t nextCode { resetDict() };
    std::size_t codeSize { 9U };
    std::size_t nextBump { 512U };
    std::uint16_t oldCode { 0xFFFFU };

    std::uint16_t code { 0U };
    while (reader.readBits(codeSize, code)) {
        if (code == kEofCode) {
            break;
        }
        if (code == kClearCode) {
            nextCode = resetDict();
            codeSize = 9U;
            nextBump = 512U;
            oldCode = 0xFFFFU;
            continue;
        }

        std::uint16_t current { code };
        std::size_t stackPtr { 0U };

        if (oldCode == 0xFFFFU) {
            if (code >= 256U || outWritten >= outMax) {
                return false;
            }
            outBuf[outWritten++] = static_cast<std::uint8_t>(code);
            oldCode = code;
            continue;
        }

        if (current >= nextCode) {
            if (current > nextCode || stackPtr >= stack.size()) {
                return false;
            }
            std::uint16_t temp { oldCode };
            while (temp >= 256U && temp < nextCode) {
                temp = dict[temp].prefix;
            }
            stack[stackPtr++] = dict[temp].suffix;
            current = oldCode;
        }

        while (current >= 256U && current < nextCode) {
            if (stackPtr >= stack.size()) {
                return false;
            }
            stack[stackPtr++] = dict[current].suffix;
            current = dict[current].prefix;
        }
        if (current >= 256U || stackPtr >= stack.size()) {
            return false;
        }
        stack[stackPtr++] = dict[current].suffix;
        const std::uint8_t firstChar { dict[current].suffix };

        while (stackPtr > 0U) {
            if (outWritten >= outMax) {
                return false;
            }
            outBuf[outWritten++] = stack[--stackPtr];
        }

        if (nextCode < kMaxDictEntries) {
            dict[nextCode].prefix = oldCode;
            dict[nextCode].suffix = firstChar;
            ++nextCode;
            if (nextCode >= nextBump && codeSize < 12U) {
                ++codeSize;
                nextBump = static_cast<std::size_t>(1U << codeSize);
            }
        }

        oldCode = code;
    }

    return outWritten > 0U;
}

} // namespace SdlAdapter
