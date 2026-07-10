// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorTexture.h"
#include "Loader.h"
#include "controls/ctrlButton.h"
#include "defines.h"

iwEditorTexture::iwEditorTexture()
    : IngameWindow(101, IngameWindow::posLastOrCenter, Extent(220, 133), "Terrain",
                   LOADER.GetImageN("resource", 41))
{
    const int x0 = contentOffset.x;
    const int y0 = contentOffset.y;
    const int step = 34;
    const int picSize = 32;

    static constexpr int texIds[] = {
        PICTURE_GREENLAND_TEXTURE_SNOW,
        PICTURE_GREENLAND_TEXTURE_STEPPE,
        PICTURE_GREENLAND_TEXTURE_SWAMP,
        PICTURE_GREENLAND_TEXTURE_FLOWER,
        PICTURE_GREENLAND_TEXTURE_MINING1,
        PICTURE_GREENLAND_TEXTURE_MINING2,
        PICTURE_GREENLAND_TEXTURE_MINING3,
        PICTURE_GREENLAND_TEXTURE_MINING4,
        PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW1,
        PICTURE_GREENLAND_TEXTURE_MEADOW1,
        PICTURE_GREENLAND_TEXTURE_MEADOW2,
        PICTURE_GREENLAND_TEXTURE_MEADOW3,
        PICTURE_GREENLAND_TEXTURE_STEPPE_MEADOW2,
        PICTURE_GREENLAND_TEXTURE_MINING_MEADOW,
        PICTURE_GREENLAND_TEXTURE_WATER,
        PICTURE_GREENLAND_TEXTURE_LAVA,
        PICTURE_GREENLAND_TEXTURE_MEADOW_MIXED,
    };

    for(unsigned i = 0; i < sizeof(texIds) / sizeof(texIds[0]); i++)
    {
        auto* img = LOADER.GetImageN("editio", texIds[i]);
        if(!img) continue;
        int col = i % 6;
        int row = i / 6;
        AddImageButton(i + 1, DrawPoint(x0 + col * step, y0 + row * step), Extent(picSize, picSize),
                       TextureColor::Invisible, img, "")->SetBorder(false);
    }
}
