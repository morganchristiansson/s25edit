// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../Texture.h"
#include "CWindow.h"
#include <vector>

class CMinimapWindow final : public CWindow
{
    std::vector<uint32_t> pixels_; ///< Pixel buffer for minimap terrain
    Texture minimapTex_;

    void Draw(Position parentOrigin) override;

public:
    using CWindow::CWindow;
};
