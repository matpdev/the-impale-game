#pragma once
#include "box2d/box2d.h"

// SpriteTransform component: stores sprite/shape half-extent in pixels
// (Project convention: extent values are in pixels; conversions handled in systems)
struct SpriteTransform
{
    b2Vec2 extent; // half-size in pixels
};
