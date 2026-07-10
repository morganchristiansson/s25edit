// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "GameWorldEditor.h"
#include "EditorWorldLoader.h"
#include "Loader.h"
#include "defines.h"
#include "world/GameWorldViewer.h"
#include "world/GameWorld.h"
#include "TerrainRenderer.h"
#include "gameData/MapConsts.h"
#include "nodeObjs/noBase.h"
#include "nodeObjs/noTree.h"
#include "libsiedler2/Archiv.h"
#include "ogl/glArchivItem_Bitmap.h"
#include <glad/glad.h>
#include <SDL.h>

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

    const auto mapSize = world_.GetSize();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, screenSize.x, screenSize.y, 0, -100, 100);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Translate modelview to scroll position — same as GameWorldView
    glTranslatef(static_cast<GLfloat>(-offset_.x), static_cast<GLfloat>(-offset_.y), 0.0f);

    // Draw terrain
    viewer_.GetTerrainRenderer().Draw(firstPt, lastPt, viewer_, nullptr);

    // Draw objects (trees, animals, buildings, etc.) on top of terrain
    // Same visible range iteration as TerrainRenderer
    const auto& tr = viewer_.GetTerrainRenderer();
    for(int y = firstY; y <= lastY; y++)
    {
        for(int x = firstX; x <= lastX; x++)
        {
            Position rawPt(x, y);
            // Convert to map point with wrapping, get pixel offset for wrap
            Position wrapOffset;
            MapPoint pt = tr.ConvertCoords(rawPt, &wrapOffset);
            if(pt.x >= mapSize.x || pt.y >= mapSize.y)
                continue;

            noBase* obj = world_.GetNO(pt);
            if(!obj)
                continue;

            // World-space pixel position of this node
            Position nodePos = world_.GetNodePos(pt);
            // Apply wrap offset so wrapped copies draw at the correct screen position
            nodePos += wrapOffset;

            // Draw objects using LOADER's MAP00 archive with clean-index lookup
            if(auto* tree = dynamic_cast<noTree*>(obj))
            {
                static Uint32 lastTick = 0;
                static unsigned frame = 0;
                Uint32 now = SDL_GetTicks();
                if(now - lastTick > 80) { frame = (frame + 1) % 8; lastTick = now; }
                int cleanIdx = MAPPIC_TREE_PINE + tree->getTreeType() * 15 + frame;
                int archIdx = editorMapCleanToArchiveIdx(cleanIdx);
                if(archIdx >= 0)
                {
                    auto& arch = LOADER.GetArchive("map00");
                    auto* bmp = dynamic_cast<glArchivItem_Bitmap*>(arch.get(static_cast<unsigned>(archIdx)));
                    if(bmp)
                        bmp->DrawFull(DrawPoint(nodePos.x, nodePos.y));
                }
            } else
            {
                obj->Draw(DrawPoint(nodePos.x, nodePos.y));
            }
        }
    }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
