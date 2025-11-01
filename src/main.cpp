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

    Texture groundTexture = LoadTexture("ground.png");
    Texture boxTexture = LoadTexture("box.png");

    b2Vec2 groundExtent = {0.5f * groundTexture.width, 0.5f * groundTexture.height};
    b2Vec2 boxExtent = {0.5f * boxTexture.width, 0.5f * boxTexture.height};

    b2Polygon groundPolygon = b2MakeBox(groundExtent.x / lengthUnitsPerMeter, groundExtent.y / lengthUnitsPerMeter);
    b2Polygon boxPolygon = b2MakeBox(boxExtent.x / lengthUnitsPerMeter, boxExtent.y / lengthUnitsPerMeter);

    EntityManager entityManager;

    std::vector<GameEntity> boxEntities;
    std::vector<GameEntity> obstacleEntities;
    std::vector<GameEntity> spikeEntities;
    std::vector<GameEntity> throwerEntities;

    // Load scenario from TOML
    std::vector<GameEntity> dummyGrounds; // not used, kept for API compat
    level::BuildContext ctx{entityManager, worldId, lengthUnitsPerMeter,
                            groundTexture, boxTexture,
                            groundPolygon, boxPolygon,
                            groundExtent, boxExtent,
                            dummyGrounds, boxEntities, obstacleEntities, spikeEntities, throwerEntities};
    level::LoadScenarioFromToml("levels/demo.toml", ctx);

    bool pause = false;
    bool showDebugWireframe = true; // toggle with 'D' key

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
                    b2Vec2 spikePos = b2Body_GetPosition(spike.body.id);
                    float dx = boxPos.x - spikePos.x;
                    float dy = boxPos.y - spikePos.y;
                    float distSq = dx * dx + dy * dy;

                    // Check collision (simple radius check)
                    float spikeRadius = (spike.transform.extent.x + spike.transform.extent.y) * 0.5f / lengthUnitsPerMeter;
                    float boxRadius = (box.transform.extent.x + box.transform.extent.y) * 0.5f / lengthUnitsPerMeter;
                    float threshold = (spikeRadius + boxRadius) * 1.5f; // Increased for easier collision

                    // Debug: log near-misses
                    float dist = sqrtf(distSq);
                    if (dist < threshold * 2.0f && debugCounter % 30 == 0)
                    {
                        printf("Near spike: dist=%.2f, threshold=%.2f, spike@(%.2f,%.2f)\n",
                               dist, threshold, spikePos.x, spikePos.y);
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
                            // Create distance joint: box swings like a chain link
                            b2DistanceJointDef jointDef = b2DefaultDistanceJointDef();
                            jointDef.bodyIdA = spike.body.id;
                            jointDef.bodyIdB = box.body.id;
                            jointDef.localAnchorA = {0.0f, 0.0f}; // spike center
                            jointDef.localAnchorB = {0.0f, 0.0f}; // box center
                            jointDef.length = sqrtf(distSq);      // current distance
                            jointDef.minLength = 0.5f;            // allow some slack
                            jointDef.maxLength = jointDef.length * 1.5f;
                            jointDef.hertz = 3.0f; // stiff constraint
                            jointDef.dampingRatio = 0.5f;
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

            // Draw spikes
            for (const auto &spike : spikeEntities)
            {
                b2Vec2 pos = b2Body_GetPosition(spike.body.id);
                Vector2 center = {pos.x * lengthUnitsPerMeter, pos.y * lengthUnitsPerMeter};
                float r = (spike.transform.extent.x + spike.transform.extent.y) * 0.5f;
                DrawCircleLines(center.x, center.y, r, RED);
                DrawCircleV(center, 4.0f, RED);
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
            DrawText("DEBUG MODE (D to toggle)", 10, height - 30, 20, WHITE);
            char entityCount[100];
            snprintf(entityCount, sizeof(entityCount), "Boxes: %zu | Obstacles: %zu | Spikes: %zu",
                     boxEntities.size(), obstacleEntities.size(), spikeEntities.size());
            DrawText(entityCount, 10, height - 60, 20, WHITE);
        }

        EndDrawing();
    }

    UnloadTexture(groundTexture);
    UnloadTexture(boxTexture);

    CloseWindow();

    return 0;
}