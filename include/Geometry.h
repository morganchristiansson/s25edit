// Copyright (C) 2009 - 2025 Settlers Freaks <sf-team at siedler25.org>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "defines.h"

/// ---------------------------------------------------------------------------
/// Two USD triangle coordinate conventions.
///
/// s25client (new, "client"):
///   visual USD(x, y) ≡ vertex (x, y).usdTexture
///
/// s25edit (old, "editor"):
///   visual USD(x, y) ≡ vertex (x - !(y & 1), y).usdTexture
///
/// RSU triangles are identical in both.
///
/// During the transition:
///   - Updated call sites use the client* coordinate wrappers or access
///     .usdTexture directly (identity convention).
///   - Non-updated call sites use the editor* wrappers.
///   - When all call sites are migrated, the editor* wrappers can be deleted.
/// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Per-convention USD source-vertex mapping
// ---------------------------------------------------------------------------

/// s25client convention: USD(x,y) reads vertex (x, y).usdTexture
inline int clientUsdX(int triX, int /*triY*/)
{
    return triX;
}

/// Source vertex position for a visual USD position (s25client convention).
inline Position clientUsdVertexPos(Position visualPos)
{
    return {clientUsdX(visualPos.x, visualPos.y), visualPos.y};
}

/// Source vertex position for a visual USD position given as (x,y).
inline Position clientUsdVertexPos(int x, int y)
{
    return {clientUsdX(x, y), y};
}

/// Editor convention: USD(x,y) reads vertex (x - !(y&1), y).usdTexture
inline int editorUsdX(int triX, int triY)
{
    return triX - !(triY & 1);
}

// ---------------------------------------------------------------------------
// Per-convention USD triangle vertex sets
// ---------------------------------------------------------------------------

/// Three vertices of a USD triangle in DrawTriangle parameter order:
/// P1 = bottom-left, P2 = top (reads usdTexture), P3 = bottom-right.
struct UsdVertices
{
    int p1x, p1y;
    int p2x, p2y;
    int p3x, p3y;
};

/// USD triangle vertices for s25client convention (identity mapping).
inline UsdVertices clientUsdTriangleVertices(int triX, int triY)
{
    // sx = triX
    return {triX + (triY & 1),
            triY + 1, // P1 = bottom-left
            triX,
            triY, // P2 = top
            triX + 1,
            triY}; // P3 = bottom-right
}

/// USD triangle vertices for editor convention (backward compat).
inline UsdVertices editorUsdTriangleVertices(int triX, int triY)
{
    int sx = editorUsdX(triX, triY); // sx = triX - !(triY&1)
    return {sx + (triY & 1),
            triY + 1, // P1 = bottom-left
            sx,
            triY, // P2 = top
            sx + 1,
            triY}; // P3 = bottom-right
}
