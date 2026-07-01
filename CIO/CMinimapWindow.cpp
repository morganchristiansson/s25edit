// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CMinimapWindow.h"
#include "../CGame.h"
#include "../CMap.h"
#include "../globals.h"

bool CMinimapWindow::render()
{
    // Always re-render: the minimap terrain overlay may have changed
    needRender = true;
    // Draw window chrome (frame, title, close button, background, child elements)
    CWindow::render();
    // Draw minimap terrain overlay on top of the chrome
    if(surface)
    {
        if(auto* map = global::s2->getMapObj())
            map->drawMinimap(surface.get());
    }
    return true;
}
