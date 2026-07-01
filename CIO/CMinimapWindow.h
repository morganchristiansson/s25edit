// Copyright (C) 2026 - 2026 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "CWindow.h"

class CMinimapWindow final : public CWindow
{
    bool render() final;

public:
    using CWindow::CWindow;
};
