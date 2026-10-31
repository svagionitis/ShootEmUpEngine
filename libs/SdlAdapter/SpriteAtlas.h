#pragma once

#include <cstddef>
#include <cstdint>

#include <array>

#ifndef SDL_MAIN_HANDLED
#define SDL_MAIN_HANDLED
#endif
#include <SDL.h>

#include "Types.h"

namespace SdlAdapter {

/// @class SpriteAtlas
/// @brief Hardware-accelerated 16-color EGA / VGA sprite and tileset atlas.
/// @details Generates an in-memory 256x256 pixel texture containing authentic
///          Thexder retro pixel-art sprites and maps SpriteId to SDL_Rect coords.
class SpriteAtlas final {
public:
    static constexpr int kAtlasWidth { 256 };
    static constexpr int kAtlasHeight { 256 };

    SpriteAtlas() noexcept = default;
    ~SpriteAtlas() noexcept;

    // Non-copyable, movable
    SpriteAtlas(const SpriteAtlas&) = delete;
    SpriteAtlas& operator=(const SpriteAtlas&) = delete;
    SpriteAtlas(SpriteAtlas&& other) noexcept;
    SpriteAtlas& operator=(SpriteAtlas&& other) noexcept;

    /// @brief Initialize the hardware texture atlas on the active renderer.
    /// @param[in] renderer Target SDL hardware renderer.
    /// @return true if texture created and initialized successfully; false on error.
    bool init(SDL_Renderer* renderer) noexcept;

    /// @brief Destroy the allocated texture and reset atlas state.
    void shutdown() noexcept;

    /// @brief Check whether the hardware texture has been initialized.
    /// @return true if texture is valid.
    [[nodiscard]] bool isLoaded() const noexcept;

    /// @brief Retrieve underlying SDL hardware texture.
    /// @return Pointer to SDL_Texture or nullptr if not initialized.
    [[nodiscard]] SDL_Texture* getTexture() const noexcept;

    /// @brief Resolve SpriteId to its texture source rectangle.
    /// @param[in] id Sprite identifier.
    /// @return Source rectangle on the sprite atlas.
    [[nodiscard]] SDL_Rect getSourceRect(ShooterCore::SpriteId id) const noexcept;

    /// @brief Resolve TileType to its texture source rectangle.
    /// @param[in] tile Environmental tile category.
    /// @return Source rectangle on the sprite atlas.
    [[nodiscard]] SDL_Rect getTileSourceRect(ShooterCore::TileType tile) const noexcept;

private:
    void generateAtlasPixels(std::uint32_t* pixelBuffer) noexcept;
    void buildRectTables() noexcept;

    SDL_Texture* atlasTexture_ { nullptr };
    std::array<SDL_Rect, 64U> spriteRects_ { };
    std::array<SDL_Rect, 8U> tileRects_ { };
};

} // namespace SdlAdapter
