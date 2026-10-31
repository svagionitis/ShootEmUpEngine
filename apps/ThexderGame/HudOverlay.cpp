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

void HudOverlay::drawSegmentedBar(ShooterCore::RenderCommandQueue& queue, float startX, float startY,
    std::uint8_t totalSegments, std::uint8_t litSegments, ShooterCore::ColorRgba litColor,
    ShooterCore::ColorRgba unlitColor) noexcept
{
    const std::uint8_t clampedLit { std::min(litSegments, totalSegments) };
    for (std::uint8_t i { 0U }; i < totalSegments; ++i) {
        const float segX { startX + static_cast<float>(i) * 3.0f };
        const ShooterCore::ColorRgba col { (i < clampedLit) ? litColor : unlitColor };
        queue.pushBar(ShooterCore::WorldPos { segX, startY }, 2.0f, 6.0f, 1.0f, col);
    }
}

void HudOverlay::renderHud(ShooterCore::RenderCommandQueue& queue, ShooterCore::EnergyPoints energy,
    ShooterCore::ShieldGauge shield, std::uint32_t score, std::uint32_t hiScore, ShooterCore::TransformMode mode,
    float warningTimer, ShooterCore::SecondaryWeaponType secType, std::uint8_t secAmmo, std::string_view sectorName,
    bool lockWarning) noexcept
{
    // Layer 20: Cockpit Canopy & Segmented LED HUD Presentation
    const ShooterCore::ColorRgba bgCockpit { 14U, 18U, 28U, 245U };
    const ShooterCore::ColorRgba bezelBorder { 48U, 60U, 82U, 255U };
    const ShooterCore::ColorRgba rivetColor { 150U, 175U, 210U, 255U };
    const ShooterCore::ColorRgba strutBase { 22U, 28U, 42U, 240U };
    const ShooterCore::ColorRgba strutGlare { 65U, 80U, 110U, 180U };

    const ShooterCore::ColorRgba txtWhite { 240U, 245U, 255U, 255U };
    const ShooterCore::ColorRgba txtCyan { 40U, 220U, 255U, 255U };
    const ShooterCore::ColorRgba txtGold { 255U, 210U, 40U, 255U };
    const ShooterCore::ColorRgba txtAlert { 255U, 50U, 40U, 255U };
    const ShooterCore::ColorRgba unlitCathode { 12U, 24U, 34U, 190U };

    // 1. Molded Top Canopy Frame & Bezel (Y: 0 to 16 px)
    queue.pushBar(ShooterCore::WorldPos { 0.0f, 0.0f }, 320.0f, 16.0f, 1.0f, bgCockpit);
    queue.pushBar(ShooterCore::WorldPos { 0.0f, 15.0f }, 320.0f, 1.0f, 1.0f, bezelBorder);

    // Corner Rivet Studs
    queue.pushBar(ShooterCore::WorldPos { 2.0f, 2.0f }, 2.0f, 2.0f, 1.0f, rivetColor);
    queue.pushBar(ShooterCore::WorldPos { 316.0f, 2.0f }, 2.0f, 2.0f, 1.0f, rivetColor);

    // 2. Left & Right Vertical Canopy Struts (Y: 16 to 222 px)
    queue.pushBar(ShooterCore::WorldPos { 0.0f, 16.0f }, 2.0f, 206.0f, 1.0f, strutBase);
    queue.pushBar(ShooterCore::WorldPos { 1.0f, 16.0f }, 1.0f, 206.0f, 1.0f, strutGlare);
    queue.pushBar(ShooterCore::WorldPos { 318.0f, 16.0f }, 2.0f, 206.0f, 1.0f, strutBase);
    queue.pushBar(ShooterCore::WorldPos { 318.0f, 16.0f }, 1.0f, 206.0f, 1.0f, strutGlare);

    // 3. Energy Instrument: 24-Segment LED Ladder Meter
    drawText(queue, "EN", 6.0f, 5.0f, 1U, txtCyan);

    const std::uint16_t enVal { energy.value() };
    const auto enRawLit { static_cast<std::uint8_t>(
        (static_cast<std::uint32_t>(enVal) * 24U + 50U) / ShooterCore::EnergyPoints::kDefaultMax) };
    const std::uint8_t enLit { (enVal > 0U && enRawLit == 0U) ? static_cast<std::uint8_t>(1U)
                                                              : std::min(enRawLit, static_cast<std::uint8_t>(24U)) };

    ShooterCore::ColorRgba enLitColor { 0U, 230U, 195U, 255U };
    if (enVal <= 20U) {
        const auto alertPhase { static_cast<std::uint32_t>(warningTimer * 4.0f) };
        enLitColor = ((alertPhase % 2U) == 0U) ? txtAlert : ShooterCore::ColorRgba { 180U, 30U, 30U, 200U };
    } else if (enVal <= 50U) {
        enLitColor = txtGold;
    }

    drawSegmentedBar(queue, 20.0f, 5.0f, 24U, enLit, enLitColor, unlitCathode);

    std::array<char, 8> enBuf { };
    (void)formatDecimal(enBuf.data(), enBuf.size(), enVal, 3U);
    drawText(queue, std::string_view { enBuf.data() }, 95.0f, 5.0f, 1U, txtWhite);

    // 4. Shield Instrument: 16-Segment LED Ladder Meter
    drawText(queue, "SH", 118.0f, 5.0f, 1U, txtCyan);

    const std::uint8_t shVal { shield.percentage() };
    const auto shLit { static_cast<std::uint8_t>(
        std::min((static_cast<std::uint32_t>(shVal) * 16U + 50U) / 100U, 16U)) };
    const ShooterCore::ColorRgba shLitColor { 30U, 200U, 255U, 255U };

    drawSegmentedBar(queue, 132.0f, 5.0f, 16U, shLit, shLitColor, unlitCathode);

    // 5. Scores (High Score & Current Score)
    drawText(queue, "HI", 184.0f, 5.0f, 1U, txtGold);
    std::array<char, 12> hiBuf { };
    (void)formatDecimal(hiBuf.data(), hiBuf.size(), hiScore, 6U);
    drawText(queue, std::string_view { hiBuf.data() }, 200.0f, 5.0f, 1U, txtWhite);

    drawText(queue, "SC", 246.0f, 5.0f, 1U, txtCyan);
    std::array<char, 12> scBuf { };
    (void)formatDecimal(scBuf.data(), scBuf.size(), score, 6U);
    drawText(queue, std::string_view { scBuf.data() }, 262.0f, 5.0f, 1U, txtWhite);

    // 6. Molded Bottom Deck Console (Y: 222 to 240 px)
    queue.pushBar(ShooterCore::WorldPos { 0.0f, 222.0f }, 320.0f, 18.0f, 1.0f, bgCockpit);
    queue.pushBar(ShooterCore::WorldPos { 0.0f, 222.0f }, 320.0f, 1.0f, 1.0f, bezelBorder);

    // Bottom Corner Rivets
    queue.pushBar(ShooterCore::WorldPos { 2.0f, 236.0f }, 2.0f, 2.0f, 1.0f, rivetColor);
    queue.pushBar(ShooterCore::WorldPos { 316.0f, 236.0f }, 2.0f, 2.0f, 1.0f, rivetColor);

    // 7. Mechanical Transformation Mode Badge
    std::string_view modeStr { "[ROBOT]" };
    ShooterCore::ColorRgba modeColor { 0U, 230U, 180U, 255U };
    if (mode == ShooterCore::TransformMode::Jet) {
        modeStr = "[ JET ]";
        modeColor = txtCyan;
    } else if (mode == ShooterCore::TransformMode::TransformingToJet
        || mode == ShooterCore::TransformMode::TransformingToRobot) {
        modeStr = "[TRANS]";
        modeColor = txtGold;
    }
    drawText(queue, modeStr, 6.0f, 227.0f, 1U, modeColor);

    // 8. Sector Telemetry Title
    drawText(queue, sectorName, 56.0f, 227.0f, 1U, txtWhite);

    // 9. Secondary Munition Bay Indicator
    std::string_view secName { "SEC:---" };
    ShooterCore::ColorRgba secColor { 140U, 140U, 150U, 255U };
    if (secType == ShooterCore::SecondaryWeaponType::HomingMissile) {
        secName = "MSL:";
        secColor = txtGold;
    } else if (secType == ShooterCore::SecondaryWeaponType::FlashBomb) {
        secName = "BMB:";
        secColor = txtAlert;
    }
    drawText(queue, secName, 150.0f, 227.0f, 1U, secColor);

    if (secType != ShooterCore::SecondaryWeaponType::None) {
        std::array<char, 8> secBuf { };
        (void)formatDecimal(secBuf.data(), secBuf.size(), secAmmo, 2U);
        drawText(queue, std::string_view { secBuf.data() }, 176.0f, 227.0f, 1U, txtWhite);
    }

    // 10. Annunciator Status Lamps
    if (enVal <= 20U) {
        const auto phase { static_cast<std::uint32_t>(warningTimer * 4.0f) };
        if ((phase % 2U) == 0U) {
            drawText(queue, "!LOW!", 200.0f, 227.0f, 1U, txtAlert);
        }
    }

    if (shield.isActive()) {
        drawText(queue, "[SHLD]", 236.0f, 227.0f, 1U, txtCyan);
    }

    if (lockWarning) {
        drawText(queue, "[LOCK]", 276.0f, 227.0f, 1U, txtGold);
    }
}

} // namespace ThexderGame
