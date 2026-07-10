// Copyright (C) 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "EditorWorldLoader.h"
#include "EventManager.h"
#include "GlobalGameSettings.h"
#include "PlayerInfo.h"
#include "RttrForeachPt.h"
#include "globals.h"
#include "lua/GameDataLoader.h"
#include "notifications/NodeNote.h"
#include "world/GameWorld.h"
#include "world/GameWorldViewer.h"
#include "gameData/MapConsts.h"

class EditorWorld
{
    EventManager em_;
    GlobalGameSettings ggs_;
    GameWorld world_;
    std::unique_ptr<GameWorldViewer> viewer_;

public:
    EditorWorld(const MapExtent& size, unsigned numPlayers)
        : em_(0), world_(std::vector<PlayerInfo>(numPlayers), ggs_, em_)
    {
        world_.GetDescriptionWriteable() = global::worldDesc;
        loadTexturesForEditorWorld(global::gameDataFilePath.string());
        world_.Init(size);
        RTTR_FOREACH_PT(MapPoint, size)
            world_.GetNodeWriteable(pt).fow[0].visibility = Visibility::Visible;
        world_.InitAfterLoad();
        viewer_ = std::make_unique<GameWorldViewer>(0, world_);
        viewer_->InitTerrainRenderer();
    }

    GameWorld& getWorld() { return world_; }
    const GameWorld& getWorld() const { return world_; }
    GameWorldViewer& getViewer() { return *viewer_; }
    const GameWorldViewer& getViewer() const { return *viewer_; }

    void notifyChanged(MapPoint pt)
    {
        world_.GetNotifications().publish(NodeNote(NodeNote::Altitude, pt));
    }

    void draw(const Position& firstPt, const Position& lastPt) const
    {
        viewer_->GetTerrainRenderer().Draw(firstPt, lastPt, *viewer_, nullptr);
    }
};
