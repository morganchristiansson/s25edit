// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CMinimapWindow.h"
#include "../CGame.h"
#include "../CMap.h"
#include "../Texture.h"
#include "../globals.h"

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

    // Draw minimap terrain overlay onto a temporary SDL surface, then upload to texture
    if(auto* map = global::s2->getMapObj())
    {
        // Create or resize the minimap surface
        if(!minimapSurface_ || minimapSurface_->w != contentW || minimapSurface_->h != contentH)
            minimapSurface_ = makeRGBSurface(static_cast<unsigned>(contentW), static_cast<unsigned>(contentH), true);

        if(minimapSurface_)
        {
            // Clear with transparency
            SDL_FillRect(minimapSurface_.get(), nullptr, SDL_MapRGBA(minimapSurface_->format, 0, 0, 0, 0));

            // Draw minimap onto the temporary surface
            map->drawMinimap(minimapSurface_.get());

            // Upload to texture and draw
            minimapTex_.load(minimapSurface_.get());
            minimapTex_.Draw(Rect(contentX, contentY, contentW, contentH));
        }
    }
}
