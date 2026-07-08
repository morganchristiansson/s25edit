// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Texture.h"
#include "defines.h"
#include "globals.h"
#include <glad/glad.h>
#include <algorithm>
#include <array>
#include <set>
#include <utility>
#include <vector>

Texture::~Texture()
{
    if(texture_)
        glDeleteTextures(1, &texture_);
}

Texture::Texture(Texture&& other) noexcept
    : texture_(std::exchange(other.texture_, 0)), size_(std::exchange(other.size_, {0, 0}))
{}

Texture& Texture::operator=(Texture&& other) noexcept
{
    if(this != &other)
    {
        if(texture_)
            glDeleteTextures(1, &texture_);
        texture_ = std::exchange(other.texture_, 0);
        size_ = std::exchange(other.size_, {0, 0});
    }
    return *this;
}

void Texture::load(const void* bgraPixels, Extent size, bool filterLinear)
{
    if(texture_)
        glDeleteTextures(1, &texture_);
    texture_ = 0;
    size_ = {0, 0};

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterLinear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterLinear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_BGRA, GL_UNSIGNED_BYTE, bgraPixels);
    size_ = size;
}

void Texture::createEmpty(Extent size, bool filterLinear)
{
    load(nullptr, size, filterLinear);
}

void Texture::upload(const void* bgraPixels)
{
    if(!texture_)
        return;
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size_.x, size_.y, GL_BGRA, GL_UNSIGNED_BYTE, bgraPixels);
}

bool Texture::load(SDL_Surface* surface, bool filterLinear)
{
    if(!surface)
        return false;

    // For 8-bit paletted surfaces, honour colorkey if set.
    if(surface->format->palette)
    {
        const int w = surface->w, h = surface->h;
        std::vector<Uint32> pixels(static_cast<size_t>(w) * h);

        SDL_Palette* pal = surface->format->palette;
        Uint32 ck;
        const bool hasCK = SDL_GetColorKey(surface, &ck) == 0;
        const Uint8 ckIdx = hasCK ? static_cast<Uint8>(ck & 0xFF) : 0;

        // Water color keys used by the ice floe rendering (two-pass alpha test).
        // These are the RGB values decoded from the old 32-bit SGE colorkeys
        // that were used to punch holes in the snow/swamp texture so that
        // the animated water underneath shows through.
        static constexpr std::array<SDL_Color, 5> kWaterColorKeys = {{
          {0, 55, 111, 0},
          {0, 55, 115, 0},
          {0, 51, 111, 0},
          {0, 51, 103, 0},
          {0, 43, 111, 0},
        }};

        // Build set of transparent palette indices.
        // Start with the SDL colorkey (if any), then add any palette entries
        // that match the water color keys.
        std::set<Uint8> transparentIdxs;
        if(hasCK)
            transparentIdxs.insert(ckIdx);
        for(const auto& key : kWaterColorKeys)
        {
            for(int i = 0; i < 256; i++)
            {
                if(pal->colors[i].r == key.r && pal->colors[i].g == key.g && pal->colors[i].b == key.b)
                {
                    transparentIdxs.insert(static_cast<Uint8>(i));
                    break;
                }
            }
        }

        SDL_LockSurface(surface);
        for(int row = 0; row < h; row++)
        {
            const auto* src = (const Uint8*)surface->pixels + row * surface->pitch;
            for(int col = 0; col < w; col++)
            {
                const Uint8 idx = src[col];
                if(transparentIdxs.count(idx))
                    pixels[row * w + col] = 0; // transparent
                else
                {
                    const SDL_Color& c = pal->colors[idx];
                    // BGRA layout: A<<24 | R<<16 | G<<8 | B (little-endian GL_BGRA)
                    pixels[row * w + col] = (0xFFu << 24) | (Uint32(c.r) << 16) | (Uint32(c.g) << 8) | Uint32(c.b);
                }
            }
        }
        SDL_UnlockSurface(surface);

        load(pixels.data(), Extent(w, h), filterLinear);
        return true;
    }

    // Only 8-bit paletted surfaces are used (LBM files).
    // 32-bit surfaces are not expected from any current caller.
    return false;
}

void Texture::draw(const Rect& destRect) const
{
    if(!texture_)
        return;

    glColor4f(1, 1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2i(destRect.left, destRect.top);
    glTexCoord2f(1, 0);
    glVertex2i(destRect.right, destRect.top);
    glTexCoord2f(1, 1);
    glVertex2i(destRect.right, destRect.bottom);
    glTexCoord2f(0, 1);
    glVertex2i(destRect.left, destRect.bottom);
    glEnd();
}

void Texture::draw(Position pos) const
{
    if(!texture_)
        return;

    glColor4f(1, 1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2i(pos.x, pos.y);
    glTexCoord2f(1, 0);
    glVertex2i(pos.x + size_.x, pos.y);
    glTexCoord2f(1, 1);
    glVertex2i(pos.x + size_.x, pos.y + size_.y);
    glTexCoord2f(0, 1);
    glVertex2i(pos.x, pos.y + size_.y);
    glEnd();
}

void Texture::drawTiled(const Rect& destRect) const
{
    if(!texture_)
        return;

    const Extent tileSize = getSize();
    if(tileSize.x == 0 || tileSize.y == 0)
        return;

    glBindTexture(GL_TEXTURE_2D, texture_);
    glBegin(GL_QUADS);
    for(int y = destRect.top; y < destRect.bottom; y += static_cast<int>(tileSize.y))
    {
        const int rowH = std::min(static_cast<int>(tileSize.y), destRect.bottom - y);
        const float v1 = float(rowH) / float(tileSize.y);
        for(int x = destRect.left; x < destRect.right; x += static_cast<int>(tileSize.x))
        {
            const int colW = std::min(static_cast<int>(tileSize.x), destRect.right - x);
            const float u1 = float(colW) / float(tileSize.x);

            glTexCoord2f(0, 0);
            glVertex2i(x, y);
            glTexCoord2f(u1, 0);
            glVertex2i(x + colW, y);
            glTexCoord2f(u1, v1);
            glVertex2i(x + colW, y + rowH);
            glTexCoord2f(0, v1);
            glVertex2i(x, y + rowH);
        }
    }
    glEnd();
}

void Texture::draw(const Rect& destRect, const Rect& srcRect) const
{
    if(!texture_)
        return;

    // Clamp source rect to texture bounds
    int srcL = std::max(0, srcRect.left);
    int srcT = std::max(0, srcRect.top);
    int srcR = std::min(static_cast<int>(size_.x), srcRect.right);
    int srcB = std::min(static_cast<int>(size_.y), srcRect.bottom);
    if(srcL >= srcR || srcT >= srcB)
        return;

    const float u0 = float(srcL) / float(size_.x);
    const float v0 = float(srcT) / float(size_.y);
    const float u1 = float(srcR) / float(size_.x);
    const float v1 = float(srcB) / float(size_.y);

    glColor4f(1, 1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glBegin(GL_QUADS);
    glTexCoord2f(u0, v0);
    glVertex2i(destRect.left, destRect.top);
    glTexCoord2f(u1, v0);
    glVertex2i(destRect.right, destRect.top);
    glTexCoord2f(u1, v1);
    glVertex2i(destRect.right, destRect.bottom);
    glTexCoord2f(u0, v1);
    glVertex2i(destRect.left, destRect.bottom);
    glEnd();
}

void drawRect(const Rect& rect, unsigned color)
{
    glDisable(GL_TEXTURE_2D);
    glColor4ub((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, (color >> 24) & 0xFF);
    glBegin(GL_QUADS);
    glVertex2i(rect.left, rect.top);
    glVertex2i(rect.right, rect.top);
    glVertex2i(rect.right, rect.bottom);
    glVertex2i(rect.left, rect.bottom);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    glColor4f(1, 1, 1, 1);
}

Texture& getBmpTexture(int idx, bool filterLinear)
{
    if(idx < 0 || static_cast<size_t>(idx) >= global::bmpArray.size())
    {
        static Texture dummy;
        return dummy;
    }
    return Texture::getBmpTexture(idx, filterLinear);
}

void drawButtonBox(const Rect& area, bool pressed, unsigned baseTex, unsigned faceTex)
{
    getBmpTexture(baseTex).drawTiled(area);

    const auto sz = area.getSize();

    // 2px black frame: left+top if pressed, right+bottom otherwise
    if(pressed)
    {
        drawRect(Rect(area.getOrigin(), 2, sz.y), 0xFF000000);
        drawRect(Rect(area.getOrigin(), sz.x, 2), 0xFF000000);
    } else
    {
        drawRect(Rect(area.right - 2, area.top, 2, sz.y), 0xFF000000);
        drawRect(Rect(area.left, area.bottom - 2, sz.x, 2), 0xFF000000);
    }

    // Foreground inset by 2px
    const Rect fgRect(area.getOrigin() + Position(2, 2), sz - Extent(4, 4));
    getBmpTexture(faceTex).drawTiled(fgRect);
}

// ---------------------------------------------------------------------------
//  Bitmap-texture cache (static members)
// ---------------------------------------------------------------------------

std::vector<std::unique_ptr<Texture>> Texture::s_bmpTexCache;
std::vector<bool> Texture::s_bmpTexLinearFlags;

Texture& Texture::getBmpTexture(int idx, bool filterLinear)
{
    if(idx < 0 || idx >= static_cast<int>(global::bmpArray.size()))
    {
        static Texture dummy;
        return dummy;
    }
    if(static_cast<int>(s_bmpTexCache.size()) <= idx)
    {
        s_bmpTexCache.resize(idx + 1);
        s_bmpTexLinearFlags.resize(idx + 1, false);
    }
    if(!s_bmpTexCache[idx] || s_bmpTexLinearFlags[idx] != filterLinear || !s_bmpTexCache[idx]->isValid())
    {
        if(!s_bmpTexCache[idx])
            s_bmpTexCache[idx] = std::make_unique<Texture>();
        s_bmpTexLinearFlags[idx] = filterLinear;
        auto& bmp = global::bmpArray[idx];
        if(bmp.surface)
        {
            s_bmpTexCache[idx]->load(bmp.surface.get(), filterLinear);
            s_bmpTexCache[idx]->anchorX_ = bmp.nx;
            s_bmpTexCache[idx]->anchorY_ = bmp.ny;
        }
    }
    return *s_bmpTexCache[idx];
}

void Texture::drawSprite(int baseX, int baseY) const
{
    if(!isValid())
        return;
    draw(Position(baseX - anchorX_, baseY - anchorY_));
}

void Texture::ensureBmpTex(int idx)
{
    if(idx < 0 || idx >= static_cast<int>(global::bmpArray.size()))
        return;
    getBmpTexture(idx);
}

void Texture::invalidateBmpCache(int start, int end)
{
    if(start > end)
        return;
    if(end >= static_cast<int>(s_bmpTexCache.size()))
        end = static_cast<int>(s_bmpTexCache.size()) - 1;
    for(int i = start; i <= end; i++)
    {
        if(i >= 0 && i < static_cast<int>(s_bmpTexCache.size()))
            s_bmpTexCache[i].reset();
    }
}
