// Copyright (C) 2009 - 2021 Marc Vester (XaserLE)
// Copyright (C) 2009 - 2021 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CWindow.h"
#include "../CGame.h"
#include "../Texture.h"
#include "../globals.h"
#include "CButton.h"
#include "CFont.h"
#include "CPicture.h"
#include "CSelectBox.h"
#include "CTextfield.h"
#include "CollisionDetection.h"
#include "helpers/containerUtils.h"
#include <glad/glad.h>
#include <cassert>

CWindow::CWindow(void callback(int), int callbackQuitMessage, Position pos, Extent size, const char* title, int color,
                 Uint8 flags)
    : CControlContainer(color, {global::bmpArray[WINDOW_LEFT_FRAME].w, global::bmpArray[WINDOW_UPPER_FRAME].h,
                                global::bmpArray[WINDOW_RIGHT_FRAME].w, global::bmpArray[WINDOW_LOWER_FRAME].h}),
      x_(pos.x), y_(pos.y), w_(size.x), h_(size.y), title(title), callback_(callback),
      callbackQuitMessage(callbackQuitMessage)
{
    assert(callback);
    canMove = (flags & WINDOW_MOVE) != 0;
    canClose = (flags & WINDOW_CLOSE) != 0;
    canMinimize = (flags & WINDOW_MINIMIZE) != 0;
    canResize = (flags & WINDOW_RESIZE) != 0;
}

static Position makePos(WindowPos pos, Extent size)
{
    if(pos == WindowPos::Center)
    {
        const auto res = global::s2->getRes();
        return Position(res.x / 2, res.y / 2) - size / 2;
    } else
        return {};
}

CWindow::CWindow(void callback(int), int callbackQuitMessage, WindowPos pos, Extent size,
                 const char* title /*= nullptr*/, int color /*= WINDOW_GREEN1*/, Uint8 flags /*= 0*/)
    : CWindow(callback, callbackQuitMessage, makePos(pos, size), size, title, color, flags)
{}

void CWindow::setTitle(const char* title)
{
    this->title = title;
}

void CWindow::setColor(int color)
{
    setBackgroundPicture(color);
}

bool CWindow::hasActiveInputElement()
{
    return helpers::contains_if(getTextFields(), [](const auto& text) { return text->isActive(); });
}

void CWindow::setMouseData(SDL_MouseMotionEvent motion)
{
    // cursor is on the title frame (+/-2 and +/-4 are only for a good optic)
    const Position titleFrameLT = Position(x_, y_) + Position(global::bmpArray[WINDOW_LEFT_UPPER_CORNER].w + 2, 4);
    const Position titleFrameRB = Position(x_ + w_ - global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w - 2,
                                           y_ + global::bmpArray[WINDOW_UPPER_FRAME].h - 4);
    if(IsPointInRect(motion.x, motion.y, Rect(titleFrameLT, Extent(titleFrameRB - titleFrameLT))))
    {
        // left button was pressed while moving
        if(clicked)
            moving = true;
    } else if(!moving)
        clicked = false;

    if(!(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)))
        moving = false;
    if(moving && canMove)
    {
        x_ += motion.xrel;
        y_ += motion.yrel;
        // make sure to not move the window outside the display surface
        if(x_ < 0)
            x_ = 0;
        {
            const auto res = global::s2->getRes();
            if(x_ + w_ >= static_cast<int>(res.x)) //-V807
                x_ = static_cast<int>(res.x) - w_ - 1;
            if(y_ + h_ >= static_cast<int>(res.y))
                y_ = static_cast<int>(res.y) - h_ - 1;
        }
    }

    // check whats happen to the close button
    if(canClose)
    {
        // cursor is on the button (+/-2 is only for the optic)
        canClose_marked = (motion.x >= x_ + 2) && (motion.x < x_ + global::bmpArray[WINDOW_BUTTON_CLOSE].w - 2)
                          && (motion.y >= y_ + 2) && (motion.y < y_ + global::bmpArray[WINDOW_BUTTON_CLOSE].h - 2);
    }
    // check whats happen to the minimize button
    if(canMinimize)
    {
        // cursor is on the button (+/-2 is only for the optic)
        canMinimize_marked = (motion.x >= x_ + w_ - global::bmpArray[WINDOW_BUTTON_MINIMIZE].w + 2)
                             && (motion.x < x_ + w_ - 2) && (motion.y >= y_ + 2)
                             && (motion.y < y_ + global::bmpArray[WINDOW_BUTTON_MINIMIZE].h - 2);
    }
    // check whats happen to the resize button
    if(canResize)
    {
        // cursor is on the button (+/-2 is only for the optic)
        if((motion.x >= x_ + w_ - global::bmpArray[WINDOW_BUTTON_RESIZE].w + 2) && (motion.x < x_ + w_ - 2)
           && (motion.y >= y_ + h_ - global::bmpArray[WINDOW_BUTTON_RESIZE].h + 2) && (motion.y < y_ + h_ - 2))
        {
            // left button was pressed while moving
            if(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT))
                resizing = true;
            canResize_marked = true;
        } else if(!resizing)
            canResize_marked = false;

        if(!(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)))
            resizing = false;
        if(resizing)
        {
            // only resize if not minimized
            if(!minimized)
            {
                w_ += motion.xrel;
                h_ += motion.yrel;

                // MISSING: we have to test if window size is under minimum

                // notify the callback that the window has been resized
                callback_(WINDOW_RESIZED_CALL);
            }
        }
    }

    // deliver mouse data to the content objects of the window (if mouse cursor is inside the window)
    if(IsPointInRect(motion.x, motion.y, Rect(getPos(), getSize())))
    {
        // IMPORTANT: we use the left upper corner of the window as (x,y)=(0,0), so we have to manipulate
        //           the motion-structure before give it to buttons, pictures....: x_absolute - x_window, y_absolute -
        //           y_window
        motion.x -= x_;
        motion.y -= y_;
        CControlContainer::setMouseData(motion);
    }
}

void CWindow::setMouseData(SDL_MouseButtonEvent button)
{
    // at first check if the right mouse button was pressed, cause in this case we will close the window
    if(button.button == SDL_BUTTON_RIGHT && button.state == SDL_PRESSED)
    {
        callback_(callbackQuitMessage);
        return;
    }

    // save width and height in case we minimize the window (the initializing values are for preventing any mistakes and
    // compilerwarning --- in fact: uninitialized values are only a problem if the window is created minimized, but this
    // will not happen)
    static int maximized_h = global::bmpArray[WINDOW_UPPER_FRAME].h + global::bmpArray[WINDOW_CORNER_RECTANGLE].h;
    if(!minimized)
        maximized_h = h_;

    // left button is pressed
    if(button.button == SDL_BUTTON_LEFT)
    {
        // cursor is on the title frame (+/-2 and +/-4 are only for a good optic)
        if((button.x >= x_ + global::bmpArray[WINDOW_LEFT_UPPER_CORNER].w + 2)
           && (button.x < x_ + w_ - global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w - 2) && (button.y >= y_ + 4)
           && (button.y < y_ + +global::bmpArray[WINDOW_UPPER_FRAME].h - 4))
        {
            marked = true;
            clicked = true;
        }
        // pressed inside the window
        if(button.state == SDL_PRESSED && (button.x >= x_) && (button.x <= x_ + w_) && (button.y >= y_)
           && (button.y <= y_ + h_))
            marked = true;
        // else pressed outside of the window
        else if(button.state == SDL_PRESSED)
            marked = false;

        // check whats happen to the close button
        // only set 'clicked' if pressed AND cursor is ON the button (marked == true)
        if(button.state == SDL_PRESSED && canClose_marked)
            canClose_clicked = true;
        else if(button.state == SDL_RELEASED)
        {
            canClose_clicked = false;
            // if mouse button is released ON the close button (marked = true), then send the quit message to the
            // callback
            if(canClose_marked)
            {
                callback_(callbackQuitMessage);
                return;
            }
        }
        // check whats happen to the minimize button
        // only set 'clicked' if pressed AND cursor is ON the button (marked == true)
        if(button.state == SDL_PRESSED && canMinimize_marked)
            canMinimize_clicked = true;
        else if(button.state == SDL_RELEASED)
        {
            canMinimize_clicked = false;
            // if mouse button is released ON the BUTTON (marked = true), then minimize or maximize the window
            if(canMinimize_marked)
            {
                if(minimized) // maximize now
                {
                    h_ = maximized_h;
                    minimized = false;
                } else // minimize now
                {
                    h_ = global::bmpArray[WINDOW_UPPER_FRAME].h + global::bmpArray[WINDOW_CORNER_RECTANGLE].h;
                    minimized = true;
                }
            }
        }
        // check whats happen to the resize button
        // only set 'clicked' if pressed AND cursor is ON the button (marked == true)
        if(button.state == SDL_PRESSED && canResize_marked)
            canResize_clicked = true;
        else if(button.state == SDL_RELEASED)
            canResize_clicked = false;
    }

    // deliver mouse data to the content objects of the window (if mouse cursor is inside the window)
    if(IsPointInRect(button.x, button.y, Rect(getPos(), getSize())))
    {
        // IMPORTANT: we use the left upper corner of the window as (x,y)=(0,0), so we have to manipulate
        //           the motion-structure before give it to buttons, pictures....: x_absolute - x_window, y_absolute -
        //           y_window
        button.x -= x_;
        button.y -= y_;
        CControlContainer::setMouseData(button);
    }

    // at least call the callback
    callback_(WINDOW_CLICKED_CALL);
}

// ---------------------------------------------------------------------------
//  Draw — OpenGL version of the old render()
// ---------------------------------------------------------------------------

void CWindow::Draw(Position /*parentOrigin*/)
{
    const Position origin(x_, y_);
    const Rect winRect(origin, Extent(w_, h_));

    // 1. Background fill (tiled)
    if(getBackground() != WINDOW_NOTHING)
        drawTiledBmp(getBackground(), winRect);

    // 2. Content (if not minimized) — clipped to the area inside frames
    if(!minimized)
    {
        const auto viewH = global::s2->getRes().y;
        const auto& b = getBorder();
        const auto contentX = origin.x + b.left;
        const auto contentY = origin.y + b.top;
        const auto contentW = static_cast<int>(w_) - b.left - b.right;
        const auto contentH = static_cast<int>(h_) - b.top - b.bottom;
        if(contentW > 0 && contentH > 0)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(contentX, viewH - (contentY + contentH), contentW, contentH);
            DrawChildren(origin);
            glDisable(GL_SCISSOR_TEST);
        }
    }

    // 3. Upper frame
    int upperframe;
    if(clicked)
        upperframe = WINDOW_UPPER_FRAME_CLICKED;
    else if(marked)
        upperframe = WINDOW_UPPER_FRAME_MARKED;
    else
        upperframe = WINDOW_UPPER_FRAME;

    // Draw upper frame tile across the top of the window
    {
        const Rect upperFrameRect(origin, Extent(w_, getBmpTexture(upperframe).getHeight()));
        drawTiledBmp(upperframe, upperFrameRect);
    }

    // 4. Title text
    if(title)
    {
        const int titleY = origin.y + (getBmpTexture(WINDOW_UPPER_FRAME).getHeight() - 9) / 2;
        CFont::Draw(title, Position(origin.x + static_cast<int>(w_) / 2, titleY), FontSize::Small, FontColor::Yellow,
                    FontAlign::Middle);
    }

    // 5. Lower frame (tiled across bottom)
    {
        const int lowerH = getBmpTexture(WINDOW_LOWER_FRAME).getHeight();
        const Rect lowerFrameRect(Position(origin.x, origin.y + static_cast<int>(h_) - lowerH),
                                  Extent(w_, lowerH));
        drawTiledBmp(WINDOW_LOWER_FRAME, lowerFrameRect);
    }

    // 6. Left frame (tiled down left side)
    {
        const Rect leftFrameRect(origin, Extent(getBmpTexture(WINDOW_LEFT_FRAME).getWidth(), h_));
        drawTiledBmp(WINDOW_LEFT_FRAME, leftFrameRect);
    }

    // 7. Right frame (tiled down right side)
    {
        const int rightW = getBmpTexture(WINDOW_RIGHT_FRAME).getWidth();
        const Rect rightFrameRect(Position(origin.x + static_cast<int>(w_) - rightW, origin.y),
                                  Extent(rightW, h_));
        drawTiledBmp(WINDOW_RIGHT_FRAME, rightFrameRect);
    }

    // 8. Corners
    {
        getBmpTexture(WINDOW_LEFT_UPPER_CORNER).Draw(origin);

        const int ruW = getBmpTexture(WINDOW_RIGHT_UPPER_CORNER).getWidth();
        getBmpTexture(WINDOW_RIGHT_UPPER_CORNER)
          .Draw(Position(origin.x + static_cast<int>(w_) - ruW, origin.y));

        const int crW = getBmpTexture(WINDOW_CORNER_RECTANGLE).getWidth();
        const int crH = getBmpTexture(WINDOW_CORNER_RECTANGLE).getHeight();
        getBmpTexture(WINDOW_CORNER_RECTANGLE)
          .Draw(Position(origin.x, origin.y + static_cast<int>(h_) - crH));
        getBmpTexture(WINDOW_CORNER_RECTANGLE)
          .Draw(Position(origin.x + static_cast<int>(w_) - crW,
                         origin.y + static_cast<int>(h_) - crH));
    }

    // 9. Close button
    if(canClose)
    {
        int closebutton;
        if(canClose_clicked)
            closebutton = WINDOW_BUTTON_CLOSE_CLICKED;
        else if(canClose_marked)
            closebutton = WINDOW_BUTTON_CLOSE_MARKED;
        else
            closebutton = WINDOW_BUTTON_CLOSE;
        getBmpTexture(closebutton).Draw(origin);
    }

    // 10. Minimize button
    if(canMinimize)
    {
        int minimizebutton;
        if(canMinimize_clicked)
            minimizebutton = WINDOW_BUTTON_MINIMIZE_CLICKED;
        else if(canMinimize_marked)
            minimizebutton = WINDOW_BUTTON_MINIMIZE_MARKED;
        else
            minimizebutton = WINDOW_BUTTON_MINIMIZE;
        getBmpTexture(minimizebutton)
          .Draw(Position(origin.x + static_cast<int>(w_) - getBmpTexture(minimizebutton).getWidth(), origin.y));
    }

    // 11. Resize button
    if(canResize)
    {
        int resizebutton;
        if(canResize_clicked)
            resizebutton = WINDOW_BUTTON_RESIZE_CLICKED;
        else if(canResize_marked)
            resizebutton = WINDOW_BUTTON_RESIZE_MARKED;
        else
            resizebutton = WINDOW_BUTTON_RESIZE;
        getBmpTexture(resizebutton)
          .Draw(Position(origin + Position(w_, h_)) - Position(getBmpTexture(resizebutton).getWidth(), getBmpTexture(resizebutton).getHeight()));
    }
}

void CWindow::setInactive()
{
    active = false;
    clicked = false;
    marked = false;

    for(auto& textfield : getTextFields())
    {
        textfield->setInactive();
    }
}
