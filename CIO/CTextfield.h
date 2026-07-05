// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"
#include <SDL.h>
#include <memory>
#include <vector>

class CFont;

class CTextfield
{
    friend class CDebug;

private:
    std::unique_ptr<CFont> textObj;
    Extent size_;
    Uint16 cols;
    Uint16 rows;
    int pic_background;
    int pic_foreground;
    std::vector<char> text_;
    // if active, keyboard data will be delivered and the cursor is blinking
    bool active;
    // if true, the textfield looks like a button
    bool button_style;
    // Cursor blink state
    bool blinking_chiffre = false;

public:
    // Constructor - Destructor
    CTextfield(Position pos = {0, 0}, Uint16 cols = 10, Uint16 rows = 1, FontSize fontsize = FontSize::Large,
               FontColor text_color = FontColor::Yellow, int bg_color = -1, bool button_style = false);
    // Access
    Position getPos() const;
    void setPos(Position pos);
    const Extent& getSize() const { return size_; };
    int getCols() const { return cols; }
    int getRows() const { return rows; }
    void setText(const std::string& text);
    void setActive() { active = true; }
    void setInactive() { active = false; }
    bool isActive() const { return active; }
    void setMouseData(SDL_MouseButtonEvent button);
    void setKeyboardData(const SDL_KeyboardEvent& key);
    void draw(Position parentOrigin);
    void setColor(int color);
    void setTextColor(FontColor color);
    std::string getText() const { return text_.data(); }
};
