// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "GameWorldEditor.h"
#include "world/GameWorldViewer.h"
#include "world/GameWorld.h"
#include "gameData/MapConsts.h"
#include <glad/glad.h>

void GameWorldEditor::MoveBy(const DrawPoint& delta)
{
    offset_ += delta;
    WrapOffset();
}

void GameWorldEditor::SetOffset(const DrawPoint& offset)
{
    offset_ = offset;
    WrapOffset();
}

void GameWorldEditor::WrapOffset()
{
    // Wrap offset modulo map size — same as GameWorldView::MoveTo
    DrawPoint size(world_.GetWidth() * TR_W, world_.GetHeight() * TR_H);
    if(size.x && size.y)
    {
        offset_.x %= size.x;
        offset_.y %= size.y;
        if(offset_.x < 0)
            offset_.x += size.x;
        if(offset_.y < 0)
            offset_.y += size.y;
    }
}

void GameWorldEditor::Draw(const Extent& screenSize)
{
    // Same tile range calculation as GameWorldView::CalcFxLx()
    int firstX = offset_.x / TR_W - 1;
    int firstY = offset_.y / TR_H - 1;
    int lastX = (offset_.x + screenSize.x) / TR_W + 1;
    int lastY = (offset_.y + screenSize.y + viewer_.getMaxNodeAltitude() * HEIGHT_FACTOR) / TR_H + 1;

    Position firstPt(firstX, firstY);
    Position lastPt(lastX, lastY);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screenSize.x, screenSize.y, 0, -100, 100);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Translate modelview to scroll position — same as GameWorldView
    glTranslatef(static_cast<GLfloat>(-offset_.x), static_cast<GLfloat>(-offset_.y), 0.0f);

    viewer_.GetTerrainRenderer().Draw(firstPt, lastPt, viewer_, nullptr);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
