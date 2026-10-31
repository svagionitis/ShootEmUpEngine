#include "SpriteAtlas.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>

#include <SDL.h>

#include "Types.h"

namespace SdlAdapter {

namespace {

    // Authentic IBM PC EGA 16-color palette (RGBA values)
    constexpr std::uint32_t kColorTransparent { 0x00000000U };
    constexpr std::uint32_t kColorBlack { 0xFF000000U };
    constexpr std::uint32_t kColorBlue { 0xFFAA0000U }; // RGBA: B=AA, R=0
    constexpr std::uint32_t kColorRed { 0xFF0000AAU };
    constexpr std::uint32_t kColorMagenta { 0xFFAA00AAU };
    constexpr std::uint32_t kColorBrown { 0xFF0055AAU };
    constexpr std::uint32_t kColorLightGray { 0xFFAAAAAAU };
    constexpr std::uint32_t kColorDarkGray { 0xFF555555U };
    constexpr std::uint32_t kColorLightBlue { 0xFFFF5555U };
    constexpr std::uint32_t kColorLightGreen { 0xFF55FF55U };
    constexpr std::uint32_t kColorLightCyan { 0xFFFFFF55U };
    constexpr std::uint32_t kColorLightRed { 0xFF5555FFU };
    constexpr std::uint32_t kColorBrightRed { 0xFF0000FFU };
    constexpr std::uint32_t kColorLightMagenta { 0xFFFF55FFU };
    constexpr std::uint32_t kColorYellow { 0xFF55FFFFU };
    constexpr std::uint32_t kColorWhite { 0xFFFFFFFFU };

    inline void putPixel(std::uint32_t* buf, int bufWidth, int x, int y, std::uint32_t color) noexcept
    {
        if (x >= 0 && x < bufWidth && y >= 0 && y < bufWidth) {
            buf[y * bufWidth + x] = color;
        }
    }

    void fillBox(std::uint32_t* buf, int bw, int x, int y, int w, int h, std::uint32_t color) noexcept
    {
        for (int dy { 0 }; dy < h; ++dy) {
            for (int dx { 0 }; dx < w; ++dx) {
                putPixel(buf, bw, x + dx, y + dy, color);
            }
        }
    }

    void drawRobot(std::uint32_t* buf, int bw, int sx, int sy, int walkFrame, bool jump, bool crouch) noexcept
    {
        if (crouch) {
            // Compact squatted robot pose (16 x 16 px)
            fillBox(buf, bw, sx + 2, sy + 4, 12, 10, kColorLightBlue);
            fillBox(buf, bw, sx + 4, sy + 6, 8, 3, kColorLightCyan);
            putPixel(buf, bw, sx + 7, sy + 10, kColorBrightRed);
            putPixel(buf, bw, sx + 8, sy + 10, kColorBrightRed);
            fillBox(buf, bw, sx + 1, sy + 12, 14, 4, kColorDarkGray);
            return;
        }

        // Helmet & Visor (Y: 0..7)
        fillBox(buf, bw, sx + 4, sy + 1, 8, 7, kColorLightBlue);
        fillBox(buf, bw, sx + 6, sy + 0, 4, 2, kColorWhite); // Crest
        fillBox(buf, bw, sx + 5, sy + 3, 6, 2, kColorLightCyan); // Glowing visor
        fillBox(buf, bw, sx + 6, sy + 6, 4, 2, kColorDarkGray); // Chin / rebreather

        // Torso / Cockpit (Y: 8..19)
        fillBox(buf, bw, sx + 2, sy + 8, 12, 10, kColorLightBlue);
        fillBox(buf, bw, sx + 4, sy + 9, 8, 8, kColorWhite);
        fillBox(buf, bw, sx + 5, sy + 10, 6, 6, kColorLightBlue);
        // Chest glowing power core
        fillBox(buf, bw, sx + 6, sy + 12, 4, 3, kColorRed);
        putPixel(buf, bw, sx + 7, sy + 13, kColorYellow);
        putPixel(buf, bw, sx + 8, sy + 13, kColorYellow);

        // Arms & Shoulders
        fillBox(buf, bw, sx + 1, sy + 8, 2, 8, kColorDarkGray);
        fillBox(buf, bw, sx + 13, sy + 8, 2, 8, kColorDarkGray);

        // Waist / Belt (Y: 18..19)
        fillBox(buf, bw, sx + 3, sy + 18, 10, 2, kColorDarkGray);

        // Legs (Y: 20..31)
        if (jump) {
            // Tucked thruster legs with booster sparks
            fillBox(buf, bw, sx + 2, sy + 20, 5, 8, kColorLightBlue);
            fillBox(buf, bw, sx + 9, sy + 20, 5, 8, kColorLightBlue);
            fillBox(buf, bw, sx + 3, sy + 28, 3, 2, kColorDarkGray);
            fillBox(buf, bw, sx + 10, sy + 28, 3, 2, kColorDarkGray);
            putPixel(buf, bw, sx + 4, sy + 30, kColorYellow);
            putPixel(buf, bw, sx + 11, sy + 30, kColorYellow);
        } else if (walkFrame == 0) {
            // Left leg stepping forward, right leg back
            fillBox(buf, bw, sx + 2, sy + 20, 5, 6, kColorLightBlue);
            fillBox(buf, bw, sx + 1, sy + 26, 5, 6, kColorDarkGray);
            fillBox(buf, bw, sx + 9, sy + 20, 5, 6, kColorLightBlue);
            fillBox(buf, bw, sx + 10, sy + 26, 5, 6, kColorDarkGray);
        } else if (walkFrame == 1) {
            // Neutral standing / passing stride
            fillBox(buf, bw, sx + 3, sy + 20, 4, 11, kColorLightBlue);
            fillBox(buf, bw, sx + 9, sy + 20, 4, 11, kColorLightBlue);
            fillBox(buf, bw, sx + 2, sy + 30, 5, 2, kColorDarkGray);
            fillBox(buf, bw, sx + 9, sy + 30, 5, 2, kColorDarkGray);
        } else {
            // Right leg stepping forward, left leg back
            fillBox(buf, bw, sx + 3, sy + 20, 5, 6, kColorLightBlue);
            fillBox(buf, bw, sx + 4, sy + 26, 5, 6, kColorDarkGray);
            fillBox(buf, bw, sx + 9, sy + 20, 5, 6, kColorLightBlue);
            fillBox(buf, bw, sx + 8, sy + 26, 5, 6, kColorDarkGray);
        }
    }

    void drawJet(std::uint32_t* buf, int bw, int sx, int sy) noexcept
    {
        // Dimensions: 32 x 16 px
        // Sleek supersonic fuselage
        fillBox(buf, bw, sx + 8, sy + 6, 16, 5, kColorLightBlue);
        fillBox(buf, bw, sx + 10, sy + 7, 12, 3, kColorWhite);

        // Needle nosecone
        fillBox(buf, bw, sx + 24, sy + 7, 5, 3, kColorLightBlue);
        fillBox(buf, bw, sx + 29, sy + 8, 3, 1, kColorWhite);

        // Cockpit canopy (slanted cyan glass)
        fillBox(buf, bw, sx + 16, sy + 4, 6, 2, kColorLightCyan);
        putPixel(buf, bw, sx + 17, sy + 4, kColorWhite);
        putPixel(buf, bw, sx + 18, sy + 4, kColorWhite);

        // Delta wings (angled top and bottom)
        fillBox(buf, bw, sx + 4, sy + 2, 12, 2, kColorDarkGray);
        fillBox(buf, bw, sx + 2, sy + 1, 4, 1, kColorRed); // Wingtip beacon
        fillBox(buf, bw, sx + 4, sy + 12, 12, 2, kColorDarkGray);
        fillBox(buf, bw, sx + 2, sy + 14, 4, 1, kColorRed);

        // Rear engine thruster nacelles
        fillBox(buf, bw, sx + 0, sy + 5, 8, 2, kColorDarkGray);
        fillBox(buf, bw, sx + 0, sy + 9, 8, 2, kColorDarkGray);
        putPixel(buf, bw, sx + 0, sy + 5, kColorYellow);
        putPixel(buf, bw, sx + 0, sy + 6, kColorYellow);
        putPixel(buf, bw, sx + 0, sy + 9, kColorYellow);
        putPixel(buf, bw, sx + 0, sy + 10, kColorYellow);
    }

    void drawExhaust(std::uint32_t* buf, int bw, int sx, int sy, int frame) noexcept
    {
        // Twin afterburner flames (8 x 16 px)
        const int len { (frame == 0) ? 5 : 7 };
        for (int i { 0 }; i < len; ++i) {
            const std::uint32_t col { (i < 2) ? kColorWhite : (i < 4 ? kColorYellow : kColorLightRed) };
            putPixel(buf, bw, sx + 7 - i, sy + 5, col);
            putPixel(buf, bw, sx + 7 - i, sy + 6, col);
            putPixel(buf, bw, sx + 7 - i, sy + 9, col);
            putPixel(buf, bw, sx + 7 - i, sy + 10, col);
        }
    }

    void drawMorph(std::uint32_t* buf, int bw, int sx, int sy, int stage) noexcept
    {
        if (stage == 1) {
            // Stage 1 (24x24): Torso tilts forward, limbs folding
            fillBox(buf, bw, sx + 4, sy + 4, 16, 12, kColorLightBlue);
            fillBox(buf, bw, sx + 10, sy + 5, 8, 3, kColorLightCyan);
            fillBox(buf, bw, sx + 2, sy + 16, 12, 6, kColorDarkGray);
        } else if (stage == 2) {
            // Stage 2 (28x20): Fuselage elongating, wings deploying
            fillBox(buf, bw, sx + 4, sy + 6, 20, 8, kColorLightBlue);
            fillBox(buf, bw, sx + 14, sy + 5, 8, 3, kColorLightCyan);
            fillBox(buf, bw, sx + 2, sy + 2, 8, 3, kColorDarkGray);
            fillBox(buf, bw, sx + 2, sy + 15, 8, 3, kColorDarkGray);
        } else {
            // Stage 3 (32x16): Near-complete jet chassis
            fillBox(buf, bw, sx + 6, sy + 5, 22, 6, kColorLightBlue);
            fillBox(buf, bw, sx + 16, sy + 4, 8, 2, kColorLightCyan);
            fillBox(buf, bw, sx + 2, sy + 3, 10, 2, kColorDarkGray);
            fillBox(buf, bw, sx + 2, sy + 11, 10, 2, kColorDarkGray);
            fillBox(buf, bw, sx + 28, sy + 7, 4, 2, kColorWhite);
        }
    }

    void drawBuzzer(std::uint32_t* buf, int bw, int sx, int sy, int frame) noexcept
    {
        // 16x16 rotating drone
        fillBox(buf, bw, sx + 4, sy + 4, 8, 8, kColorDarkGray);
        fillBox(buf, bw, sx + 6, sy + 6, 4, 4, (frame == 0) ? kColorBrightRed : kColorYellow);

        if (frame == 0) {
            // Cross blades
            fillBox(buf, bw, sx + 1, sy + 7, 14, 2, kColorLightGray);
            fillBox(buf, bw, sx + 7, sy + 1, 2, 14, kColorLightGray);
        } else {
            // Diagonal blades
            for (int i { 1 }; i < 15; ++i) {
                putPixel(buf, bw, sx + i, sy + i, kColorLightGray);
                putPixel(buf, bw, sx + 15 - i, sy + i, kColorLightGray);
            }
        }
    }

    void drawCrawler(std::uint32_t* buf, int bw, int sx, int sy, int frame) noexcept
    {
        // 16x16 segmented trilobite
        fillBox(buf, bw, sx + 3, sy + 4, 10, 8, kColorMagenta);
        fillBox(buf, bw, sx + 5, sy + 5, 6, 6, kColorLightMagenta);
        putPixel(buf, bw, sx + 4, sy + 6, kColorWhite);
        putPixel(buf, bw, sx + 11, sy + 6, kColorWhite);

        // Legs
        const int legOffset { (frame == 0) ? 0 : 2 };
        putPixel(buf, bw, sx + 1, sy + 12 - legOffset, kColorDarkGray);
        putPixel(buf, bw, sx + 2, sy + 13 - legOffset, kColorDarkGray);
        putPixel(buf, bw, sx + 13, sy + 12 - legOffset, kColorDarkGray);
        putPixel(buf, bw, sx + 14, sy + 13 - legOffset, kColorDarkGray);
    }

    void drawManta(std::uint32_t* buf, int bw, int sx, int sy, int frame) noexcept
    {
        // 24x16 aerial scout
        fillBox(buf, bw, sx + 8, sy + 5, 8, 6, kColorDarkGray);
        fillBox(buf, bw, sx + 10, sy + 7, 4, 2, kColorLightGreen); // Optic

        if (frame == 0) {
            // Wings curved upwards
            fillBox(buf, bw, sx + 1, sy + 2, 8, 3, kColorLightBlue);
            fillBox(buf, bw, sx + 15, sy + 2, 8, 3, kColorLightBlue);
        } else {
            // Wings swept downwards
            fillBox(buf, bw, sx + 1, sy + 8, 8, 3, kColorLightBlue);
            fillBox(buf, bw, sx + 15, sy + 8, 8, 3, kColorLightBlue);
        }
    }

    void drawNest(std::uint32_t* buf, int bw, int sx, int sy, bool destroyed) noexcept
    {
        // 32x32 fortified drone dispenser
        if (destroyed) {
            // Burned slag crater
            fillBox(buf, bw, sx + 4, sy + 8, 24, 18, kColorDarkGray);
            fillBox(buf, bw, sx + 8, sy + 12, 16, 10, kColorBlack);
            for (int i { 0 }; i < 6; ++i) {
                putPixel(buf, bw, sx + 6 + i * 3, sy + 10 + (i % 2) * 2, kColorBrown);
            }
            return;
        }

        // Active fortified hive bunker
        fillBox(buf, bw, sx + 2, sy + 2, 28, 28, kColorDarkGray);
        fillBox(buf, bw, sx + 4, sy + 4, 24, 24, kColorLightGray);

        // Core chamber
        fillBox(buf, bw, sx + 8, sy + 8, 16, 16, kColorBlack);
        fillBox(buf, bw, sx + 10, sy + 10, 12, 12, kColorBrightRed);
        fillBox(buf, bw, sx + 12, sy + 12, 8, 8, kColorYellow);
        fillBox(buf, bw, sx + 14, sy + 14, 4, 4, kColorWhite);

        // Corner armor bolts
        putPixel(buf, bw, sx + 4, sy + 4, kColorWhite);
        putPixel(buf, bw, sx + 27, sy + 4, kColorWhite);
        putPixel(buf, bw, sx + 4, sy + 27, kColorWhite);
        putPixel(buf, bw, sx + 27, sy + 27, kColorWhite);
    }

    void drawExplosionPhase(std::uint32_t* buf, int bw, int sx, int sy, int phase) noexcept
    {
        // 24x24 explosion burst
        const int cx { sx + 12 };
        const int cy { sy + 12 };

        if (phase == 1) {
            // Intense white/cyan flash
            fillBox(buf, bw, cx - 4, cy - 4, 8, 8, kColorWhite);
            fillBox(buf, bw, cx - 6, cy - 2, 12, 4, kColorLightCyan);
            fillBox(buf, bw, cx - 2, cy - 6, 4, 12, kColorLightCyan);
        } else if (phase == 2) {
            // Expanding yellow/orange fireball
            fillBox(buf, bw, cx - 7, cy - 7, 14, 14, kColorYellow);
            fillBox(buf, bw, cx - 4, cy - 4, 8, 8, kColorWhite);
            fillBox(buf, bw, cx - 9, cy - 3, 18, 6, kColorLightRed);
            fillBox(buf, bw, cx - 3, cy - 9, 6, 18, kColorLightRed);
        } else if (phase == 3) {
            // Fragmenting debris cloud
            fillBox(buf, bw, cx - 8, cy - 8, 16, 16, kColorRed);
            fillBox(buf, bw, cx - 4, cy - 4, 8, 8, kColorDarkGray);
            putPixel(buf, bw, cx - 10, cy - 10, kColorYellow);
            putPixel(buf, bw, cx + 10, cy - 10, kColorYellow);
            putPixel(buf, bw, cx - 10, cy + 10, kColorYellow);
            putPixel(buf, bw, cx + 10, cy + 10, kColorYellow);
        } else {
            // Dissipating smoke
            fillBox(buf, bw, cx - 9, cy - 9, 18, 18, kColorDarkGray);
            fillBox(buf, bw, cx - 5, cy - 5, 10, 10, kColorBlack);
        }
    }

    void drawTilesets(std::uint32_t* buf, int bw) noexcept
    {
        // 1. Metal Wall (32, 64) 16x16
        const int mx { 32 };
        const int my { 64 };
        fillBox(buf, bw, mx, my, 16, 16, kColorDarkGray);
        fillBox(buf, bw, mx + 1, my + 1, 14, 14, kColorLightGray);
        // Beveled steel border
        for (int i { 0 }; i < 16; ++i) {
            putPixel(buf, bw, mx + i, my, kColorWhite);
            putPixel(buf, bw, mx, my + i, kColorWhite);
            putPixel(buf, bw, mx + i, my + 15, kColorDarkGray);
            putPixel(buf, bw, mx + 15, my + i, kColorDarkGray);
        }
        // Center cross rivets
        putPixel(buf, bw, mx + 3, my + 3, kColorWhite);
        putPixel(buf, bw, mx + 12, my + 3, kColorWhite);
        putPixel(buf, bw, mx + 3, my + 12, kColorWhite);
        putPixel(buf, bw, mx + 12, my + 12, kColorWhite);

        // 2. Circuit Wall (48, 64) 16x16
        const int cx { 48 };
        const int cy { 64 };
        fillBox(buf, bw, cx, cy, 16, 16, kColorBlack);
        fillBox(buf, bw, cx + 2, cy + 2, 12, 12, kColorBlue);
        // Neon green circuit traces
        for (int i { 3 }; i < 13; ++i) {
            putPixel(buf, bw, cx + i, cy + 5, kColorLightGreen);
            putPixel(buf, bw, cx + 5, cy + i, kColorLightGreen);
        }
        putPixel(buf, bw, cx + 10, cy + 10, kColorYellow);
        putPixel(buf, bw, cx + 11, cy + 10, kColorYellow);

        // 3. Destructible Wall (64, 64) 16x16
        const int dx { 64 };
        const int dy { 64 };
        fillBox(buf, bw, dx, dy, 16, 16, kColorBrown);
        // Mortar lines & crystal fractures
        fillBox(buf, bw, dx, dy + 7, 16, 1, kColorBlack);
        fillBox(buf, bw, dx + 8, dy, 1, 7, kColorBlack);
        fillBox(buf, bw, dx + 4, dy + 8, 1, 8, kColorBlack);
        fillBox(buf, bw, dx + 12, dy + 8, 1, 8, kColorBlack);
        putPixel(buf, bw, dx + 3, dy + 3, kColorYellow);
        putPixel(buf, bw, dx + 11, dy + 11, kColorYellow);

        // 4. Hazard Lava (80, 64) 16x16
        const int hx { 80 };
        const int hy { 64 };
        fillBox(buf, bw, hx, hy, 16, 16, kColorRed);
        for (int y { 0 }; y < 16; ++y) {
            for (int x { 0 }; x < 16; ++x) {
                if ((x + y * 3) % 4 == 0) {
                    putPixel(buf, bw, hx + x, hy + y, kColorYellow);
                } else if ((x + y) % 3 == 0) {
                    putPixel(buf, bw, hx + x, hy + y, kColorBrightRed);
                }
            }
        }
    }

} // namespace

SpriteAtlas::~SpriteAtlas() noexcept
{
    shutdown();
}

SpriteAtlas::SpriteAtlas(SpriteAtlas&& other) noexcept
    : atlasTexture_ { other.atlasTexture_ }
    , spriteRects_ { other.spriteRects_ }
    , tileRects_ { other.tileRects_ }
{
    other.atlasTexture_ = nullptr;
}

SpriteAtlas& SpriteAtlas::operator=(SpriteAtlas&& other) noexcept
{
    if (this != &other) {
        shutdown();
        atlasTexture_ = other.atlasTexture_;
        spriteRects_ = other.spriteRects_;
        tileRects_ = other.tileRects_;
        other.atlasTexture_ = nullptr;
    }
    return *this;
}

bool SpriteAtlas::init(SDL_Renderer* renderer) noexcept
{
    if (renderer == nullptr) {
        return false;
    }

    shutdown();

    // Allocate 256x256 RGBA pixel buffer on stack/heap during init only
    std::array<std::uint32_t, kAtlasWidth * kAtlasHeight> pixelBuffer { };
    pixelBuffer.fill(kColorTransparent);

    generateAtlasPixels(pixelBuffer.data());

    SDL_Surface* surface { SDL_CreateRGBSurfaceWithFormatFrom(pixelBuffer.data(), kAtlasWidth, kAtlasHeight, 32,
        kAtlasWidth * static_cast<int>(sizeof(std::uint32_t)), SDL_PIXELFORMAT_RGBA8888) };

    if (surface == nullptr) {
        return false;
    }

    atlasTexture_ = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (atlasTexture_ == nullptr) {
        return false;
    }

    (void)SDL_SetTextureBlendMode(atlasTexture_, SDL_BLENDMODE_BLEND);

    buildRectTables();
    return true;
}

void SpriteAtlas::shutdown() noexcept
{
    if (atlasTexture_ != nullptr) {
        SDL_DestroyTexture(atlasTexture_);
        atlasTexture_ = nullptr;
    }
}

bool SpriteAtlas::isLoaded() const noexcept
{
    return atlasTexture_ != nullptr;
}

SDL_Texture* SpriteAtlas::getTexture() const noexcept
{
    return atlasTexture_;
}

void SpriteAtlas::generateAtlasPixels(std::uint32_t* pixelBuffer) noexcept
{
    // 1. Robot walking & jumping & crouching frames (Y: 0..31)
    drawRobot(pixelBuffer, kAtlasWidth, 0, 0, 1, false, false); // Idle (16x32)
    drawRobot(pixelBuffer, kAtlasWidth, 16, 0, 0, false, false); // Walk 1 (16x32)
    drawRobot(pixelBuffer, kAtlasWidth, 32, 0, 1, false, false); // Walk 2 (16x32)
    drawRobot(pixelBuffer, kAtlasWidth, 48, 0, 2, false, false); // Walk 3 (16x32)
    drawRobot(pixelBuffer, kAtlasWidth, 64, 0, 1, true, false); // Jump (16x32)
    drawRobot(pixelBuffer, kAtlasWidth, 80, 16, 1, false, true); // Crouch (16x16)

    // 2. Transformation morph frames
    drawMorph(pixelBuffer, kAtlasWidth, 96, 8, 1); // Morph 1 (24x24)
    drawMorph(pixelBuffer, kAtlasWidth, 120, 12, 2); // Morph 2 (28x20)
    drawMorph(pixelBuffer, kAtlasWidth, 148, 16, 3); // Morph 3 (32x16)

    // 3. Jet & Exhausts
    drawJet(pixelBuffer, kAtlasWidth, 180, 16); // Jet Chassis (32x16)
    drawExhaust(pixelBuffer, kAtlasWidth, 212, 16, 0); // Exhaust Fr 1 (8x16)
    drawExhaust(pixelBuffer, kAtlasWidth, 220, 16, 1); // Exhaust Fr 2 (8x16)

    // 4. Enemies (Y: 32)
    drawBuzzer(pixelBuffer, kAtlasWidth, 0, 32, 0); // Buzzer 1 (16x16)
    drawBuzzer(pixelBuffer, kAtlasWidth, 16, 32, 1); // Buzzer 2 (16x16)
    drawCrawler(pixelBuffer, kAtlasWidth, 32, 32, 0); // Crawler 1 (16x16)
    drawCrawler(pixelBuffer, kAtlasWidth, 48, 32, 1); // Crawler 2 (16x16)
    drawManta(pixelBuffer, kAtlasWidth, 64, 32, 0); // Manta 1 (24x16)
    drawManta(pixelBuffer, kAtlasWidth, 88, 32, 1); // Manta 2 (24x16)

    // 5. Nests (Y: 32)
    drawNest(pixelBuffer, kAtlasWidth, 112, 32, false); // Nest Active (32x32)
    drawNest(pixelBuffer, kAtlasWidth, 144, 32, true); // Nest Slag (32x32)

    // 6. Explosions
    drawExplosionPhase(pixelBuffer, kAtlasWidth, 176, 32, 1); // Expl 1 (24x24)
    drawExplosionPhase(pixelBuffer, kAtlasWidth, 200, 32, 2); // Expl 2 (24x24)
    drawExplosionPhase(pixelBuffer, kAtlasWidth, 224, 32, 3); // Expl 3 (24x24)
    drawExplosionPhase(pixelBuffer, kAtlasWidth, 0, 64, 4); // Expl 4 (24x24)

    // 7. Tilesets
    drawTilesets(pixelBuffer, kAtlasWidth);
}

void SpriteAtlas::buildRectTables() noexcept
{
    spriteRects_.fill(SDL_Rect { 0, 0, 0, 0 });
    tileRects_.fill(SDL_Rect { 0, 0, 0, 0 });

    // Player Robot frames
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::PlayerRobotIdle)] = SDL_Rect { 0, 0, 16, 32 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::PlayerRobotWalk1)] = SDL_Rect { 16, 0, 16, 32 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::PlayerRobotWalk2)] = SDL_Rect { 32, 0, 16, 32 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::PlayerRobotWalk3)] = SDL_Rect { 48, 0, 16, 32 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::PlayerRobotJump)] = SDL_Rect { 64, 0, 16, 32 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::PlayerRobotCrouch)] = SDL_Rect { 80, 16, 16, 16 };

    // Morph frames
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::PlayerMorph1)] = SDL_Rect { 96, 8, 24, 24 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::PlayerMorph2)] = SDL_Rect { 120, 12, 28, 20 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::PlayerMorph3)] = SDL_Rect { 148, 16, 32, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::PlayerTransforming)] = SDL_Rect { 120, 12, 28, 20 };

    // Jet & Exhaust
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::PlayerJet)] = SDL_Rect { 180, 16, 32, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::PlayerJetExhaust1)] = SDL_Rect { 212, 16, 8, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::PlayerJetExhaust2)] = SDL_Rect { 220, 16, 8, 16 };

    // Enemies
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::EnemyPatrol)] = SDL_Rect { 32, 32, 16, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::EnemyBuzzer1)] = SDL_Rect { 0, 32, 16, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::EnemyBuzzer2)] = SDL_Rect { 16, 32, 16, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::EnemyCrawler1)] = SDL_Rect { 32, 32, 16, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::EnemyCrawler2)] = SDL_Rect { 48, 32, 16, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::EnemyFlyer)] = SDL_Rect { 0, 32, 16, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::EnemyManta1)] = SDL_Rect { 64, 32, 24, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::EnemyManta2)] = SDL_Rect { 88, 32, 24, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::EnemyBoss)] = SDL_Rect { 112, 32, 32, 32 };

    // Nest
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::NestCoreActive)] = SDL_Rect { 112, 32, 32, 32 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::NestCoreDestroyed)] = SDL_Rect { 144, 32, 32, 32 };

    // Explosions
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::Explosion)] = SDL_Rect { 200, 32, 24, 24 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::ExplosionPhase1)] = SDL_Rect { 176, 32, 24, 24 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::ExplosionPhase2)] = SDL_Rect { 200, 32, 24, 24 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::ExplosionPhase3)] = SDL_Rect { 224, 32, 24, 24 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::ExplosionPhase4)] = SDL_Rect { 0, 64, 24, 24 };

    // Tiles
    tileRects_[static_cast<std::size_t>(ShooterCore::TileType::SolidWall)] = SDL_Rect { 32, 64, 16, 16 };
    tileRects_[static_cast<std::size_t>(ShooterCore::TileType::DestructibleWall)] = SDL_Rect { 64, 64, 16, 16 };
    tileRects_[static_cast<std::size_t>(ShooterCore::TileType::Hazard)] = SDL_Rect { 80, 64, 16, 16 };

    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::TileMetalWall)] = SDL_Rect { 32, 64, 16, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::TileCircuitWall)] = SDL_Rect { 48, 64, 16, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::TileDestructibleWall)] = SDL_Rect { 64, 64, 16, 16 };
    spriteRects_[static_cast<std::size_t>(ShooterCore::SpriteId::TileHazardLava)] = SDL_Rect { 80, 64, 16, 16 };
}

SDL_Rect SpriteAtlas::getSourceRect(ShooterCore::SpriteId id) const noexcept
{
    const auto idx { static_cast<std::size_t>(id) };
    if (idx < spriteRects_.size()) {
        return spriteRects_[idx];
    }
    return SDL_Rect { 0, 0, 0, 0 };
}

SDL_Rect SpriteAtlas::getTileSourceRect(ShooterCore::TileType tile) const noexcept
{
    const auto idx { static_cast<std::size_t>(tile) };
    if (idx < tileRects_.size()) {
        return tileRects_[idx];
    }
    return SDL_Rect { 0, 0, 0, 0 };
}

} // namespace SdlAdapter
