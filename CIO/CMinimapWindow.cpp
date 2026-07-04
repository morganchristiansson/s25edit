// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CMinimapWindow.h"
#include "../CGame.h"
#include "../CMap.h"
#include "../Texture.h"
#include "../globals.h"
#include "CFont.h"

void CMinimapWindow::Draw(Position /*parentOrigin*/)
{
    // Draw window chrome (frame, title, close button, background, child elements)
    CWindow::Draw(Position(x_, y_));

    // Compute content area (inside the frames)
    const auto& b = getBorder();
    const Position contentPos(x_ + b.left, y_ + b.top);
    const int cw = static_cast<int>(w_) - b.left - b.right;
    const int ch = static_cast<int>(h_) - b.top - b.bottom;
    if(cw <= 0 || ch <= 0)
        return;
    const Extent contentSize(cw, ch);

    auto* map = global::s2->getMapObj();
    if(!map)
        return;

    // Fill pixel buffer with minimap terrain
    int num_x = 1, num_y = 1;
    map->drawMinimap(pixels_, cw, ch, num_x, num_y);

    // Upload to texture and draw
    if(!minimapTex_.isValid() || minimapTex_.getWidth() != cw || minimapTex_.getHeight() != ch)
        minimapTex_.createEmpty(contentSize);
    minimapTex_.upload(pixels_.data());
    minimapTex_.Draw(Rect(contentPos, contentSize));

    // Draw player flags and numbers on top
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        const auto hqX = map->getPlayerHQx()[i];
        const auto hqY = map->getPlayerHQy()[i];
        if(hqX == 0xFFFF || hqY == 0xFFFF)
            continue;

        const int flagIdx = FLAG_BLUE_DARK + i % 7;
        const Position hqPos(hqX / num_x, hqY / num_y);
        getBmpTexture(flagIdx).Draw(contentPos + hqPos - Position(static_cast<int>(global::bmpArray[flagIdx].nx),
                                                                  static_cast<int>(global::bmpArray[flagIdx].ny)));

        // Player number
        CFont::Draw(std::to_string(i + 1), contentPos + hqPos, FontSize::Small,
                    FontColor::MintGreen);
    }

    // Draw the position arrow
    {
        const int arrowIdx = MAPPIC_ARROWCROSS_ORANGE;
        const auto& dispRect = map->getDisplayRect();
        const Position arrowPos = contentPos
          + Position((dispRect.left + static_cast<int>(dispRect.getSize().x) / 2) / triangleWidth / num_x,
                     (dispRect.top + static_cast<int>(dispRect.getSize().y) / 2) / triangleHeight / num_y)
          - Position(static_cast<int>(global::bmpArray[arrowIdx].nx),
                     static_cast<int>(global::bmpArray[arrowIdx].ny));
        getBmpTexture(arrowIdx).Draw(arrowPos);
    }
}
