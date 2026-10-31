#include "ProceduralSprites.h"

#include <cstddef>
#include <cstdint>

#include "RenderCommandQueue.h"
#include "Types.h"

namespace ThexderGame {

void ProceduralSprites::drawTitleLogo(ShooterCore::RenderCommandQueue& queue, float x, float y) noexcept
{
    // Outer futuristic title backing plate (220 x 48 px)
    const ShooterCore::ColorRgba plateOuter { 20U, 30U, 55U, 240U };
    const ShooterCore::ColorRgba plateInner { 35U, 50U, 90U, 255U };
    const ShooterCore::ColorRgba neonCyan { 0U, 230U, 255U, 255U };
    const ShooterCore::ColorRgba neonGold { 255U, 215U, 0U, 255U };

    queue.pushBar(ShooterCore::WorldPos { x, y }, 220.0f, 48.0f, 1.0f, plateOuter);
    queue.pushBar(ShooterCore::WorldPos { x + 2.0f, y + 2.0f }, 216.0f, 44.0f, 1.0f, plateInner);

    // Futuristic vector accent stripes across the logo plate
    queue.pushBeam(
        ShooterCore::WorldPos { x + 6.0f, y + 6.0f }, ShooterCore::WorldPos { x + 214.0f, y + 6.0f }, neonCyan);
    queue.pushBeam(
        ShooterCore::WorldPos { x + 6.0f, y + 42.0f }, ShooterCore::WorldPos { x + 214.0f, y + 42.0f }, neonGold);

    // Decorative corner brackets
    queue.pushBar(ShooterCore::WorldPos { x + 4.0f, y + 4.0f }, 12.0f, 3.0f, 1.0f, neonCyan);
    queue.pushBar(ShooterCore::WorldPos { x + 4.0f, y + 4.0f }, 3.0f, 12.0f, 1.0f, neonCyan);
    queue.pushBar(ShooterCore::WorldPos { x + 204.0f, y + 4.0f }, 12.0f, 3.0f, 1.0f, neonCyan);
    queue.pushBar(ShooterCore::WorldPos { x + 213.0f, y + 4.0f }, 3.0f, 12.0f, 1.0f, neonCyan);
}

void ProceduralSprites::drawGameOver(ShooterCore::RenderCommandQueue& queue, float x, float y) noexcept
{
    const ShooterCore::ColorRgba bgRed { 60U, 10U, 15U, 230U };
    const ShooterCore::ColorRgba borderRed { 255U, 40U, 40U, 255U };

    queue.pushBar(ShooterCore::WorldPos { x, y }, 180.0f, 36.0f, 1.0f, bgRed);
    queue.pushBeam(ShooterCore::WorldPos { x, y }, ShooterCore::WorldPos { x + 180.0f, y }, borderRed);
    queue.pushBeam(ShooterCore::WorldPos { x, y + 36.0f }, ShooterCore::WorldPos { x + 180.0f, y + 36.0f }, borderRed);
}

void ProceduralSprites::drawVictory(ShooterCore::RenderCommandQueue& queue, float x, float y) noexcept
{
    const ShooterCore::ColorRgba bgGold { 40U, 35U, 10U, 230U };
    const ShooterCore::ColorRgba borderGold { 255U, 215U, 40U, 255U };

    queue.pushBar(ShooterCore::WorldPos { x, y }, 200.0f, 40.0f, 1.0f, bgGold);
    queue.pushBeam(ShooterCore::WorldPos { x, y }, ShooterCore::WorldPos { x + 200.0f, y }, borderGold);
    queue.pushBeam(ShooterCore::WorldPos { x, y + 40.0f }, ShooterCore::WorldPos { x + 200.0f, y + 40.0f }, borderGold);
}

void ProceduralSprites::drawNestCore(
    ShooterCore::RenderCommandQueue& queue, ShooterCore::WorldPos screenPos, bool destroyed) noexcept
{
    if (destroyed) {
        // Collapsed burned generator crater
        const ShooterCore::ColorRgba ruinColor { 40U, 40U, 45U, 255U };
        queue.pushBar(screenPos, 16.0f, 16.0f, 1.0f, ruinColor);
        return;
    }

    // Fortified mechanical drone dispenser
    const ShooterCore::ColorRgba armorColor { 80U, 90U, 110U, 255U };
    const ShooterCore::ColorRgba corePulsing { 255U, 200U, 30U, 255U };

    queue.pushBar(screenPos, 16.0f, 16.0f, 1.0f, armorColor);
    queue.pushBar(ShooterCore::WorldPos { screenPos.x + 4.0f, screenPos.y + 4.0f }, 8.0f, 8.0f, 1.0f, corePulsing);
}

} // namespace ThexderGame
