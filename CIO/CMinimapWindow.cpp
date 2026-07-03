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
    const auto borderBegin = getBorderBegin();
    const auto borderEnd = getBorderEnd();
    const int contentX = x_ + static_cast<int>(borderBegin.x);
    const int contentY = y_ + static_cast<int>(borderBegin.y);
    const int contentW = static_cast<int>(w_) - static_cast<int>(borderBegin.x) - static_cast<int>(borderEnd.x);
    const int contentH = static_cast<int>(h_) - static_cast<int>(borderBegin.y) - static_cast<int>(borderEnd.y);

    if(contentW <= 0 || contentH <= 0)
        return;

    auto* map = global::s2->getMapObj();
    if(!map)
        return;

    // Fill pixel buffer with minimap terrain
    int num_x = 1, num_y = 1;
    map->drawMinimap(pixels_, contentW, contentH, num_x, num_y);

    // Upload to texture and draw
    if(!minimapTex_.isValid() || minimapTex_.getWidth() != contentW || minimapTex_.getHeight() != contentH)
        minimapTex_.createEmpty(Extent(contentW, contentH));
    minimapTex_.upload(pixels_.data());
    minimapTex_.Draw(Rect(contentX, contentY, contentW, contentH));

    // Draw player flags and numbers on top
    for(int i = 0; i < MAXPLAYERS; i++)
    {
        const auto hqX = map->getPlayerHQx()[i];
        const auto hqY = map->getPlayerHQy()[i];
        if(hqX == 0xFFFF || hqY == 0xFFFF)
            continue;

        const int flagIdx = FLAG_BLUE_DARK + i % 7;
        const auto& flagBmp = global::bmpArray[flagIdx];
        auto& flagTex = getBmpTexture(flagIdx);
        if(flagTex.isValid())
        {
            const int fx = contentX + hqX / num_x - static_cast<int>(flagBmp.nx);
            const int fy = contentY + hqY / num_y - static_cast<int>(flagBmp.ny);
            flagTex.Draw(Position(fx, fy));
        }

        // Player number
        CFont::Draw(std::to_string(i + 1), Position(contentX + hqX / num_x, contentY + hqY / num_y), FontSize::Small,
                    FontColor::MintGreen);
    }

    // Draw the position arrow
    {
        const int arrowIdx = MAPPIC_ARROWCROSS_ORANGE;
        const auto& arrowBmp = global::bmpArray[arrowIdx];
        auto& arrowTex = getBmpTexture(arrowIdx);
        if(arrowTex.isValid())
        {
            const auto& dispRect = map->getDisplayRect();
            const int ax = contentX
                           + (dispRect.left + static_cast<int>(dispRect.getSize().x) / 2) / triangleWidth / num_x
                           - static_cast<int>(arrowBmp.nx);
            const int ay = contentY
                           + (dispRect.top + static_cast<int>(dispRect.getSize().y) / 2) / triangleHeight / num_y
                           - static_cast<int>(arrowBmp.ny);
            arrowTex.Draw(Position(ax, ay));
        }
    }
}
