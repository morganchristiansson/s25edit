// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorPlayer.h"
#include "Loader.h"
#include "controls/ctrlButton.h"

iwEditorPlayer::iwEditorPlayer()
    : IngameWindow(106, IngameWindow::posLastOrCenter, Extent(100, 80), "Players",
                   LOADER.GetImageN("resource", 41))
{
    const int x0 = contentOffset.x;
    const int y0 = contentOffset.y;

    AddTextButton(1, DrawPoint(x0, y0), Extent(20, 20), TextureColor::Grey, "-", NormalFont);
    AddTextButton(2, DrawPoint(x0 + 40, y0), Extent(20, 20), TextureColor::Grey, "+", NormalFont);
    AddTextButton(3, DrawPoint(x0, y0 + 20), Extent(60, 20), TextureColor::Grey, "Go to", NormalFont);
}
