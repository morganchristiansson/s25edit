// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include <functional>
#include <memory>
#include <vector>

class CFont;
class CButton;

class CSelectBox
{
    friend class CDebug;

private:
    std::vector<std::unique_ptr<CFont>> Entries;
    Position pos_;
    Extent size_;
    FontSize fontsize;
    int pic_background;
    int pic_foreground;
    FontColor text_color;
    std::unique_ptr<CButton> ScrollUpButton;
    std::unique_ptr<CButton> ScrollDownButton;
    Uint16 last_text_pos_y = 10;

public:
    CSelectBox(Position pos, Extent size, FontSize fontsize = FontSize::Large, FontColor text_color = FontColor::Yellow,
               int bg_color = -1);
    const Position& getPos() const { return pos_; }
    const Extent& getSize() const { return size_; }
    void setMouseData(SDL_MouseButtonEvent button);
    void setMouseData(SDL_MouseMotionEvent motion);
    void draw(Position parentOrigin);
    void setColor(int color);
    void setTextColor(FontColor color) { text_color = color; }
    void addOption(const std::string& string, std::function<void(int)> callback = nullptr, int param = 0);
    void setSize(Extent size);
    void setPos(Position pos);
};
