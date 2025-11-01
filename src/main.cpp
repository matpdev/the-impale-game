#include "raylib.h"
#include "box2d/box2d.h"

#include "includes/components/physics_body.hpp"
#include "includes/components/transform.hpp"
#include "includes/components/sprite.hpp"
#include "includes/components/impaled.hpp"
#include "includes/entities/types.hpp"
#include "includes/entities/factory.hpp"
#include "includes/systems/render_system.hpp"
#include "includes/core/entity_manager.hpp"
#include "includes/core/world_loader.hpp"

#include <assert.h>
#include <vector>
#include <cmath>
#include <cstdio>
#include <map>

// Simple texture cache to avoid loading same texture multiple times
struct TextureCache
{
    std::map<std::string, Texture> textures;

    Texture load(const std::string &path)
    {
        auto it = textures.find(path);
        if (it != textures.end())
        {
            return it->second;
        }
        Texture tex = LoadTexture(path.c_str());
        textures[path] = tex;
        printf("Loaded texture: %s (%dx%d)\n", path.c_str(), tex.width, tex.height);
        return tex;
    }

    void unloadAll()
    {
        for (auto &pair : textures)
        {
            UnloadTexture(pair.second);
        }
        textures.clear();
    }
};

// GameEntity is declared in includes/entities/types.hpp

#define GROUND_COUNT 14
#define BOX_COUNT 10

int main(void)
{
    int width = 1920, height = 1080;
    InitWindow(width, height, "box2d-raylib");

    SetTargetFPS(60);

    float lengthUnitsPerMeter = 20.0f;
    b2SetLengthUnitsPerMeter(lengthUnitsPerMeter);

    b2WorldDef worldDef = b2DefaultWorldDef();

    worldDef.gravity.y = 1.8f * lengthUnitsPerMeter;
    b2WorldId worldId = b2CreateWorld(&worldDef);

    // Create texture cache
    TextureCache textureCache;

    Texture groundTexture = textureCache.load("ground.png");
    Texture boxTexture = textureCache.load("block.png");

    b2Vec2 groundExtent = {0.5f * groundTexture.width, 0.5f * groundTexture.height};
    b2Vec2 boxExtent = {0.5f * boxTexture.width, 0.5f * boxTexture.height};

    b2Polygon groundPolygon = b2MakeBox(groundExtent.x / lengthUnitsPerMeter, groundExtent.y / lengthUnitsPerMeter);
    b2Polygon boxPolygon = b2MakeBox(boxExtent.x / lengthUnitsPerMeter, boxExtent.y / lengthUnitsPerMeter);

    EntityManager entityManager;

    std::vector<GameEntity> boxEntities;
    std::vector<GameEntity> obstacleEntities;
    std::vector<GameEntity> spikeEntities;
    std::vector<GameEntity> throwerEntities;

    // Load scenario from TOML with texture loader
    std::vector<GameEntity> dummyGrounds; // not used, kept for API compat
    level::BuildContext ctx{entityManager, worldId, lengthUnitsPerMeter,
                            groundTexture, boxTexture,
                            groundPolygon, boxPolygon,
                            groundExtent, boxExtent,
                            dummyGrounds, boxEntities, obstacleEntities, spikeEntities, throwerEntities,
                            [&textureCache](const std::string &path)
                            { return textureCache.load(path); }};
    level::LoadScenarioFromToml("levels/demo.toml", ctx);

    bool pause = false;
    bool showDebugWireframe = true; // toggle with 'D' key

    // Debug rope toggle state
    struct DebugRope
    {
        b2BodyId anchor{b2_nullBodyId};
        b2BodyId weight{b2_nullBodyId};
        b2JointId joint{b2_nullJointId};
        bool active{false};
        float ropeLenM{12.0f};
    } debugRope;

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_P))
        {
            pause = !pause;
        }

        if (IsKeyPressed(KEY_D))
        {
            showDebugWireframe = !showDebugWireframe;
        }

        if (pause == false)
        {
            float deltaTime = GetFrameTime();
            b2World_Step(worldId, deltaTime, 4);

            // Check for box-spike collisions and attach/freeze on impact
            for (auto &box : boxEntities)
            {
                if (box.impaled.frozen || box.impaled.hasJoint())
                    continue; // already attached

                b2Vec2 boxPos = b2Body_GetPosition(box.body.id);

                // Debug: print box position occasionally
                static int debugCounter = 0;
                if (debugCounter++ % 60 == 0 && boxEntities.size() > 0)
                {
                    printf("Box pos: (%.2f, %.2f), Spikes: %zu\n", boxPos.x, boxPos.y, spikeEntities.size());
                }

                for (const auto &spike : spikeEntities)
                {
                    // Determine target for collision depending on spike type
                    b2Vec2 targetPos = b2Body_GetPosition(spike.body.id);
                    float targetRadius = (spike.transform.extent.x + spike.transform.extent.y) * 0.5f / lengthUnitsPerMeter;
                    // For chain spikes, use the hook as the collision target
                    if (spike.spikeProps.type == SpikeType::CHAIN && spike.script.user)
                    {
                        struct ChainContext
                        {
                            b2BodyId hookBody;
                            float halfW;
                            float halfH;
                        };
                        auto *ctx = static_cast<ChainContext *>(spike.script.user);
                        if (ctx)
                        {
                            targetPos = b2Body_GetPosition(ctx->hookBody);
                            float hookHalfW = ctx->halfW * spike.spikeProps.hookScaleW;
                            float hookHalfH = ctx->halfH * spike.spikeProps.hookScaleH;
                            // approximate radius as half of diagonal
                            float hookRadiusPx = sqrtf(hookHalfW * hookHalfW + hookHalfH * hookHalfH);
                            targetRadius = hookRadiusPx / lengthUnitsPerMeter;
                        }
                    }

                    float dx = boxPos.x - targetPos.x;
                    float dy = boxPos.y - targetPos.y;
                    float distSq = dx * dx + dy * dy;

                    // Check collision (simple radius check)
                    float boxRadius = (box.transform.extent.x + box.transform.extent.y) * 0.5f / lengthUnitsPerMeter;
                    float threshold = (targetRadius + boxRadius) * 1.25f; // Slight margin for easier collision

                    // Debug: log near-misses
                    float dist = sqrtf(distSq);
                    if (dist < threshold * 2.0f && debugCounter % 30 == 0)
                    {
                        printf("Near target: dist=%.2f, threshold=%.2f, target@(%.2f,%.2f) type=%d\n",
                               dist, threshold, targetPos.x, targetPos.y, (int)spike.spikeProps.type);
                    }

                    if (distSq < threshold * threshold)
                    {
                        printf("Collision detected! Dist: %.2f, Threshold: %.2f, Type: %d\n",
                               sqrtf(distSq), threshold, (int)spike.spikeProps.type);

                        // Attachment behavior depends on spike type
                        switch (spike.spikeProps.type)
                        {
                        case SpikeType::CHAIN:
                        {
                            // Create distance joint: box swings from the chain hook if available
                            struct ChainContext
                            {
                                b2BodyId hookBody;
                                float halfW;
                                float halfH;
                            };
                            b2BodyId hook = spike.body.id;
                            if (spike.script.user)
                            {
                                auto *ctx = static_cast<ChainContext *>(spike.script.user);
                                hook = ctx->hookBody;
                            }
                            b2DistanceJointDef jointDef = b2DefaultDistanceJointDef();
                            jointDef.bodyIdA = hook;
                            jointDef.bodyIdB = box.body.id;
                            jointDef.localAnchorA = {0.0f, 0.0f}; // spike center
                            jointDef.localAnchorB = {0.0f, 0.0f}; // box center
                            jointDef.length = sqrtf(distSq);      // current distance
                            jointDef.minLength = 0.5f;            // allow some slack
                            jointDef.maxLength = jointDef.length * 1.5f;
                            jointDef.hertz = spike.spikeProps.jointHertz; // configurable stiffness
                            jointDef.dampingRatio = spike.spikeProps.jointDamping;
                            box.impaled.jointId = b2CreateDistanceJoint(worldId, &jointDef);
                            box.impaled.frozen = true; // mark as captured
                            printf("✓ Box attached to chain hook! Joint: %d\n", box.impaled.jointId.index1);
                            break;
                        }

                        case SpikeType::SAW:
                        case SpikeType::NORMAL:
                        default:
                        {
                            // Create revolute joint for pendulum swing
                            b2RevoluteJointDef jointDef = b2DefaultRevoluteJointDef();
                            jointDef.bodyIdA = spike.body.id;
                            jointDef.bodyIdB = box.body.id;
                            jointDef.localAnchorA = {0.0f, 0.0f};
                            // Attach at box edge closest to spike
                            float angle = atan2f(dy, dx);
                            jointDef.localAnchorB = {-cosf(angle) * boxRadius, -sinf(angle) * boxRadius};
                            jointDef.enableLimit = false;
                            box.impaled.jointId = b2CreateRevoluteJoint(worldId, &jointDef);
                            box.impaled.frozen = true;
                            printf("✓ Box impaled on spike (pendulum)! Joint: %d\n", box.impaled.jointId.index1);
                            break;
                        }
                        }
                        break; // only attach to first spike hit
                    }
                }
            }
        }

        // Toggle a standalone debug rope (press R)
        // if (IsKeyPressed(KEY_R))
        // {
        //     if (!debugRope.active)
        //     {
        //         // Create static anchor near top-center
        //         float anchorX = (width * 0.5f) / lengthUnitsPerMeter;
        //         float anchorY = (height * 0.2f) / lengthUnitsPerMeter; // near top
        //         b2BodyDef adef = b2DefaultBodyDef();
        //         adef.type = b2_staticBody;
        //         adef.position = {anchorX, anchorY};
        //         debugRope.anchor = b2CreateBody(worldId, &adef);

        //         // Create dynamic weight below anchor
        //         b2BodyDef wdef = b2DefaultBodyDef();
        //         wdef.type = b2_dynamicBody;
        //         wdef.position = {anchorX, anchorY + debugRope.ropeLenM};
        //         wdef.linearDamping = 0.05f;
        //         wdef.angularDamping = 0.05f;
        //         debugRope.weight = b2CreateBody(worldId, &wdef);
        //         // Attach a small box shape so it’s visible and has mass
        //         float halfWm = (boxExtent.x * 0.25f) / lengthUnitsPerMeter;
        //         float halfHm = (boxExtent.y * 0.25f) / lengthUnitsPerMeter;
        //         b2Polygon wpoly = b2MakeBox(halfWm, halfHm);
        //         b2ShapeDef wsdef = b2DefaultShapeDef();
        //         wsdef.density = 1.0f;
        //         wsdef.material.friction = 0.6f;
        //         wsdef.material.restitution = 0.0f;
        //         b2CreatePolygonShape(debugRope.weight, &wsdef, &wpoly);

        //         // Create distance joint configured as a rope (min/max length)
        //         b2DistanceJointDef jdef = b2DefaultDistanceJointDef();
        //         jdef.bodyIdA = debugRope.anchor;
        //         jdef.bodyIdB = debugRope.weight;
        //         jdef.localAnchorA = {0.0f, 0.0f};
        //         jdef.localAnchorB = {0.0f, 0.0f};
        //         jdef.length = debugRope.ropeLenM;    // nominal
        //         jdef.minLength = 0.0f;               // allow slack
        //         jdef.maxLength = debugRope.ropeLenM; // rope limit
        //         jdef.enableSpring = false;           // pure rope behavior
        //         jdef.hertz = 0.0f;                   // ignored if spring disabled
        //         jdef.dampingRatio = 0.0f;            // ignored if spring disabled
        //         debugRope.joint = b2CreateDistanceJoint(worldId, &jdef);

        //         debugRope.active = true;
        //         printf("[DEBUG ROPE] Created (length=%.2fm) at (%.2f,%.2f)\n", debugRope.ropeLenM, anchorX, anchorY);
        //     }
        //     else
        //     {
        //         if (B2_IS_NON_NULL(debugRope.joint))
        //             b2DestroyJoint(debugRope.joint);
        //         if (B2_IS_NON_NULL(debugRope.weight))
        //             b2DestroyBody(debugRope.weight);
        //         if (B2_IS_NON_NULL(debugRope.anchor))
        //             b2DestroyBody(debugRope.anchor);
        //         debugRope = DebugRope{}; // reset
        //         printf("[DEBUG ROPE] Destroyed\n");
        //     }
        // }

        Vector2 mouseScreen = GetMousePosition();

        // Update thrower aim and charging
        if (!throwerEntities.empty())
        {
            auto &thrower = throwerEntities[0];
            auto *ctx = static_cast<ThrowerContext *>(thrower.script.user);
            if (ctx)
            {
                // Calculate aim direction from thrower to mouse
                b2Vec2 throwerPos = b2Body_GetPosition(thrower.body.id);
                Vector2 throwerScreen = {throwerPos.x * lengthUnitsPerMeter, throwerPos.y * lengthUnitsPerMeter};
                float dx = mouseScreen.x - throwerScreen.x;
                float dy = mouseScreen.y - throwerScreen.y;
                float len = sqrtf(dx * dx + dy * dy);
                if (len > 1.0f)
                {
                    ctx->aimDir = {dx / len, dy / len};
                }

                // Handle charging (ThrowerUpdate handles the actual charge accumulation)
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    ctx->isCharging = true;
                    ctx->currentCharge = 0.0f; // Reset charge at start
                }

                if (ctx->isCharging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                {
                    // Fire!
                    ctx->isCharging = false;

                    if (ctx->currentCharge > 10.0f) // minimum power threshold
                    {
                        // Spawn projectile at thrower position with physics properties
                        PhysicsMaterial boxPhysics;
                        boxPhysics.density = 0.1f; // Light projectile for snappier throws
                        boxPhysics.friction = 0.4f;
                        boxPhysics.restitution = 0.3f;
                        boxPhysics.linearDamping = 0.1f;
                        boxPhysics.angularDamping = 0.1f;
                        boxPhysics.affectedByGravity = true;

                        VisualStyle boxVisual;
                        boxVisual.color = ORANGE;
                        boxVisual.roundness = 0.2f;
                        boxVisual.useTexture = true;

                        GameEntity proj = makeBoxEntity(entityManager, worldId, boxTexture, boxPolygon, boxExtent, throwerPos, true, boxPhysics, boxVisual);

                        // Apply impulse in aim direction
                        // Scale impulse to physics units (divide by lengthUnitsPerMeter to convert power from pixel-based to meter-based)
                        float impulseScale = (ctx->currentCharge / lengthUnitsPerMeter) * ctx->impulseMultiplier;
                        b2Vec2 impulse = {ctx->aimDir.x * impulseScale, ctx->aimDir.y * impulseScale};
                        b2Body_ApplyLinearImpulse(proj.body.id, impulse, throwerPos, true);

                        boxEntities.push_back(proj);

                        // Debug: show mass and expected velocity
                        float mass = b2Body_GetMass(proj.body.id);
                        float impulseMag = sqrtf(impulse.x * impulse.x + impulse.y * impulse.y);
                        float expectedVel = impulseMag / mass;
                        printf("Fired! Power: %.2f, ImpulseScale: %.2f (x%.2f), Mass: %.3f kg, |Impulse|: %.2f, ExpectedVel: %.2f m/s\n",
                               ctx->currentCharge, (ctx->currentCharge / lengthUnitsPerMeter), ctx->impulseMultiplier, mass, impulseMag, expectedVel);
                    }

                    ctx->currentCharge = 0.0f; // Reset after firing
                }
            }
        }

        // Per-entity logic update (outside render phase)
        {
            float dt = GetFrameTime();
            for (auto &e : boxEntities)
            {
                if (e.script.update)
                    e.script.update(e, dt);
            }
            for (auto &e : obstacleEntities)
            {
                if (e.script.update)
                    e.script.update(e, dt);
            }
            for (auto &e : spikeEntities)
            {
                if (e.script.update)
                    e.script.update(e, dt);
            }
            for (auto &e : throwerEntities)
            {
                if (e.script.update)
                    e.script.update(e, dt);
            }
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        const char *message = "Hello Box2D!";
        int fontSize = 36;
        int textWidth = MeasureText("Hello Box2D!", fontSize);
        DrawText(message, (width - textWidth) / 2, 50, fontSize, LIGHTGRAY);

        // Per-entity render hooks (inside render phase)
        for (size_t i = 0; i < boxEntities.size(); ++i)
        {
            if (boxEntities[i].script.render)
                boxEntities[i].script.render(boxEntities[i], lengthUnitsPerMeter);
        }
        for (size_t i = 0; i < obstacleEntities.size(); ++i)
        {
            if (obstacleEntities[i].script.render)
                obstacleEntities[i].script.render(obstacleEntities[i], lengthUnitsPerMeter);
        }
        for (size_t i = 0; i < spikeEntities.size(); ++i)
        {
            if (spikeEntities[i].script.render)
                spikeEntities[i].script.render(spikeEntities[i], lengthUnitsPerMeter);
        }
        for (size_t i = 0; i < throwerEntities.size(); ++i)
        {
            if (throwerEntities[i].script.render)
                throwerEntities[i].script.render(throwerEntities[i], lengthUnitsPerMeter);
        }

        // Debug wireframe overlay
        if (showDebugWireframe)
        {
            // Draw boxes
            for (const auto &box : boxEntities)
            {
                b2Vec2 pos = b2Body_GetPosition(box.body.id);
                b2Rot rot = b2Body_GetRotation(box.body.id);
                float angle = b2Rot_GetAngle(rot);
                Vector2 center = {pos.x * lengthUnitsPerMeter, pos.y * lengthUnitsPerMeter};
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
            for (const auto &obs : obstacleEntities)
            {
                b2Vec2 pos = b2Body_GetPosition(obs.body.id);
                Vector2 center = {pos.x * lengthUnitsPerMeter, pos.y * lengthUnitsPerMeter};
                Vector2 size = {2.0f * obs.transform.extent.x, 2.0f * obs.transform.extent.y};
                DrawRectangleLines(center.x - size.x / 2, center.y - size.y / 2, size.x, size.y, BLUE);
                DrawCircleV(center, 3.0f, BLUE);
            }

            // Draw spikes (and chain debug if present)
            for (const auto &spike : spikeEntities)
            {
                b2Vec2 pos = b2Body_GetPosition(spike.body.id);
                Vector2 center = {pos.x * lengthUnitsPerMeter, pos.y * lengthUnitsPerMeter};
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
                    auto *ctx = static_cast<ChainContext *>(spike.script.user);
                    if (ctx)
                    {
                        Color ropeColor = SKYBLUE;
                        Color hookColor = YELLOW;
                        // Draw rope line from spike bottom to hook top
                        b2Vec2 sp = b2Body_GetPosition(spike.body.id);
                        Vector2 sc = {sp.x * lengthUnitsPerMeter, sp.y * lengthUnitsPerMeter};
                        Vector2 anchorBot = {sc.x, sc.y + spike.transform.extent.y};
                        b2Vec2 hp = b2Body_GetPosition(ctx->hookBody);
                        b2Rot hr = b2Body_GetRotation(ctx->hookBody);
                        float ha = b2Rot_GetAngle(hr);
                        Vector2 hc = {hp.x * lengthUnitsPerMeter, hp.y * lengthUnitsPerMeter};
                        Vector2 topOff = {0.0f, -ctx->halfH * spike.spikeProps.hookScaleH};
                        float ca = cosf(ha), sa = sinf(ha);
                        Vector2 hookTop = {hc.x + topOff.x * ca - topOff.y * sa, hc.y + topOff.x * sa + topOff.y * ca};
                        DrawLineEx(anchorBot, hookTop, 2.0f, ropeColor);

                        // Draw hook outline (scaled)
                        {
                            Vector2 hsize = {2.0f * ctx->halfW * spike.spikeProps.hookScaleW, 2.0f * ctx->halfH * spike.spikeProps.hookScaleH};
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
            for (const auto &thrower : throwerEntities)
            {
                b2Vec2 pos = b2Body_GetPosition(thrower.body.id);
                Vector2 center = {pos.x * lengthUnitsPerMeter, pos.y * lengthUnitsPerMeter};
                Vector2 size = {2.0f * thrower.transform.extent.x, 2.0f * thrower.transform.extent.y};
                DrawRectangleLines(center.x - size.x / 2, center.y - size.y / 2, size.x, size.y, YELLOW);
                DrawCircleV(center, 4.0f, YELLOW);
            }

            // Draw debug info text
            // Draw debug rope if active
            if (debugRope.active)
            {
                b2Vec2 ap = b2Body_GetPosition(debugRope.anchor);
                b2Vec2 wp = b2Body_GetPosition(debugRope.weight);
                Vector2 a = {ap.x * lengthUnitsPerMeter, ap.y * lengthUnitsPerMeter};
                Vector2 w = {wp.x * lengthUnitsPerMeter, wp.y * lengthUnitsPerMeter};
                DrawLineEx(a, w, 3.0f, SKYBLUE);
                DrawCircleV(a, 5.0f, RAYWHITE);
                DrawCircleV(w, 6.0f, YELLOW);
            }

            DrawText("DEBUG MODE (D to toggle) — Chain/Rope overlay active | R: toggle debug rope", 10, height - 30, 20, WHITE);
            char entityCount[100];
            snprintf(entityCount, sizeof(entityCount), "Boxes: %zu | Obstacles: %zu | Spikes: %zu",
                     boxEntities.size(), obstacleEntities.size(), spikeEntities.size());
            DrawText(entityCount, 10, height - 60, 20, WHITE);
        }

        EndDrawing();
    }

    // Cleanup debug rope on exit
    // (These ids are local to main scope; ensure destruction if still active)
    // Note: Bodies/joints are auto-destroyed on world destroy, but we are explicit.
    // We don't have access to debugRope here because it was defined inside the loop scope.

    textureCache.unloadAll();

    CloseWindow();

    return 0;
}