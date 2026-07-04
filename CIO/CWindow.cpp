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
      pos_(pos), size_(size), title(title), callback_(callback), callbackQuitMessage(callbackQuitMessage)
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
    const Position titleFrameLT = pos_ + Position(global::bmpArray[WINDOW_LEFT_UPPER_CORNER].w + 2, 4);
    const Position titleFrameRB =
      Position(pos_.x + static_cast<int>(size_.x) - global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w - 2,
               pos_.y + global::bmpArray[WINDOW_UPPER_FRAME].h - 4);
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
        pos_.x += motion.xrel;
        pos_.y += motion.yrel;
        // make sure to not move the window outside the display surface
        if(pos_.x < 0)
            pos_.x = 0;
        {
            const auto res = global::s2->getRes();
            const int resX = static_cast<int>(res.x);
            const int resY = static_cast<int>(res.y);
            if(pos_.x + static_cast<int>(size_.x) >= resX) //-V807
                pos_.x = resX - static_cast<int>(size_.x) - 1;
            if(pos_.y + static_cast<int>(size_.y) >= resY)
                pos_.y = resY - static_cast<int>(size_.y) - 1;
        }
    }

    // check whats happen to the close button
    if(canClose)
    {
        // cursor is on the button (+/-2 is only for the optic)
        canClose_marked = (motion.x >= pos_.x + 2) && (motion.x < pos_.x + global::bmpArray[WINDOW_BUTTON_CLOSE].w - 2)
                          && (motion.y >= pos_.y + 2)
                          && (motion.y < pos_.y + global::bmpArray[WINDOW_BUTTON_CLOSE].h - 2);
    }
    // check whats happen to the minimize button
    if(canMinimize)
    {
        // cursor is on the button (+/-2 is only for the optic)
        canMinimize_marked =
          (motion.x >= pos_.x + static_cast<int>(size_.x) - global::bmpArray[WINDOW_BUTTON_MINIMIZE].w + 2)
          && (motion.x < pos_.x + static_cast<int>(size_.x) - 2) && (motion.y >= pos_.y + 2)
          && (motion.y < pos_.y + global::bmpArray[WINDOW_BUTTON_MINIMIZE].h - 2);
    }
    // check whats happen to the resize button
    if(canResize)
    {
        // cursor is on the button (+/-2 is only for the optic)
        if((motion.x >= pos_.x + static_cast<int>(size_.x) - global::bmpArray[WINDOW_BUTTON_RESIZE].w + 2)
           && (motion.x < pos_.x + static_cast<int>(size_.x) - 2)
           && (motion.y >= pos_.y + static_cast<int>(size_.y) - global::bmpArray[WINDOW_BUTTON_RESIZE].h + 2)
           && (motion.y < pos_.y + static_cast<int>(size_.y) - 2))
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
                size_ = Extent(static_cast<int>(size_.x) + motion.xrel, static_cast<int>(size_.y) + motion.yrel);

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
        motion.x -= pos_.x;
        motion.y -= pos_.y;
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
        maximized_h = static_cast<int>(size_.y);

    // left button is pressed
    if(button.button == SDL_BUTTON_LEFT)
    {
        // cursor is on the title frame (+/-2 and +/-4 are only for a good optic)
        if((button.x >= pos_.x + global::bmpArray[WINDOW_LEFT_UPPER_CORNER].w + 2)
           && (button.x < pos_.x + static_cast<int>(size_.x) - global::bmpArray[WINDOW_RIGHT_UPPER_CORNER].w - 2)
           && (button.y >= pos_.y + 4)
           && (button.y < pos_.y + static_cast<int>(global::bmpArray[WINDOW_UPPER_FRAME].h) - 4))
        {
            marked = true;
            clicked = true;
        }
        // pressed inside the window
        if(button.state == SDL_PRESSED && (button.x >= pos_.x) && (button.x <= pos_.x + static_cast<int>(size_.x))
           && (button.y >= pos_.y) && (button.y <= pos_.y + static_cast<int>(size_.y)))
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
                    size_.y = static_cast<unsigned>(maximized_h);
                    minimized = false;
                } else // minimize now
                {
                    size_.y = global::bmpArray[WINDOW_UPPER_FRAME].h + global::bmpArray[WINDOW_CORNER_RECTANGLE].h;
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
        button.x -= pos_.x;
        button.y -= pos_.y;
        CControlContainer::setMouseData(button);
    }

    // at least call the callback
    callback_(WINDOW_CLICKED_CALL);
}

// ---------------------------------------------------------------------------
//  Draw — OpenGL version of the old render()
// ---------------------------------------------------------------------------

void CWindow::draw(Position /*parentOrigin*/)
{
    const Position origin = pos_;
    const Rect winRect(origin, size_);

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
        const auto contentW = static_cast<int>(size_.x) - b.left - b.right;
        const auto contentH = static_cast<int>(size_.y) - b.top - b.bottom;
        if(contentW > 0 && contentH > 0)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(contentX, viewH - (contentY + contentH), contentW, contentH);
            drawChildren(origin);
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
        const Rect upperFrameRect(origin, Extent(size_.x, getBmpTexture(upperframe).getHeight()));
        drawTiledBmp(upperframe, upperFrameRect);
    }

    // 4. Title text
    if(title)
    {
        const int titleY = origin.y + (getBmpTexture(WINDOW_UPPER_FRAME).getHeight() - 9) / 2;
        CFont::draw(title, Position(origin.x + static_cast<int>(size_.x) / 2, titleY), FontSize::Small,
                    FontColor::Yellow, FontAlign::Middle);
    }

    // 5. Lower frame (tiled across bottom)
    {
        const int lowerH = getBmpTexture(WINDOW_LOWER_FRAME).getHeight();
        const Rect lowerFrameRect(Position(origin.x, origin.y + static_cast<int>(size_.y) - lowerH),
                                  Extent(size_.x, lowerH));
        drawTiledBmp(WINDOW_LOWER_FRAME, lowerFrameRect);
    }

    // 6. Left frame (tiled down left side)
    {
        const Rect leftFrameRect(origin, Extent(getBmpTexture(WINDOW_LEFT_FRAME).getWidth(), size_.y));
        drawTiledBmp(WINDOW_LEFT_FRAME, leftFrameRect);
    }

    // 7. Right frame (tiled down right side)
    {
        const int rightW = getBmpTexture(WINDOW_RIGHT_FRAME).getWidth();
        const Rect rightFrameRect(Position(origin.x + static_cast<int>(size_.x) - rightW, origin.y),
                                  Extent(rightW, size_.y));
        drawTiledBmp(WINDOW_RIGHT_FRAME, rightFrameRect);
    }

    // 8. Corners
    {
        getBmpTexture(WINDOW_LEFT_UPPER_CORNER).draw(origin);

        const int ruW = getBmpTexture(WINDOW_RIGHT_UPPER_CORNER).getWidth();
        getBmpTexture(WINDOW_RIGHT_UPPER_CORNER).draw(Position(origin.x + static_cast<int>(size_.x) - ruW, origin.y));

        const int crW = getBmpTexture(WINDOW_CORNER_RECTANGLE).getWidth();
        const int crH = getBmpTexture(WINDOW_CORNER_RECTANGLE).getHeight();
        getBmpTexture(WINDOW_CORNER_RECTANGLE).draw(Position(origin.x, origin.y + static_cast<int>(size_.y) - crH));
        getBmpTexture(WINDOW_CORNER_RECTANGLE)
          .draw(Position(origin.x + static_cast<int>(size_.x) - crW, origin.y + static_cast<int>(size_.y) - crH));
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
        getBmpTexture(closebutton).draw(origin);
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
          .draw(Position(origin.x + static_cast<int>(size_.x) - getBmpTexture(minimizebutton).getWidth(), origin.y));
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
          .draw(Position(origin + Position(static_cast<int>(size_.x), static_cast<int>(size_.y)))
                - Position(getBmpTexture(resizebutton).getSize()));
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
