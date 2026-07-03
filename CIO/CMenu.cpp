// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CMenu.h"
#include "../CGame.h"
#include "../Texture.h"
#include "../globals.h"

CMenu::CMenu(int pic_background) : CControlContainer(pic_background) {}

void CMenu::Draw(Position /*parentOrigin*/)
{
    // Draw full-screen background texture
    const int picIdx = getBackground();
    if(picIdx >= 0 && picIdx < static_cast<int>(global::bmpArray.size()))
    {
        auto& tex = getBmpTexture(picIdx, true);
        if(tex.isValid())
        {
            const auto res = global::s2->getRes();
            tex.Draw(Rect(0, 0, res.x, res.y));
        }
    }

    // Draw children
    DrawChildren(Position(0, 0));
}
