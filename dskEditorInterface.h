// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "desktops/Desktop.h"
#include <memory>

class EditorWorld;
class GameWorldEditor;

class dskEditorInterface : public Desktop
{
public:
    dskEditorInterface(std::unique_ptr<EditorWorld> world);
    ~dskEditorInterface() override;

    void Msg_ButtonClick(unsigned ctrl_id) override;
    void Msg_PaintBefore() override;
    bool Msg_LeftDown(const MouseCoords& mc) override;
    bool Msg_LeftUp(const MouseCoords& mc) override;
    bool Msg_MouseMove(const MouseCoords& mc) override;
    bool Msg_KeyDown(const KeyEvent& ke) override;
    bool Msg_WheelUp(const MouseCoords& mc) override;
    bool Msg_WheelDown(const MouseCoords& mc) override;

protected:
    void Draw_() override;

private:
    std::unique_ptr<EditorWorld> world_;
    std::unique_ptr<GameWorldEditor> gwEditor_;

    enum ControlIds
    {
        ID_btQuitEditor,
        ID_btSave,
        ID_btLoad,
        ID_btMinimap,
        ID_btToolCut,
        ID_btToolTree,
        ID_btToolHeightRaise,
        ID_btToolHeightReduce,
        ID_btToolTexture,
        ID_btToolLandscape,
        ID_btToolFlag,
        ID_btToolResource,
        ID_btToolAnimal,
        ID_btToolPlayer,
        ID_btEditorMenu,
        ID_FIRST_TOOL = ID_btToolCut,
        ID_LAST_TOOL = ID_btEditorMenu,
        ID_btRLoad,
        ID_btRSave,
    };
};
