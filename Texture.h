// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Rect.h"
#include <glad/glad.h>
#include <Point.h>
#include <SDL.h>
#include <memory>
#include <vector>

/// Wraps a texture with RAII and provides draw methods.
class Texture
{
public:
    Texture() = default;
    ~Texture();

    Texture(Texture&&) noexcept;
    Texture& operator=(Texture&&) noexcept;

    // No copy
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    /// Load from a 32-bit or 8-bit paletted SDL surface.
    /// For 8-bit surfaces, optional colorkey is respected (keyed pixels become transparent).
    bool load(SDL_Surface* surface, bool filterLinear = false);

    /// Create an empty texture of the given size (for use as a render-target).
    void createEmpty(Extent size, bool filterLinear = false);

    /// Upload new pixel data to an existing texture (glTexSubImage2D).
    void upload(const void* bgraPixels);

    /// Draw the texture stretched to fill the given rect.
    void draw(const Rect& destRect) const;

    /// Draw the texture at native size at the given position.
    void draw(Position pos) const;

    /// Tile the texture to fill the given rectangle.
    void drawTiled(const Rect& destRect) const;

    /// Direct handle access for manual GL ops.
    GLuint getHandle() const { return texture_; }

    /// Size in pixels.
    Extent getSize() const { return size_; }

    /// Draw a sub-rect of the texture stretched to fill the given dest rect.
    void draw(const Rect& destRect, const Rect& srcRect) const;

    /// Returns true if the texture has been created.
    bool isValid() const { return texture_ != 0; }

    /// Draw at native size at (baseX, baseY) adjusted by the sprite anchor.
    void drawSprite(int baseX, int baseY) const;

    // ---- Static bitmap-texture cache (used by CFont::draw) ----

    /// Return (or create on first use) a cached GL texture for a bobBMP entry.
    static Texture& getBmpTexture(int idx, bool filterLinear = false);

    /// Ensure the bitmap at idx has a cached GL texture (pre-warm).
    static void ensureBmpTex(int idx);

    /// Invalidate the texture cache for entries [start, end] so the next getBmpTexture() re-uploads.
    static void invalidateBmpCache(int start, int end);

private:
    GLuint texture_ = 0;
    Extent size_;
    Sint16 anchorX_ = 0, anchorY_ = 0; // sprite anchor offset, set by getBmpTexture

    // ---- Bitmap-texture cache internals ----
    static std::vector<std::unique_ptr<Texture>> s_bmpTexCache;
    static std::vector<bool> s_bmpTexLinearFlags;

    /// Internal: create or recreate texture from raw BGRA pixel data.
    void load(const void* bgraPixels, Extent size, bool filterLinear);
};

/// Draw a filled rectangle with a 32-bit ARGB colour.
void drawRect(const Rect& rect, unsigned color);

/// Draw a 3D-style button box: tiled background, 2px black frame (sunken if pressed, raised otherwise),
/// and tiled foreground inset by 2px.
void drawButtonBox(const Rect& area, bool pressed, unsigned baseTex, unsigned faceTex);

/// Get or create the cached OpenGL texture for a bitmap index.
/// The texture is loaded from the SDL surface on first access.
/// @param filterLinear Whether to use linear filtering (for scaled backgrounds).
Texture& getBmpTexture(int idx, bool filterLinear = false);
