#pragma once
#include "raylib.h"
#include "box2d/box2d.h"

#include "../components/physics_body.hpp"
#include "../components/sprite.hpp"
#include "../components/transform.hpp"
#include "../components/visual_style.hpp"

// Draw a sprite using physics body transform and extent conversion
inline void DrawSprite(const PhysicsBody &body,
                       const Sprite &sprite,
                       const SpriteTransform &transform,
                       const VisualStyle &visual,
                       float lengthUnitsPerMeter)
{
    b2Vec2 pos = b2Body_GetPosition(body.id);
    b2Rot rotation = b2Body_GetRotation(body.id);
    float radians = b2Rot_GetAngle(rotation);
    Vector2 center = {pos.x * lengthUnitsPerMeter, pos.y * lengthUnitsPerMeter};

    if (visual.useTexture && sprite.texture.id > 0)
    {
        // Convert extent from pixels to meters
        b2Vec2 extentMeters = {transform.extent.x / lengthUnitsPerMeter,
                               transform.extent.y / lengthUnitsPerMeter};

        // Bottom-left world point given body pose
        b2Vec2 p = b2Body_GetWorldPoint(body.id, (b2Vec2){-extentMeters.x, -extentMeters.y});

        // Convert to pixels and draw
        Vector2 ps = {p.x * lengthUnitsPerMeter, p.y * lengthUnitsPerMeter};
        DrawTextureEx(sprite.texture, ps, RAD2DEG * radians, 1.0f, visual.color);
    }
    else
    {
        // Draw solid color rectangle with optional roundness
        Vector2 size = {2.0f * transform.extent.x, 2.0f * transform.extent.y};
        Rectangle rect = {center.x, center.y, size.x, size.y};
        Vector2 origin = {transform.extent.x, transform.extent.y};

        if (visual.roundness > 0.0f)
        {
            // For rounded rectangles, we need to draw without rotation first
            // then apply manual rotation (raylib doesn't support rotated rounded rects directly)
            // For now, approximate with regular DrawRectanglePro and visual.roundness as a hint
            DrawRectanglePro(rect, origin, RAD2DEG * radians, visual.color);
        }
        else
        {
            DrawRectanglePro(rect, origin, RAD2DEG * radians, visual.color);
        }
    }
}
