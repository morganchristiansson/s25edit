// Copyright (C) 2009 - 2025 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"

/// ---------------------------------------------------------------------------
/// Legacy USD coordinate conversion.
///
/// In-memory storage uses s25client convention:
///   vertex(x,y).usdTexture ≡ visual USD(x, y)
///
/// The old editor convention was:
///   visual USD(x, y) ≡ vertex(x - !(y & 1), y).usdTexture
///
/// These helpers convert from old-editor visual labels to the current
/// s25client vertex indices.  Only needed for code that hasn't been
/// migrated to the new label semantics.
/// ---------------------------------------------------------------------------

/// Old-editor visual label → s25client vertex index.
inline int editorUsdX(int triX, int triY)
{
    return triX - !(triY & 1);
}

/// Three vertices of a USD triangle in DrawTriangle parameter order:
/// P1 = bottom-left, P2 = top (reads usdTexture), P3 = bottom-right.
struct UsdVertices
{
    int p1x, p1y;
    int p2x, p2y;
    int p3x, p3y;
};

/// USD triangle vertices for old-editor convention (backward compat).
inline UsdVertices editorUsdTriangleVertices(int triX, int triY)
{
    int sx = editorUsdX(triX, triY);
    return {sx + (triY & 1), triY + 1, sx, triY, sx + 1, triY};
}
