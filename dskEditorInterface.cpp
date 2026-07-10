// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dskEditorInterface.h"
#include "CGame.h"
#include "EditorWorld.h"
#include "Loader.h"
#include "WindowManager.h"
#include "controls/ctrlButton.h"
#include "defines.h"
#include "driver/MouseCoords.h"
#include "drivers/VideoDriverWrapper.h"
#include "dskMainMenu.h"
#include "globals.h"
#include "ogl/glArchivItem_Bitmap.h"
#include "s25util/colors.h"
#include <glad/glad.h>

dskEditorInterface::dskEditorInterface(std::unique_ptr<EditorWorld> world)
    : Desktop(nullptr), world_(std::move(world))
{
    AddTextButton(ID_btQuitEditor, DrawPoint(10, 10), Extent(100, 22), TextureColor::Red1,
                  "Main menu", NormalFont);

    // Bottom menubar tool buttons — invisible border, icon drawn by control
    const auto screenSize = VIDEODRIVER.GetRenderSize();
    const int cx = static_cast<int>(screenSize.x / 2);
    auto addTool = [&](unsigned id, int texIdx, int xOff) {
        auto* img = LOADER.GetImageN("editio", texIdx);
        if(!img) return;
        AddImageButton(id, DrawPoint(cx + xOff, screenSize.y - 36), Extent(37, 32),
                       TextureColor::Invisible, img, "")->SetBorder(false);
    };
    addTool(ID_btToolHeightRaise, MENUBAR_HEIGHT,    -236);
    addTool(ID_btToolTexture,     MENUBAR_TEXTURE,   -199);
    addTool(ID_btToolTree,        MENUBAR_TREE,      -162);
    addTool(ID_btToolResource,    MENUBAR_RESOURCE,  -125);
    addTool(ID_btToolLandscape,   MENUBAR_LANDSCAPE, -88);
    addTool(ID_btToolAnimal,      MENUBAR_ANIMAL,    -51);
    addTool(ID_btToolPlayer,      MENUBAR_PLAYER,    -14);
    addTool(ID_btToolCut,         MENUBAR_BUILDHELP, 92);
    addTool(ID_btToolFlag,        MENUBAR_MINIMAP,   129);
    addTool(ID_btToolHeightReduce,MENUBAR_NEWWORLD,  166);
    addTool(ID_btToolSettings,    MENUBAR_COMPUTER,  203);

    // Right menubar: Load/Save buttons
    const int rx = static_cast<int>(screenSize.x);
    const int ry = static_cast<int>(screenSize.y) / 2;
    AddImageButton(ID_btRLoad, DrawPoint(rx - 36, ry + 163), Extent(32, 37),
                   TextureColor::Invisible,
                   LOADER.GetImageN("editio", MENUBAR_BUGKILL), "")->SetBorder(false);
    AddImageButton(ID_btRSave, DrawPoint(rx - 36, ry + 200), Extent(32, 37),
                   TextureColor::Invisible,
                   LOADER.GetImageN("editio", MENUBAR_BUGKILL), "")->SetBorder(false);
}

dskEditorInterface::~dskEditorInterface() = default;

void dskEditorInterface::Msg_PaintBefore()
{
    Desktop::Msg_PaintBefore();

    if(!world_)
        return;

    const auto screenSize = VIDEODRIVER.GetRenderSize();
    const int w = static_cast<int>(screenSize.x);
    const int h = static_cast<int>(screenSize.y);

    // ── Terrain ──
    {
        Position firstPt(0, 0);
        Position lastPt(
          std::min<int>(world_->getWorld().GetWidth() - 1, screenSize.x / 56 + 2),
          std::min<int>(world_->getWorld().GetHeight() - 1, screenSize.y / 28 + 2));
        glMatrixMode(GL_PROJECTION);
        glPushMatrix(); glLoadIdentity();
        glOrtho(0, screenSize.x, screenSize.y, 0, -100, 100);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix(); glLoadIdentity();
        world_->draw(firstPt, lastPt);
        glMatrixMode(GL_PROJECTION); glPopMatrix();
        glMatrixMode(GL_MODELVIEW); glPopMatrix();
    }

    // ── Editor frame (glArchivItem_Bitmap from "editres") ──
    auto* frameTex = LOADER.GetImageN("editres", MAINFRAME_640_480);
    if(!frameTex) return;

    if(w == 640 && h == 480)
        frameTex->DrawFull(DrawPoint(0, 0));
    else if(w == 800 && h == 600)
        LOADER.GetImageN("editres", MAINFRAME_800_600)->DrawFull(DrawPoint(0, 0));
    else if(w == 1024 && h == 768)
        LOADER.GetImageN("editres", MAINFRAME_1024_768)->DrawFull(DrawPoint(0, 0));
    else if(w == 1280 && h == 1024)
    {
        LOADER.GetImageN("editres", MAINFRAME_LEFT_1280_1024)->DrawFull(DrawPoint(0, 0));
        LOADER.GetImageN("editres", MAINFRAME_RIGHT_1280_1024)->DrawFull(DrawPoint(640, 0));
    } else
    {
        // Corners
        frameTex->Draw(Rect(0, 0, 150, 150), Rect(0, 0, 150, 150), COLOR_WHITE);
        frameTex->Draw(Rect(0, h - 150, 150, 150), Rect(0, 480 - 150, 150, 150), COLOR_WHITE);
        frameTex->Draw(Rect(w - 150, 0, 150, 150), Rect(640 - 150, 0, 150, 150), COLOR_WHITE);
        frameTex->Draw(Rect(w - 150, h - 150, 150, 150), Rect(640 - 150, 480 - 150, 150, 150), COLOR_WHITE);
        // Top/bottom edges
        for(unsigned x = 150; x + 150 < static_cast<unsigned>(w); x += 150)
        {
            frameTex->Draw(Rect(static_cast<int>(x), 0, 150, 12), Rect(150, 0, 150, 12), COLOR_WHITE);
            frameTex->Draw(Rect(static_cast<int>(x), h - 12, 150, 12), Rect(150, 0, 150, 12), COLOR_WHITE);
        }
        // Left/right edges
        for(unsigned y = 150; y + 150 < static_cast<unsigned>(h); y += 150)
        {
            frameTex->Draw(Rect(0, static_cast<int>(y), 12, 150), Rect(0, 150, 12, 150), COLOR_WHITE);
            frameTex->Draw(Rect(w - 12, static_cast<int>(y), 12, 150), Rect(0, 150, 12, 150), COLOR_WHITE);
        }
    }

    // Corner statues — DrawPart with (0,0) offset to match old Texture::draw(Position)
    auto drawStat = [&](int idx, int x, int y) {
        auto* img = LOADER.GetImageN("editres", idx);
        if(!img) return;
        auto sz = img->GetSize();
        img->DrawPart(Rect(x, y, sz.x, sz.y), DrawPoint(0, 0));
    };
    drawStat(STATUE_UP_LEFT,    12, 12);
    drawStat(STATUE_UP_RIGHT,   w - static_cast<int>(LOADER.GetImageN("editres", STATUE_UP_RIGHT)->GetSize().x) - 12, 12);
    drawStat(STATUE_DOWN_LEFT,  12, h - static_cast<int>(LOADER.GetImageN("editres", STATUE_DOWN_LEFT)->GetSize().y) - 12);
    drawStat(STATUE_DOWN_RIGHT, w - static_cast<int>(LOADER.GetImageN("editres", STATUE_DOWN_RIGHT)->GetSize().x) - 12,
             h - static_cast<int>(LOADER.GetImageN("editres", STATUE_DOWN_RIGHT)->GetSize().y) - 12);

    // ── Menubars ──
    const int cx = w / 2;
    auto* menubar = LOADER.GetImageN("editres", MENUBAR);
    if(!menubar) return;
    auto mbSz = menubar->GetSize();

    // Bottom menubar — top-left origin
    menubar->DrawPart(Rect((w - mbSz.x) / 2, h - mbSz.y, mbSz.x, mbSz.y), DrawPoint(0, 0));

    // Slot backgrounds from "editio"
    auto* slotBg = LOADER.GetImageN("editio", BUTTON_GREEN1_DARK);
    if(slotBg)
    {
        for(int xOff = -236; xOff <= -14; xOff += 37)
            slotBg->Draw(Rect(cx + xOff, h - 36, 37, 32), Rect(0, 0, 37, 32), COLOR_WHITE);
        for(int xOff = 92; xOff <= 203; xOff += 37)
            slotBg->Draw(Rect(cx + xOff, h - 36, 37, 32), Rect(0, 0, 37, 32), COLOR_WHITE);
    }

    // Right menubar (rotated)
    {
        const int rx = w;
        const int ry = h / 2;
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glTranslatef(static_cast<float>(rx - mbSz.y / 2 + 1), static_cast<float>(ry + 2), 0.f);
        glRotatef(270.f, 0.f, 0.f, 1.f);
        menubar->DrawFull(Rect(-static_cast<int>(mbSz.x) / 2, -static_cast<int>(mbSz.y) / 2, mbSz.x, mbSz.y));
        glPopMatrix();

        // Right menubar slots
        if(slotBg)
        {
            for(int yOff : {-239, -202, -165, -128, -22, 15, 52, 89, 126, 163, 200})
                slotBg->Draw(Rect(rx - 36, ry + yOff, 32, 37), Rect(0, 0, 32, 37), COLOR_WHITE);
        }

        // Right menubar arrow indicators drawn on top of slots
        {
            auto& texUp = Texture::getTexture(ArchiveID::EDITBOB, CURSOR_SYMBOL_ARROW_UP);
            auto& texDn = Texture::getTexture(ArchiveID::EDITBOB, CURSOR_SYMBOL_ARROW_DOWN);
            if(texUp.isValid() && texDn.isValid())
            {
                texUp.draw(Position(rx - 33, ry - 237));
                texDn.draw(Position(rx - 20, ry - 235));
                texDn.draw(Position(rx - 33, ry - 220));
                texUp.draw(Position(rx - 20, ry - 220));
            }
        }
    }
}

void dskEditorInterface::Draw_()
{
    Desktop::Draw_();
}

void dskEditorInterface::Msg_ButtonClick(unsigned ctrl_id)
{
    switch(ctrl_id)
    {
        case ID_btQuitEditor:
            world_.reset();
            WINDOWMANAGER.Switch(std::make_unique<dskMainMenu>());
            break;
    }
}

bool dskEditorInterface::Msg_LeftDown(const MouseCoords& mc) { return Desktop::Msg_LeftDown(mc); }
bool dskEditorInterface::Msg_LeftUp(const MouseCoords& mc) { return Desktop::Msg_LeftUp(mc); }
bool dskEditorInterface::Msg_MouseMove(const MouseCoords&) { return false; }
bool dskEditorInterface::Msg_KeyDown(const KeyEvent&) { return false; }
bool dskEditorInterface::Msg_WheelUp(const MouseCoords&) { return false; }
bool dskEditorInterface::Msg_WheelDown(const MouseCoords&) { return false; }
