// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorLandscape.h"
#include "Loader.h"
#include "controls/ctrlButton.h"
#include "defines.h"

iwEditorLandscape::iwEditorLandscape()
    : IngameWindow(104, IngameWindow::posLastOrCenter, Extent(112, 174), "Landscape",
                   LOADER.GetImageN("resource", 41))
{
    const int x0 = contentOffset.x;
    const int y0 = contentOffset.y;
    const int step = 34;
    const int picSize = 32;

    // Greenland landscape objects
    static constexpr int landscapeIds[] = {
        PICTURE_LANDSCAPE_GRANITE,
        PICTURE_LANDSCAPE_TREE_DEAD,
        PICTURE_LANDSCAPE_STONE,
        PICTURE_LANDSCAPE_CACTUS,
        PICTURE_LANDSCAPE_PEBBLE,
        PICTURE_LANDSCAPE_BUSH,
        PICTURE_LANDSCAPE_SHRUB,
        PICTURE_LANDSCAPE_BONE,
        PICTURE_LANDSCAPE_MUSHROOM,
    };

    for(unsigned i = 0; i < sizeof(landscapeIds) / sizeof(landscapeIds[0]); i++)
    {
        auto* img = LOADER.GetImageN("editio", landscapeIds[i]);
        if(!img) continue;
        int col = i % 3;
        int row = i / 3;
        AddImageButton(i + 1, DrawPoint(x0 + col * step, y0 + row * step), Extent(picSize, picSize),
                       TextureColor::Invisible, img, "")->SetBorder(false);
    }
}
