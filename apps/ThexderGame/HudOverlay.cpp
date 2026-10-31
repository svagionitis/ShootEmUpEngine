#include "HudOverlay.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>
#include <string_view>

#include "RenderCommandQueue.h"
#include "Types.h"

namespace ThexderGame {

namespace {

    [[nodiscard]] constexpr char toUpperAscii(char c) noexcept
    {
        if (c >= 'a' && c <= 'z') {
            return static_cast<char>(c - 'a' + 'A');
        }
        return c;
    }

    // 5x7 ASCII bitmap font data for ASCII 32 (' ') to 93 (']')
    constexpr std::uint8_t kFontData[62][5] {
        { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U }, // 32 ' '
        { 0x00U, 0x00U, 0x5FU, 0x00U, 0x00U }, // 33 '!'
        { 0x00U, 0x07U, 0x00U, 0x07U, 0x00U }, // 34 '"'
        { 0x14U, 0x7FU, 0x14U, 0x7FU, 0x14U }, // 35 '#'
        { 0x24U, 0x2AU, 0x7FU, 0x2AU, 0x12U }, // 36 '$'
        { 0x23U, 0x13U, 0x08U, 0x64U, 0x62U }, // 37 '%'
        { 0x36U, 0x49U, 0x55U, 0x22U, 0x50U }, // 38 '&'
        { 0x00U, 0x05U, 0x03U, 0x00U, 0x00U }, // 39 '''
        { 0x00U, 0x1CU, 0x22U, 0x41U, 0x00U }, // 40 '('
        { 0x00U, 0x41U, 0x22U, 0x1CU, 0x00U }, // 41 ')'
        { 0x14U, 0x08U, 0x3EU, 0x08U, 0x14U }, // 42 '*'
        { 0x08U, 0x08U, 0x3EU, 0x08U, 0x08U }, // 43 '+'
        { 0x00U, 0x50U, 0x30U, 0x00U, 0x00U }, // 44 ','
        { 0x08U, 0x08U, 0x08U, 0x08U, 0x08U }, // 45 '-'
        { 0x00U, 0x60U, 0x60U, 0x00U, 0x00U }, // 46 '.'
        { 0x20U, 0x10U, 0x08U, 0x04U, 0x02U }, // 47 '/'
        { 0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU }, // 48 '0'
        { 0x00U, 0x42U, 0x7FU, 0x40U, 0x00U }, // 49 '1'
        { 0x42U, 0x61U, 0x51U, 0x49U, 0x46U }, // 50 '2'
        { 0x21U, 0x41U, 0x45U, 0x4BU, 0x31U }, // 51 '3'
        { 0x18U, 0x14U, 0x12U, 0x7FU, 0x10U }, // 52 '4'
        { 0x27U, 0x45U, 0x45U, 0x45U, 0x39U }, // 53 '5'
        { 0x3CU, 0x4AU, 0x49U, 0x49U, 0x30U }, // 54 '6'
        { 0x01U, 0x71U, 0x09U, 0x05U, 0x03U }, // 55 '7'
        { 0x36U, 0x49U, 0x49U, 0x49U, 0x36U }, // 56 '8'
        { 0x06U, 0x49U, 0x49U, 0x29U, 0x1EU }, // 57 '9'
        { 0x00U, 0x36U, 0x36U, 0x00U, 0x00U }, // 58 ':'
        { 0x00U, 0x56U, 0x36U, 0x00U, 0x00U }, // 59 ';'
        { 0x08U, 0x14U, 0x22U, 0x41U, 0x00U }, // 60 '<'
        { 0x14U, 0x14U, 0x14U, 0x14U, 0x14U }, // 61 '='
        { 0x00U, 0x41U, 0x22U, 0x14U, 0x08U }, // 62 '>'
        { 0x02U, 0x01U, 0x51U, 0x09U, 0x06U }, // 63 '?'
        { 0x32U, 0x49U, 0x79U, 0x41U, 0x3EU }, // 64 '@'
        { 0x7EU, 0x11U, 0x11U, 0x11U, 0x7EU }, // 65 'A'
        { 0x7FU, 0x49U, 0x49U, 0x49U, 0x36U }, // 66 'B'
        { 0x3EU, 0x41U, 0x41U, 0x41U, 0x22U }, // 67 'C'
        { 0x7FU, 0x41U, 0x41U, 0x22U, 0x1CU }, // 68 'D'
        { 0x7FU, 0x49U, 0x49U, 0x49U, 0x41U }, // 69 'E'
        { 0x7FU, 0x09U, 0x09U, 0x09U, 0x01U }, // 70 'F'
        { 0x3EU, 0x41U, 0x49U, 0x49U, 0x7AU }, // 71 'G'
        { 0x7FU, 0x08U, 0x08U, 0x08U, 0x7FU }, // 72 'H'
        { 0x00U, 0x41U, 0x7FU, 0x41U, 0x00U }, // 73 'I'
        { 0x20U, 0x40U, 0x41U, 0x3FU, 0x01U }, // 74 'J'
        { 0x7FU, 0x08U, 0x14U, 0x22U, 0x41U }, // 75 'K'
        { 0x7FU, 0x40U, 0x40U, 0x40U, 0x40U }, // 76 'L'
        { 0x7FU, 0x02U, 0x0CU, 0x02U, 0x7FU }, // 77 'M'
        { 0x7FU, 0x04U, 0x08U, 0x10U, 0x7FU }, // 78 'N'
        { 0x3EU, 0x41U, 0x41U, 0x41U, 0x3EU }, // 79 'O'
        { 0x7FU, 0x09U, 0x09U, 0x09U, 0x06U }, // 80 'P'
        { 0x3EU, 0x41U, 0x51U, 0x21U, 0x5EU }, // 81 'Q'
        { 0x7FU, 0x09U, 0x19U, 0x29U, 0x46U }, // 82 'R'
        { 0x46U, 0x49U, 0x49U, 0x49U, 0x31U }, // 83 'S'
        { 0x01U, 0x01U, 0x7FU, 0x01U, 0x01U }, // 84 'T'
        { 0x3FU, 0x40U, 0x40U, 0x40U, 0x3FU }, // 85 'U'
        { 0x1FU, 0x20U, 0x40U, 0x20U, 0x1FU }, // 86 'V'
        { 0x7FU, 0x20U, 0x18U, 0x20U, 0x7FU }, // 87 'W'
        { 0x63U, 0x14U, 0x08U, 0x14U, 0x63U }, // 88 'X'
        { 0x07U, 0x08U, 0x70U, 0x08U, 0x07U }, // 89 'Y'
        { 0x61U, 0x51U, 0x49U, 0x45U, 0x43U }, // 90 'Z'
        { 0x00U, 0x7FU, 0x41U, 0x41U, 0x00U }, // 91 '['
        { 0x02U, 0x04U, 0x08U, 0x10U, 0x20U }, // 92 '\'
        { 0x00U, 0x41U, 0x41U, 0x7FU, 0x00U } // 93 ']'
    };

} // namespace

std::size_t HudOverlay::formatDecimal(
    char* buf, std::size_t bufSize, std::uint32_t value, std::size_t minDigits) noexcept
{
    if (buf == nullptr || bufSize == 0U) {
        return 0U;
    }

    std::array<char, 16> temp { };
    std::size_t count { 0U };
    std::uint32_t v { value };

    do {
        temp[count] = static_cast<char>('0' + (v % 10U));
        ++count;
        v /= 10U;
    } while (v > 0U && count < temp.size());

    while (count < minDigits && count < temp.size()) {
        temp[count] = '0';
        ++count;
    }

    const std::size_t finalLen { std::min(count, bufSize - 1U) };
    for (std::size_t i { 0U }; i < finalLen; ++i) {
        buf[i] = temp[count - 1U - i];
    }
    buf[finalLen] = '\0';
    return finalLen;
}

void HudOverlay::drawGlyph(ShooterCore::RenderCommandQueue& queue, char c, float x, float y, std::uint8_t scale,
    ShooterCore::ColorRgba color) noexcept
{
    const char uc { toUpperAscii(c) };
    if (uc < ' ' || uc > ']') {
        return;
    }

    const auto glyphIdx { static_cast<std::size_t>(uc - ' ') };
    const float fScale { static_cast<float>(scale > 0U ? scale : 1U) };

    for (std::uint8_t col { 0U }; col < 5U; ++col) {
        const std::uint8_t colBits { kFontData[glyphIdx][col] };
        for (std::uint8_t row { 0U }; row < 7U; ++row) {
            if ((colBits & (1U << row)) != 0U) {
                const ShooterCore::WorldPos pxPos { x + static_cast<float>(col) * fScale,
                    y + static_cast<float>(row) * fScale };
                queue.pushBar(pxPos, fScale, fScale, 1.0f, color);
            }
        }
    }
}

void HudOverlay::drawText(ShooterCore::RenderCommandQueue& queue, std::string_view text, float screenX, float screenY,
    std::uint8_t scale, ShooterCore::ColorRgba color) noexcept
{
    const float fScale { static_cast<float>(scale > 0U ? scale : 1U) };
    const float glyphAdvance { 6.0f * fScale };

    float curX { screenX };
    for (const char c : text) {
        drawGlyph(queue, c, curX, screenY, scale, color);
        curX += glyphAdvance;
    }
}

void HudOverlay::renderHud(ShooterCore::RenderCommandQueue& queue, ShooterCore::EnergyPoints energy,
    ShooterCore::ShieldGauge shield, std::uint32_t score, std::uint32_t hiScore, ShooterCore::TransformMode mode,
    float warningTimer) noexcept
{
    // Layer 20: Screen-Space HUD Presentation
    const ShooterCore::ColorRgba bgHud { 10U, 14U, 24U, 220U };
    const ShooterCore::ColorRgba txtWhite { 240U, 245U, 255U, 255U };
    const ShooterCore::ColorRgba txtCyan { 40U, 220U, 255U, 255U };
    const ShooterCore::ColorRgba txtGold { 255U, 210U, 40U, 255U };
    const ShooterCore::ColorRgba txtAlert { 255U, 50U, 40U, 255U };

    // 1. Top HUD bar backdrop
    queue.pushBar(ShooterCore::WorldPos { 0.0f, 0.0f }, 320.0f, 15.0f, 1.0f, bgHud);

    // 2. Energy meter
    drawText(queue, "EN", 4.0f, 4.0f, 1U, txtCyan);
    const float energyPct { static_cast<float>(energy.value())
        / static_cast<float>(ShooterCore::EnergyPoints::kDefaultMax) };
    ShooterCore::ColorRgba enColor { 0U, 220U, 255U, 255U };
    if (energy.value() <= 20U) {
        enColor = txtAlert;
    } else if (energy.value() <= 50U) {
        enColor = txtGold;
    }
    queue.pushBar(ShooterCore::WorldPos { 20.0f, 4.0f }, 55.0f, 7.0f, energyPct, enColor);

    std::array<char, 8> enBuf { };
    (void)formatDecimal(enBuf.data(), enBuf.size(), energy.value(), 3U);
    drawText(queue, std::string_view { enBuf.data() }, 78.0f, 4.0f, 1U, txtWhite);

    // 3. Shield meter
    drawText(queue, "SH", 104.0f, 4.0f, 1U, txtCyan);
    const float shieldPct { static_cast<float>(shield.percentage()) / 100.0f };
    queue.pushBar(ShooterCore::WorldPos { 120.0f, 4.0f }, 35.0f, 7.0f, shieldPct,
        ShooterCore::ColorRgba { 0U, 180U, 255U, 240U });

    std::array<char, 8> shBuf { };
    (void)formatDecimal(shBuf.data(), shBuf.size(), shield.percentage(), 3U);
    drawText(queue, std::string_view { shBuf.data() }, 158.0f, 4.0f, 1U, txtWhite);

    // 4. Scores
    drawText(queue, "HI", 184.0f, 4.0f, 1U, txtGold);
    std::array<char, 12> hiBuf { };
    (void)formatDecimal(hiBuf.data(), hiBuf.size(), hiScore, 6U);
    drawText(queue, std::string_view { hiBuf.data() }, 200.0f, 4.0f, 1U, txtWhite);

    drawText(queue, "SC", 246.0f, 4.0f, 1U, txtCyan);
    std::array<char, 12> scBuf { };
    (void)formatDecimal(scBuf.data(), scBuf.size(), score, 6U);
    drawText(queue, std::string_view { scBuf.data() }, 262.0f, 4.0f, 1U, txtWhite);

    // 5. Bottom HUD bar backdrop
    queue.pushBar(ShooterCore::WorldPos { 0.0f, 226.0f }, 320.0f, 14.0f, 1.0f, bgHud);

    // 6. Mode Badge
    std::string_view modeStr { "[ROBOT]" };
    if (mode == ShooterCore::TransformMode::Jet) {
        modeStr = "[ JET ]";
    } else if (mode == ShooterCore::TransformMode::TransformingToJet
        || mode == ShooterCore::TransformMode::TransformingToRobot) {
        modeStr = "[TRANS]";
    }
    drawText(queue, modeStr, 4.0f, 229.0f, 1U, txtCyan);

    // 7. Level name
    drawText(queue, "LEVEL 1 - CAVERN", 90.0f, 229.0f, 1U, txtWhite);

    // 8. Low energy warning blink
    if (energy.value() <= 20U) {
        const auto phase { static_cast<std::uint32_t>(warningTimer * 4.0f) };
        if ((phase % 2U) == 0U) {
            drawText(queue, "! LOW ENERGY !", 210.0f, 229.0f, 1U, txtAlert);
        }
    }
}

} // namespace ThexderGame
