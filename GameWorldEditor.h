// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "DrawPoint.h"
#include "Point.h"
#include <glad/glad.h>

#include "Point.h"

class GameWorld;
class GameWorldViewer;

/// Editor-specific world view — sibling of GameWorldView, no game cruft.
class GameWorldEditor
{
public:
    GameWorldEditor(GameWorldViewer& viewer, GameWorld& world, const DrawPoint& offset = DrawPoint(0, 0))
        : viewer_(viewer), world_(world), offset_(offset) {}

    void MoveBy(const DrawPoint& delta);
    void SetOffset(const DrawPoint& offset);
    DrawPoint GetOffset() const { return offset_; }

    void Draw(const Extent& screenSize);

private:
    void WrapOffset();

    GameWorldViewer& viewer_;
    GameWorld& world_;
    DrawPoint offset_;
};
