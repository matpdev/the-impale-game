#pragma once
#include "raylib.h"

// Visual styling component for customizable appearance
struct VisualStyle
{
    Color color{WHITE};
    float roundness{0.0f}; // 0.0 = sharp corners, 1.0 = fully rounded
    bool useTexture{true}; // if false, use solid color only
};
