// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorTree.h"
#include "Loader.h"
#include "controls/ctrlButton.h"
#include "defines.h"

iwEditorTree::iwEditorTree()
    : IngameWindow(102, IngameWindow::posLastOrCenter, Extent(148, 140), "Trees",
                   LOADER.GetImageN("resource", 41))
{
    const int x0 = contentOffset.x;
    const int y0 = contentOffset.y;
    const int step = 34;
    const int picSize = 32;

    // Greenland trees (most complete set)
    static constexpr int treeIds[] = {
        PICTURE_TREE_PINE,
        PICTURE_TREE_BIRCH,
        PICTURE_TREE_OAK,
        PICTURE_TREE_PALM1,
        PICTURE_TREE_PALM2,
        PICTURE_TREE_PINEAPPLE,
        PICTURE_TREE_CYPRESS,
        PICTURE_TREE_CHERRY,
        PICTURE_TREE_FIR,
        PICTURE_TREE_WOOD_MIXED,
        PICTURE_TREE_PALM_MIXED,
    };

    for(unsigned i = 0; i < sizeof(treeIds) / sizeof(treeIds[0]); i++)
    {
        auto* img = LOADER.GetImageN("editio", treeIds[i]);
        if(!img) continue;
        int col = i % 4;
        int row = i / 4;
        AddImageButton(i + 1, DrawPoint(x0 + col * step, y0 + row * step), Extent(picSize, picSize),
                       TextureColor::Invisible, img, "")->SetBorder(false);
    }
}
