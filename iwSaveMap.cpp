// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iwSaveMap.h"
#include "CGame.h"
#include "CIO/CFile.h"
#include "CMap.h"
#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem;
#include "WindowManager.h"
#include "controls/ctrlButton.h"
#include "controls/ctrlEdit.h"
#include "controls/ctrlText.h"
#include "defines.h"
#include "globals.h"
#include "Loader.h"

iwSaveMap::iwSaveMap()
    : IngameWindow(102, IngameWindow::posLastOrCenter, Extent(280, 200), "Save map",
                   LOADER.GetImageN("resource", 41))
{
    auto off = DrawPoint(contentOffset.x, contentOffset.y);
    int labelX = off.x + (GetSize().x - contentOffset.x - contentOffsetEnd.x) / 2 + contentOffset.x - 30;

    AddText(ID_lblFilename, DrawPoint(labelX, off.y + 2), "Filename", COLOR_YELLOW, FontStyle::CENTER, SmallFont);
    auto* editFile = AddEdit(ID_edtFilename, off + DrawPoint(10, 13), Extent(210, 22), TextureColor::Grey, NormalFont);
    auto* mapObj = global::s2->getMapObj();
    bfs::path defaultPath = mapObj ? mapObj->getFilepath() : "";
    editFile->SetText(defaultPath.empty() ? "MyMap" : defaultPath.filename().string());

    AddText(ID_lblMapname, DrawPoint(labelX, off.y + 38), "Mapname", COLOR_YELLOW, FontStyle::CENTER, SmallFont);
    auto* editName = AddEdit(ID_edtMapname, off + DrawPoint(10, 50), Extent(210, 22), TextureColor::Grey, NormalFont);
    editName->SetText(mapObj ? mapObj->getMapname() : "");

    AddText(ID_lblAuthor, DrawPoint(labelX, off.y + 75), "Author", COLOR_YELLOW, FontStyle::CENTER, SmallFont);
    auto* editAuthor = AddEdit(ID_edtAuthor, off + DrawPoint(10, 87), Extent(210, 22), TextureColor::Grey, NormalFont);
    editAuthor->SetText(mapObj ? mapObj->getAuthor() : "");

    off += DrawPoint(0, 20);
    AddTextButton(ID_btSave, off + DrawPoint(10, 115), Extent(100, 22), TextureColor::Green2, "Save", NormalFont);
    AddTextButton(ID_btAbort, off + DrawPoint(120, 115), Extent(100, 22), TextureColor::Red1, "Abort", NormalFont);
}

void iwSaveMap::Msg_ButtonClick(unsigned ctrl_id)
{
    switch(ctrl_id)
    {
        case ID_btSave:
        {
            auto* mapObj = global::s2->getMapObj();
            if(!mapObj)
                break;
            auto* editFile = GetCtrl<ctrlEdit>(ID_edtFilename);
            auto* editName = GetCtrl<ctrlEdit>(ID_edtMapname);
            auto* editAuthor = GetCtrl<ctrlEdit>(ID_edtAuthor);
            if(!editFile || !editName || !editAuthor)
                break;

            mapObj->setMapname(editName->GetText());
            mapObj->setAuthor(editAuthor->GetText());

            bfs::path filepath = global::userMapsPath / editFile->GetText();
            if(!filepath.has_extension())
                filepath.replace_extension("SWD");
            mapObj->setFilepath(filepath);
            CFile::save_file(filepath, WLD, mapObj->getMap()); // ignore return for now

            Close();
            break;
        }
        case ID_btAbort:
            Close();
            break;
    }
}
