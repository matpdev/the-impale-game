#pragma once
#include "raylib.h"
#include "box2d/box2d.h"

#include "../components/physics_body.hpp"
#include "../components/sprite.hpp"
#include "../components/transform.hpp"
#include "../components/visual_style.hpp"
#include "../entities/types.hpp"

#include <vector>
#include <cmath>
#include <cstdio>

// Debug rope structure for visualization
struct DebugRope
{
    b2BodyId anchor{b2_nullBodyId};
    b2BodyId weight{b2_nullBodyId};
    b2JointId joint{b2_nullJointId};
    bool active{false};
    float ropeLenM{12.0f};
};

// Context for rendering
struct RenderContext
{
    int screenWidth;
    int screenHeight;
    float lengthUnitsPerMeter;
    std::vector<GameEntity> &boxes;
    std::vector<GameEntity> &obstacles;
    std::vector<GameEntity> &spikes;
    std::vector<GameEntity> &throwers;
    bool showDebugWireframe;
    DebugRope *debugRope; // optional
};

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

// Main render function: handles all drawing for a frame
inline void RenderFrame(const RenderContext &ctx)
{
    BeginDrawing();
    ClearBackground(DARKGRAY);

    // Draw title
    const char *message = "Hello Box2D!";
    int fontSize = 36;
    int textWidth = MeasureText(message, fontSize);
    DrawText(message, (ctx.screenWidth - textWidth) / 2, 50, fontSize, LIGHTGRAY);

    // Per-entity render hooks
    auto renderEntities = [&ctx](const std::vector<GameEntity> &entities)
    {
        for (const auto &e : entities)
        {
            if (e.script.render)
                e.script.render(e, ctx.lengthUnitsPerMeter);
        }
    };

    renderEntities(ctx.boxes);
    renderEntities(ctx.obstacles);
    renderEntities(ctx.spikes);
    renderEntities(ctx.throwers);

    // Debug wireframe overlay
    if (ctx.showDebugWireframe)
    {
        // Draw boxes
        for (const auto &box : ctx.boxes)
        {
            b2Vec2 pos = b2Body_GetPosition(box.body.id);
            b2Rot rot = b2Body_GetRotation(box.body.id);
            float angle = b2Rot_GetAngle(rot);
            Vector2 center = {pos.x * ctx.lengthUnitsPerMeter, pos.y * ctx.lengthUnitsPerMeter};
            Vector2 size = {2.0f * box.transform.extent.x, 2.0f * box.transform.extent.y};

            Color wireColor = box.impaled.frozen ? GREEN : LIME;
            DrawRectanglePro(
                (Rectangle){center.x, center.y, size.x, size.y},
                (Vector2){box.transform.extent.x, box.transform.extent.y},
                RAD2DEG * angle,
                Fade(wireColor, 0.0f));
            // Draw outline
            DrawRectangleLines(center.x - size.x / 2, center.y - size.y / 2, size.x, size.y, wireColor);

            // Draw center point
            DrawCircleV(center, 3.0f, wireColor);
        }

        // Draw obstacles
        for (const auto &obs : ctx.obstacles)
        {
            b2Vec2 pos = b2Body_GetPosition(obs.body.id);
            Vector2 center = {pos.x * ctx.lengthUnitsPerMeter, pos.y * ctx.lengthUnitsPerMeter};
            Vector2 size = {2.0f * obs.transform.extent.x, 2.0f * obs.transform.extent.y};
            DrawRectangleLines(center.x - size.x / 2, center.y - size.y / 2, size.x, size.y, BLUE);
            DrawCircleV(center, 3.0f, BLUE);
        }

        // Draw spikes (and chain debug if present)
        for (const auto &spike : ctx.spikes)
        {
            b2Vec2 pos = b2Body_GetPosition(spike.body.id);
            Vector2 center = {pos.x * ctx.lengthUnitsPerMeter, pos.y * ctx.lengthUnitsPerMeter};
            float r = (spike.transform.extent.x + spike.transform.extent.y) * 0.5f;
            DrawCircleLines(center.x, center.y, r, RED);
            DrawCircleV(center, 4.0f, RED);

            // Chain/Rope system debug overlay (bright outlines)
            if (spike.spikeProps.type == SpikeType::CHAIN && spike.script.user)
            {
                struct ChainContext
                {
                    b2BodyId hookBody;
                    float halfW;
                    float halfH;
                };
                auto *spikeCtx = static_cast<ChainContext *>(spike.script.user);
                if (spikeCtx)
                {
                    Color ropeColor = SKYBLUE;
                    Color hookColor = YELLOW;
                    // Draw rope line from spike bottom to hook top
                    b2Vec2 sp = b2Body_GetPosition(spike.body.id);
                    Vector2 sc = {sp.x * ctx.lengthUnitsPerMeter, sp.y * ctx.lengthUnitsPerMeter};
                    Vector2 anchorBot = {sc.x, sc.y + spike.transform.extent.y};
                    b2Vec2 hp = b2Body_GetPosition(spikeCtx->hookBody);
                    b2Rot hr = b2Body_GetRotation(spikeCtx->hookBody);
                    float ha = b2Rot_GetAngle(hr);
                    Vector2 hc = {hp.x * ctx.lengthUnitsPerMeter, hp.y * ctx.lengthUnitsPerMeter};
                    Vector2 topOff = {0.0f, -spikeCtx->halfH * spike.spikeProps.hookScaleH};
                    float ca = cosf(ha), sa = sinf(ha);
                    Vector2 hookTop = {hc.x + topOff.x * ca - topOff.y * sa, hc.y + topOff.x * sa + topOff.y * ca};
                    DrawLineEx(anchorBot, hookTop, 2.0f, ropeColor);

                    // Draw hook outline (scaled)
                    {
                        Vector2 hsize = {2.0f * spikeCtx->halfW * spike.spikeProps.hookScaleW,
                                         2.0f * spikeCtx->halfH * spike.spikeProps.hookScaleH};
                        Vector2 half = {hsize.x * 0.5f, hsize.y * 0.5f};
                        Vector2 corners[4] = {
                            {-half.x, -half.y}, {half.x, -half.y}, {half.x, half.y}, {-half.x, half.y}};
                        ca = cosf(ha);
                        sa = sinf(ha);
                        for (int i = 0; i < 4; ++i)
                        {
                            float rx = corners[i].x * ca - corners[i].y * sa;
                            float ry = corners[i].x * sa + corners[i].y * ca;
                            corners[i].x = hc.x + rx;
                            corners[i].y = hc.y + ry;
                        }
                        for (int i = 0; i < 4; ++i)
                        {
                            int j = (i + 1) % 4;
                            DrawLineEx(corners[i], corners[j], 3.0f, hookColor);
                        }
                        DrawCircleV(hc, 3.0f, hookColor);
                    }
                }
            }
        }

        // Draw thrower
        for (const auto &thrower : ctx.throwers)
        {
            b2Vec2 pos = b2Body_GetPosition(thrower.body.id);
            Vector2 center = {pos.x * ctx.lengthUnitsPerMeter, pos.y * ctx.lengthUnitsPerMeter};
            Vector2 size = {2.0f * thrower.transform.extent.x, 2.0f * thrower.transform.extent.y};
            DrawRectangleLines(center.x - size.x / 2, center.y - size.y / 2, size.x, size.y, YELLOW);
            DrawCircleV(center, 4.0f, YELLOW);
        }

        // Draw debug rope if active
        if (ctx.debugRope && ctx.debugRope->active)
        {
            b2Vec2 ap = b2Body_GetPosition(ctx.debugRope->anchor);
            b2Vec2 wp = b2Body_GetPosition(ctx.debugRope->weight);
            Vector2 a = {ap.x * ctx.lengthUnitsPerMeter, ap.y * ctx.lengthUnitsPerMeter};
            Vector2 w = {wp.x * ctx.lengthUnitsPerMeter, wp.y * ctx.lengthUnitsPerMeter};
            DrawLineEx(a, w, 3.0f, SKYBLUE);
            DrawCircleV(a, 5.0f, RAYWHITE);
            DrawCircleV(w, 6.0f, YELLOW);
        }

        DrawText("DEBUG MODE (D to toggle) — Chain/Rope overlay active | R: toggle debug rope",
                 10, ctx.screenHeight - 30, 20, WHITE);
        char entityCount[100];
        snprintf(entityCount, sizeof(entityCount), "Boxes: %zu | Obstacles: %zu | Spikes: %zu",
                 ctx.boxes.size(), ctx.obstacles.size(), ctx.spikes.size());
        DrawText(entityCount, 10, ctx.screenHeight - 60, 20, WHITE);
    }

    EndDrawing();
}
