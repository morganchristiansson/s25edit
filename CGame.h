// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "SdlSurface.h"
#include "Texture.h"
#include "driver/VideoDriverLoaderInterface.h"
#include <boost/filesystem/path.hpp>
#include <Point.h>
#include <memory>
#include <vector>

class CMap;

class CGame : public VideoDriverLoaderInterface
{

public:
    Extent GameResolution;
    bool fullscreen;

    bool Running;
    bool showLoadScreen;
    SDL_GLContext glContext_ = nullptr;
    SdlWindow window_;

private:
    // milliseconds for SDL_Delay()
    Uint32 msWait = 0;
    Uint32 lastFrameTime = 0;
    Extent appliedResolution_ = Extent{0, 0}; ///< Last resolution we applied to the window/display
    bool appliedFullscreen_ = false;          ///< Last fullscreen state we applied

    // Textures for splash screen and cursor
    Texture splashBg_;
    Texture cursor_;
    Texture cursorClicked_;
    Texture cross_;

    // structure for mouse cursor
    struct
    {
        Position pos = {0, 0};
        bool clicked = false;
        struct
        {
            bool left = false;
            bool right = false;
        } button;
    } Cursor;

    // Object for Callbacks
    std::vector<void (*)(int)> Callbacks;
    // Object for the Map
    std::unique_ptr<CMap> MapObj;

    void SetAppIcon();
    void setGLViewport();
    bool CreateWindow();

    // VideoDriverLoaderInterface callbacks
    void Msg_LeftDown(MouseCoords mc) override;
    void Msg_LeftUp(MouseCoords mc) override;
    void Msg_RightDown(const MouseCoords& mc) override;
    void Msg_RightUp(const MouseCoords& mc) override;
    void Msg_MiddleDown(const MouseCoords& mc) override;
    void Msg_MiddleUp(const MouseCoords& mc) override;
    void Msg_WheelUp(const MouseCoords& mc) override;
    void Msg_WheelDown(const MouseCoords& mc) override;
    void Msg_MouseMove(const MouseCoords& mc) override;
    void Msg_KeyDown(const KeyEvent& ke) override;
    void WindowResized() override;

public:
    // Apply current GameResolution and fullscreen settings to the window/display.
    void ApplyWindowChanges();

    void LoadSettings();
    void SaveSettings() const;

    CGame(Extent GameResolution_, bool fullscreen_);
    ~CGame();

    int Execute();

    bool Init();
    void UpdateDisplaySize(const Extent& newSize);

    void ForwardEventToWindowManager(SDL_Event* Event);

    void GameLoop();

    void Render();

    void RenderPresent();

    void RegisterCallback(void (*callback)(int));
    bool UnregisterCallback(void (*callback)(int));
    void setMapObj(std::unique_ptr<CMap> MapObj);
    CMap* getMapObj();
    void delMapObj();
    void enterEditor(const boost::filesystem::path& filepath);
};
