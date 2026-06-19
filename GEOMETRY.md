<!--
Copyright (C) 2009 - 2025 Settlers Freaks <sf-team at siedler25.org>

SPDX-License-Identifier: GPL-3.0-or-later
-->

# Map geometry alignment with s25client

The editor and s25client both use a staggered hexagonal grid. RSU triangles are
identical, but the **USD triangle label convention differs on even rows**.

## The mapping

Same physical triangle, different visual label:

```
s25client USD(x, y) ≡ editor USD(x + !(y & 1), y)
editor   USD(x, y) ≡ s25client USD(x - !(y & 1), y)
```

Odd rows (`y & 1 == 1`) are identical.

## In-memory convention: s25client

bobMAP now uses s25client convention throughout:
`vertex(x,y).usdTexture` ≡ texture for visual USD(x, y) (s25client naming).

| Component | Change |
|-----------|--------|
| **Rendering** (`CSurface.cpp`) | `DrawTriangleField` uses `clientUsdTriangleVertices(x, y)` — reads `vertex(x,y).usdTexture` for visual USD(x,y) |
| **File load** (`CIO/CFile.cpp`) | Read each file byte at (i,j); for even rows store at `memory(i+1, j)`, for odd at `memory(i, j)`. Formula: `memory(i, j) = file(i - !(j&1), j)` |
| **File save** (`CIO/CFile.cpp`) | For even rows read from `memory(i+1, j)` and write sequentially; for odd read from `memory(i, j)`. Formula: `file(i, j) = memory(i - !(j&1), j)` |
| **Map generation** (`CMap.cpp`) | Already correct — border check is vertex-position-based, matching s25client's identity mapping |

## Wrappers (`include/Geometry.h`)

| Function | Returns | Use |
|----------|---------|-----|
| `clientUsdX(x, y)` | `x` | Native — matches in-memory s25client storage |
| `editorUsdX(x, y)` | `x - !(y & 1)` | Backward compat for non-updated code |
| `clientUsdTriangleVertices(x, y)` | USD vertex coords | Native — s25client label → vertices |
| `editorUsdTriangleVertices(x, y)` | USD vertex coords | Backward compat — editor label → vertices |
