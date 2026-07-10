// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CGame.h"
#include "CIO/CWindow.h"
#include "CMap.h"
#include "CSurface.h"
#include "CollisionDetection.h"
#include "WindowManager.h"
#include "callbacks.h"
#include "drivers/VideoDriverWrapper.h"
#include "enum_cast.hpp"
#include "globals.h"
#include "s25util/utf8.h"

void CGame::EventHandling(SDL_Event* Event)
{
    switch(Event->type)
    {
        case SDL_KEYDOWN:
        {
            // NOTE: we will now deliver the data to menus, windows, map etc., sometimes we have to break the switch and
            // stop
            //      delivering earlier, for doing this we make use of a variable showing us the deliver status
            bool delivered = false;
            // now we walk through the windows and find out, if cursor is on one of these (ordered by priority)
            // we have to change the prioritys of the windows (for rendering), so find the highest one
            int highestPriority = 0;
            for(auto& Window : Windows)
            {
                if(Window->getPriority() > highestPriority)
                    highestPriority = Window->getPriority();
            }

            for(auto& Window : Windows)
            {
                if(!Window->isWaste() && Window->isMarked() && Window->getPriority() == highestPriority
                   && Window->hasActiveInputElement())
                {
                    Window->setKeyboardData(Event->key);
                    delivered = true;
                    break;
                }
            }
            // if (delivered)
            //    break;

            // deliver keyboard data to map if active
            if(!delivered)
            {
                if(MapObj && MapObj->isActive())
                {
                    MapObj->setKeyboardData(Event->key);
                    // data has been delivered to map, so no menu is in the foreground --> stop delivering
                    // break;
                }


            }

            switch(Event->key.keysym.sym)
            {
                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    if(Event->key.keysym.mod & KMOD_ALT)
                    {
                        fullscreen = !fullscreen;
                        ApplyWindowChanges();
                        SaveSettings();
                    }
                    break;

#ifdef _ADMINMODE
                case SDLK_F3: // if CTRL and ALT are pressed
                    // if (SDL_GetModState() == (KMOD_LCTRL | KMOD_LALT))
                    callback::debugger(INITIALIZING_CALL);
                    break;

#endif
                // Zoom keys removed — TerrainRenderer handles zoom via glScale

                default: break;
            }

            break;
        }

        case SDL_KEYUP:
        {
            // deliver keyboard data to map
            if(MapObj)
                MapObj->setKeyboardData(Event->key);

            break;
        }

        case SDL_MOUSEMOTION:
        {
            // setup mouse cursor data
            if(MapObj && MapObj->isActive())
            {
                if((Event->motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) == 0)
                {
                    Cursor.pos = Position(Event->motion.x, Event->motion.y);
                }
            } else
            {
                Cursor.pos = Position(Event->motion.x, Event->motion.y);
            }
            /*
                        //NOTE: we will now deliver the data to menus, windows, map etc., sometimes we have to break the
               switch and stop
                        //      delivering earlier, for doing this we make use of a variable showing us the deliver
               status int delivered = false;
                        //deliver mouse motion data to the active window
                        for (int i = 0; i < MAXWINDOWS; i++)
                        {
                            if (Windows[i] != nullptr && Windows[i]->isActive() && !Windows[i]->isWaste())
                            {
                                Windows[i]->setMouseData(Event->motion);
                                if ( (Event->motion.x >= Windows[i]->getX()) && (Event->motion.x < Windows[i]->getX() +
               Windows[i]->getW())
               && (Event->motion.y >= Windows[i]->getY()) && (Event->motion.y < Windows[i]->getY() + Windows[i]->getH())
               ) delivered = true;
                            }
                        }
                        if (delivered)
                            break;
            */

            // NOTE: we will now deliver the data to menus, windows, map etc., sometimes we have to break the switch and
            // stop
            //      delivering earlier, for doing this we make use of a variable showing us the deliver status
            bool delivered = false;
            // now we walk through the windows and find out, if cursor is on one of these (ordered by priority)
            // we have to change the prioritys of the windows (for rendering), so find the highest one
            int highestPriority = 0;
            for(auto& Window : Windows)
            {
                if(Window->getPriority() > highestPriority)
                    highestPriority = Window->getPriority();
            }

            for(int actualPriority = highestPriority; actualPriority >= 0; actualPriority--)
            {
                for(auto& Window : Windows)
                {
                    if(!Window->isWaste() && Window->getPriority() == actualPriority)
                    {
                        // is the cursor INSIDE the window or does the user move or resize the window?
                        if(IsPointInRect(Event->motion.x, Event->motion.y, Rect(Window->getPos(), Window->getSize()))
                           || Window->isMoving() || Window->isResizing())
                        {
                            // Windows[i]->setActive();
                            // Windows[i]->setPriority(highestPriority+1);
                            Window->setMouseData(Event->motion);
                            delivered = true;
                            break;
                        }
                    }
                }
                if(delivered)
                    break;
            }
            // if mouse data has been delivered, stop delivering anymore
            if(delivered)
                break;

            // deliver mouse motion data to map if active
            if(MapObj && MapObj->isActive())
            {
                MapObj->setMouseData(Event->motion);
                // data has been delivered to map, so no menu is in the foreground --> stop delivering
                break;
            }



            break;
        }

        case SDL_MOUSEBUTTONDOWN:
        {
            // setup mouse cursor data
            Cursor.clicked = true;
            Cursor.button.left = false;
            Cursor.button.right = false;
            if(Event->button.button == SDL_BUTTON_LEFT)
                Cursor.button.left = true;
            else if(Event->button.button == SDL_BUTTON_RIGHT)
                Cursor.button.right = true;

            // clicking a mouse button will close the S2 loading screen if it is shown
            if(showLoadScreen)
            {
                showLoadScreen = false;
                // prevent pressing another object "behind" the loading screen
                break;
            }

            // NOTE: we will now deliver the data to menus, windows, map etc., sometimes we have to break the switch and
            // stop
            //      delivering earlier, for doing this we make use of a variable showing us the deliver status
            bool delivered = false;
            // now we walk through the windows and find out, if cursor is on one of these (ordered by priority)
            // we have to change the prioritys of the windows (for rendering), so find the highest one
            int highestPriority = 0;
            for(auto& Window : Windows)
            {
                if(Window->getPriority() > highestPriority)
                    highestPriority = Window->getPriority();
            }

            for(int actualPriority = highestPriority; actualPriority >= 0; actualPriority--)
            {
                for(auto& Window : Windows)
                {
                    if(!Window->isWaste() && Window->getPriority() == actualPriority)
                    {
                        // is the cursor INSIDE the window?
                        if(IsPointInRect(Event->button.x, Event->button.y, Rect(Window->getPos(), Window->getSize())))
                        {
                            Window->setActive();
                            Window->setPriority(highestPriority + 1);
                            Window->setMouseData(Event->button);
                            delivered = true;
                            break;
                        } else if(Window->isActive())
                            Window->setInactive();
                    }
                }
                if(delivered)
                    break;
            }
            // if mouse data has been deliverd, stop delivering anymore
            if(delivered)
                break;

            // deliver mouse button data to map if active
            if(MapObj && MapObj->isActive())
            {
                MapObj->setMouseData(Event->button);
                // data has been delivered to map, so no menu is in the foreground --> stop delivering
                break;
            }

            break;
        }

        case SDL_MOUSEBUTTONUP:
        {
            // setup mouse cursor data
            Cursor.clicked = false;

            // NOTE: we will now deliver the data to menus, windows, map etc., sometimes we have to break the switch and
            // stop
            //      delivering earlier, for doing this we make use of a variable showing us the deliver status
            bool delivered = false;
            // now we walk through the windows and find out, if cursor is on one of these (ordered by priority)
            // we have to change the prioritys of the windows (for rendering), so find the highest one
            int highestPriority = 0;
            for(auto& Window : Windows)
            {
                if(Window->getPriority() > highestPriority)
                    highestPriority = Window->getPriority();
            }

            for(int actualPriority = highestPriority; actualPriority >= 0; actualPriority--)
            {
                for(auto& Window : Windows)
                {
                    if(!Window->isWaste() && Window->getPriority() == actualPriority)
                    {
                        // is the cursor INSIDE the window?
                        if(IsPointInRect(Event->button.x, Event->button.y, Rect(Window->getPos(), Window->getSize())))
                        {
                            // Windows[i]->setActive();
                            // Windows[i]->setPriority(highestPriority+1);
                            Window->setMouseData(Event->button);
                            delivered = true;
                            break;
                        }
                        // else if (Windows[i]->isActive())
                        // Windows[i]->setInactive();
                    }
                }
                if(delivered)
                    break;
            }
            // if mouse data has been deliverd, stop delivering anymore
            /// We can't stop here cause of problems with the map. If user has the left mouse button pressed and
            /// modifies the vertices, it will cause a problem if he walks over a window with pressed mouse button and
            /// releases it in the window. So the MapObj needs the "release-event" of the mouse button.
            // if (delivered)
            // break;

            // if still not delivered, keep delivering to secondary elements like menu or map

            // deliver mouse button data to map if active
            if(MapObj && MapObj->isActive())
            {
                MapObj->setMouseData(Event->button);
                // data has been delivered to map, so no menu is in the foreground --> stop delivering
                break;
            }

            /// now we do what we commented out a few lines before
            if(delivered)
                break;

            break;
        }

        case SDL_WINDOWEVENT:
        {
            if(Event->window.event == SDL_WINDOWEVENT_RESIZED)
            {
                // In fullscreen the compositor (e.g. Wayland) may report a size
                // different from the one we requested. We already applied the
                // resolution ourselves, so don't let the event override it.
                if(fullscreen)
                    break;
                const Extent newSize(Event->window.data1, Event->window.data2);
                // Ignore events matching the resolution we already applied.
                if(newSize == appliedResolution_)
                    break;
                UpdateDisplaySize(newSize);
            }
            break;
        }

        case SDL_QUIT: Running = false; break;

        default: break;
    }
}

void CGame::ForwardEventToWindowManager(SDL_Event* ev)
{
    static MouseCoords mouse_xy;

    switch(ev->type)
    {
        case SDL_WINDOWEVENT:
            if(ev->window.event == SDL_WINDOWEVENT_RESIZED)
                WINDOWMANAGER.WindowResized();
            break;

        case SDL_KEYDOWN:
        {
            KeyEvent ke;
            switch(ev->key.keysym.sym)
            {
                case SDLK_RETURN: ke.kt = KeyType::Return; break;
                case SDLK_SPACE: ke.kt = KeyType::Space; break;
                case SDLK_LEFT: ke.kt = KeyType::Left; break;
                case SDLK_RIGHT: ke.kt = KeyType::Right; break;
                case SDLK_UP: ke.kt = KeyType::Up; break;
                case SDLK_DOWN: ke.kt = KeyType::Down; break;
                case SDLK_BACKSPACE: ke.kt = KeyType::Backspace; break;
                case SDLK_DELETE: ke.kt = KeyType::Delete; break;
                case SDLK_TAB: ke.kt = KeyType::Tab; break;
                case SDLK_HOME: ke.kt = KeyType::Home; break;
                case SDLK_END: ke.kt = KeyType::End; break;
                case SDLK_ESCAPE: ke.kt = KeyType::Escape; break;
                case SDLK_PRINTSCREEN: ke.kt = KeyType::Print; break;
                default:
                    if(ev->key.keysym.sym >= SDLK_F1 && ev->key.keysym.sym <= SDLK_F12)
                        ke.kt = static_cast<KeyType>(rttr::enum_cast(KeyType::F1) + ev->key.keysym.sym - SDLK_F1);
                    break;
            }
            ke.alt = (ev->key.keysym.mod & KMOD_ALT) != 0;
            ke.ctrl = (ev->key.keysym.mod & KMOD_CTRL) != 0;
            ke.shift = (ev->key.keysym.mod & KMOD_SHIFT) != 0;
            if(ke.kt != KeyType::Invalid)
                WINDOWMANAGER.Msg_KeyDown(ke);
            break;
        }

        case SDL_TEXTINPUT:
        {
            const std::u32string text = s25util::utf8to32(ev->text.text);
            KeyEvent ke;
            for(char32_t c : text)
            {
                ke.c = c;
                WINDOWMANAGER.Msg_KeyDown(ke);
            }
            break;
        }

        case SDL_MOUSEBUTTONDOWN:
            mouse_xy.pos = Position(ev->button.x, ev->button.y);
            VIDEODRIVER.SetMousePos(mouse_xy.pos);
            switch(ev->button.button)
            {
                case SDL_BUTTON_LEFT:
                    mouse_xy.ldown = true;
                    WINDOWMANAGER.Msg_LeftDown(mouse_xy);
                    break;
                case SDL_BUTTON_RIGHT:
                    mouse_xy.rdown = true;
                    WINDOWMANAGER.Msg_RightDown(mouse_xy);
                    break;
                case SDL_BUTTON_MIDDLE:
                    mouse_xy.mdown = true;
                    WINDOWMANAGER.Msg_MiddleDown(mouse_xy);
                    break;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            mouse_xy.pos = Position(ev->button.x, ev->button.y);
            VIDEODRIVER.SetMousePos(mouse_xy.pos);
            switch(ev->button.button)
            {
                case SDL_BUTTON_LEFT:
                    mouse_xy.ldown = false;
                    WINDOWMANAGER.Msg_LeftUp(mouse_xy);
                    break;
                case SDL_BUTTON_RIGHT:
                    mouse_xy.rdown = false;
                    WINDOWMANAGER.Msg_RightUp(mouse_xy);
                    break;
                case SDL_BUTTON_MIDDLE:
                    mouse_xy.mdown = false;
                    WINDOWMANAGER.Msg_MiddleUp(mouse_xy);
                    break;
            }
            break;

        case SDL_MOUSEWHEEL:
        {
            int y = ev->wheel.y;
            if(ev->wheel.direction == SDL_MOUSEWHEEL_FLIPPED)
                y = -y;
            if(y > 0)
                WINDOWMANAGER.Msg_WheelUp(mouse_xy);
            else if(y < 0)
                WINDOWMANAGER.Msg_WheelDown(mouse_xy);
            break;
        }

        case SDL_MOUSEMOTION:
        {
            const Position newPos(ev->motion.x, ev->motion.y);
            if(newPos != mouse_xy.pos)
            {
                mouse_xy.pos = newPos;
                VIDEODRIVER.SetMousePos(newPos);
                WINDOWMANAGER.Msg_MouseMove(mouse_xy);
            }
            break;
        }
    }
}
