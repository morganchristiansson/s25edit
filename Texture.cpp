// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Texture.h"
#include "defines.h"
#include "globals.h"
#include <glad/glad.h>
#include <algorithm>
#include <memory>
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

        SDL_LockSurface(surface);
        for(int row = 0; row < h; row++)
        {
            const auto* src = (const Uint8*)surface->pixels + row * surface->pitch;
            for(int col = 0; col < w; col++)
            {
                const Uint8 idx = src[col];
                if(hasCK && idx == ckIdx)
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

    // 32-bit surface: convert to destination format (BGRA), preserving colorkey transparency
    SDL_Surface* converted = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_ARGB8888, 0);
    if(!converted)
        return false;

    // Force alpha to opaque (some source images may have wrong alpha=0)
    SDL_LockSurface(converted);
    for(int y = 0; y < converted->h; y++)
    {
        auto* row = (Uint32*)((Uint8*)converted->pixels + y * converted->pitch);
        for(int x = 0; x < converted->w; x++)
            row[x] |= 0xFF000000u;
    }
    SDL_UnlockSurface(converted);

    load(converted->pixels, Extent(converted->w, converted->h), filterLinear);
    SDL_FreeSurface(converted);
    return true;
}

void Texture::draw(const Rect& destRect) const
{
    if(!texture_)
        return;

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

void Texture::draw(const Rect& destRect, const Rect& srcRect) const
{
    if(!texture_)
        return;

    // Clamp source rect to texture bounds
    const Position clampedOrigin(std::max(0, srcRect.left), std::max(0, srcRect.top));
    const Position clampedEnd(std::min(static_cast<int>(size_.x), srcRect.right),
                              std::min(static_cast<int>(size_.y), srcRect.bottom));
    if(clampedOrigin.x >= clampedEnd.x || clampedOrigin.y >= clampedEnd.y)
        return;

    const auto texSize = Position(static_cast<int>(size_.x), static_cast<int>(size_.y));
    const Point<float> uv0 = Point<float>(clampedOrigin) / Point<float>(texSize);
    const Point<float> uv1 = Point<float>(clampedEnd) / Point<float>(texSize);

    glBindTexture(GL_TEXTURE_2D, texture_);
    glBegin(GL_QUADS);
    glTexCoord2f(uv0.x, uv0.y);
    glVertex2i(destRect.left, destRect.top);
    glTexCoord2f(uv1.x, uv0.y);
    glVertex2i(destRect.right, destRect.top);
    glTexCoord2f(uv1.x, uv1.y);
    glVertex2i(destRect.right, destRect.bottom);
    glTexCoord2f(uv0.x, uv1.y);
    glVertex2i(destRect.left, destRect.bottom);
    glEnd();
}

void drawRect(const Rect& rect, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2i(rect.left, rect.top);
    glVertex2i(rect.right, rect.top);
    glVertex2i(rect.right, rect.bottom);
    glVertex2i(rect.left, rect.bottom);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

void drawRect(const Rect& rect, unsigned color)
{
    drawRect(rect, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF, (color >> 24) & 0xFF);
}

void drawLine(Position p1, Position p2, unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    glDisable(GL_TEXTURE_2D);
    glColor4ub(r, g, b, a);
    glBegin(GL_LINES);
    glVertex2i(p1.x, p1.y);
    glVertex2i(p2.x, p2.y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

Texture& getBmpTexture(int idx, bool filterLinear)
{
    static std::vector<std::unique_ptr<Texture>> cache;
    static std::vector<bool> linearFlags;
    if(static_cast<unsigned>(idx) >= global::bmpArray.size())
    {
        static Texture dummy;
        return dummy;
    }
    if(static_cast<unsigned>(idx) >= cache.size())
    {
        cache.resize(static_cast<size_t>(idx) + 1);
        linearFlags.resize(static_cast<size_t>(idx) + 1, false);
    }
    if(!cache[idx] || linearFlags[idx] != filterLinear)
    {
        if(!cache[idx])
            cache[idx] = std::make_unique<Texture>();
        linearFlags[idx] = filterLinear;
        auto& bmp = global::bmpArray[idx];
        if(bmp.surface)
            cache[idx]->load(bmp.surface.get(), filterLinear);
    }
    return *cache[idx];
}

void ensureBmpTex(int idx)
{
    if(static_cast<unsigned>(idx) >= global::bmpArray.size())
        return;
    getBmpTexture(idx);
}

void drawTiledBmp(int bmpIdx, const Rect& destRect)
{
    auto& tex = getBmpTexture(bmpIdx);
    if(!tex.isValid())
        return;

    const unsigned tileW = static_cast<unsigned>(tex.getWidth());
    const unsigned tileH = static_cast<unsigned>(tex.getHeight());
    if(tileW == 0 || tileH == 0)
        return;

    glColor4f(1, 1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, tex.getHandle());
    glBegin(GL_QUADS);
    for(int y = destRect.top; y < destRect.bottom; y += static_cast<int>(tileH))
    {
        const int rowH = std::min(static_cast<int>(tileH), static_cast<int>(destRect.bottom - y));
        const float v1 = static_cast<float>(rowH) / static_cast<float>(tileH);
        for(int x = destRect.left; x < destRect.right; x += static_cast<int>(tileW))
        {
            const int colW = std::min(static_cast<int>(tileW), static_cast<int>(destRect.right - x));
            const float u1 = static_cast<float>(colW) / static_cast<float>(tileW);

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

void drawButtonBox(const Rect& area, bool pressed, int baseTex, int faceTex)
{
    drawTiledBmp(baseTex, area);

    const int w = area.right - area.left;
    const int h = area.bottom - area.top;

    // 2px black frame: left+top if pressed, right+bottom otherwise
    if(pressed)
    {
        drawRect(Rect(area.left, area.top, 2, h), 0, 0, 0);
        drawRect(Rect(area.left, area.top, w, 2), 0, 0, 0);
    } else
    {
        drawRect(Rect(area.right - 2, area.top, 2, h), 0, 0, 0);
        drawRect(Rect(area.left, area.bottom - 2, w, 2), 0, 0, 0);
    }

    // Foreground inset by 2px
    const Rect fgRect(area.getOrigin() + Position(2, 2), Extent(w - 4, h - 4));
    drawTiledBmp(faceTex, fgRect);
}
