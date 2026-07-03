// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../Texture.h"
#include "CWindow.h"

class CMinimapWindow final : public CWindow
{
    /// Temporary SDL surface for minimap terrain overlay (kept until terrain is also OpenGL)
    SdlSurface minimapSurface_;
    Texture minimapTex_;

    void Draw(Position parentOrigin) override;

public:
    using CWindow::CWindow;
};
