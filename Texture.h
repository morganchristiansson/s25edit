// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "Rect.h"
#include <Point.h>
#include <SDL.h>

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

    bool load(SDL_Surface* surface, bool filterLinear = false);

    void createEmpty(Extent size, bool filterLinear = false);

    void upload(const void* bgraPixels);

    void draw(const Rect& destRect) const;

    void draw(Position pos) const;

    void drawTiled(const Rect& destRect) const;

    unsigned getHandle() const { return texture_; }

    Extent getSize() const { return size_; }

    bool isValid() const { return texture_ != 0; }

private:
    unsigned int texture_ = 0;
    Extent size_;

    void load(const void* bgraPixels, Extent size, bool filterLinear);
};

void drawRect(const Rect& rect, unsigned color);

void drawButtonBox(const Rect& area, bool pressed, unsigned baseTex, unsigned faceTex);

Texture& getBmpTexture(unsigned idx, bool filterLinear = false);
