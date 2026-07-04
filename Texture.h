// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Rect.h"
#include <Point.h>
#include <SDL.h>

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

    /// Draw a sub-rectangle of the texture to fill the given destination rect.
    /// srcRect is in texture-local coordinates (may be clipped).
    void draw(const Rect& destRect, const Rect& srcRect) const;

    /// Returns the raw GL texture name (for use with glBindTexture).
    unsigned getHandle() const { return texture_; }

    /// Size in pixels.
    Extent getSize() const { return size_; }

    /// Width in pixels.
    int getWidth() const { return size_.x; }

    /// Height in pixels.
    int getHeight() const { return size_.y; }

    /// Returns true if the texture has been created.
    bool isValid() const { return texture_ != 0; }

private:
    unsigned int texture_ = 0;
    Extent size_;

    /// Internal: create or recreate texture from raw BGRA pixel data.
    void load(const void* bgraPixels, Extent size, bool filterLinear);
};

// ---------------------------------------------------------------------------
//  Free functions for simple GL drawing (rect, line) used by UI components.
// ---------------------------------------------------------------------------

/// Draw a filled rectangle.
void drawRect(const Rect& rect, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

/// Draw a filled rectangle with a 32-bit ARGB colour.
void drawRect(const Rect& rect, unsigned color);

/// Draw a 1-pixel-wide line.
void drawLine(Position p1, Position p2, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

// ---------------------------------------------------------------------------
//  Texture-drawing helpers for UI components
// ---------------------------------------------------------------------------

/// Ensure the OpenGL texture for a bitmap index is loaded from its SDL surface.
void ensureBmpTex(int idx);

/// Draw a bitmap texture tiled to fill the given rectangle.
void drawTiledBmp(int bmpIdx, const Rect& destRect);

/// Draw a 3D-style button box: tiled background, 2px black frame (sunken if pressed, raised otherwise),
/// and tiled foreground inset by 2px.
void drawButtonBox(const Rect& area, bool pressed, int baseTex, int faceTex);

/// Get or create the cached OpenGL texture for a bitmap index.
/// The texture is loaded from the SDL surface on first access.
/// @param filterLinear Whether to use linear filtering (for scaled backgrounds).
Texture& getBmpTexture(int idx, bool filterLinear = false);
