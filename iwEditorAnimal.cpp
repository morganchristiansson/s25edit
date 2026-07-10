// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwEditorAnimal.h"
#include "Loader.h"
#include "controls/ctrlButton.h"
#include "defines.h"

iwEditorAnimal::iwEditorAnimal()
    : IngameWindow(105, IngameWindow::posLastOrCenter, Extent(116, 106), "Animals",
                   LOADER.GetImageN("resource", 41))
{
    const int x0 = contentOffset.x;
    const int y0 = contentOffset.y;
    const int step = 34;
    const int picSize = 32;

    static constexpr int animalIds[] = {
        PICTURE_ANIMAL_RABBIT,
        PICTURE_ANIMAL_FOX,
        PICTURE_ANIMAL_STAG,
        PICTURE_ANIMAL_ROE,
        PICTURE_ANIMAL_DUCK,
        PICTURE_ANIMAL_SHEEP,
    };

    for(unsigned i = 0; i < sizeof(animalIds) / sizeof(animalIds[0]); i++)
    {
        auto* img = LOADER.GetImageN("editio", animalIds[i]);
        if(!img) continue;
        int col = i % 3;
        int row = i / 3;
        AddImageButton(i + 1, DrawPoint(x0 + col * step, y0 + row * step), Extent(picSize, picSize),
                       TextureColor::Invisible, img, "")->SetBorder(false);
    }
}
