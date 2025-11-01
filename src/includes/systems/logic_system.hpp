#pragma once
#include "raylib.h"
#include "box2d/box2d.h"

#include "../entities/types.hpp"
#include "../entities/factory.hpp"
#include "../core/entity_manager.hpp"

#include <vector>
#include <cmath>

// Context for logic updates
struct LogicContext
{
    b2WorldId worldId;
    float lengthUnitsPerMeter;
    std::vector<GameEntity> &boxes;
    std::vector<GameEntity> &obstacles;
    std::vector<GameEntity> &spikes;
    std::vector<GameEntity> &throwers;
    EntityManager &entityManager;
    Texture &boxTexture;
    b2Polygon &boxPolygon;
    b2Vec2 &boxExtent;
    bool isPaused;
};

// Main logic update: physics, collision, input, entity updates
inline void UpdateLogic(LogicContext &ctx, float deltaTime)
{
    if (ctx.isPaused)
        return;

    // Step physics simulation
    b2World_Step(ctx.worldId, deltaTime, 4);

    // Check for box-spike collisions and attach/freeze on impact
    for (auto &box : ctx.boxes)
    {
        if (box.impaled.frozen || box.impaled.hasJoint())
            continue; // already attached

        b2Vec2 boxPos = b2Body_GetPosition(box.body.id);

        for (const auto &spike : ctx.spikes)
        {
            // Determine target for collision depending on spike type
            b2Vec2 targetPos = b2Body_GetPosition(spike.body.id);
            float targetRadius = (spike.transform.extent.x + spike.transform.extent.y) * 0.5f / ctx.lengthUnitsPerMeter;

            // For chain spikes, use the hook as the collision target
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
                    targetPos = b2Body_GetPosition(spikeCtx->hookBody);
                    float hookHalfW = spikeCtx->halfW * spike.spikeProps.hookScaleW;
                    float hookHalfH = spikeCtx->halfH * spike.spikeProps.hookScaleH;
                    // approximate radius as half of diagonal
                    float hookRadiusPx = sqrtf(hookHalfW * hookHalfW + hookHalfH * hookHalfH);
                    targetRadius = hookRadiusPx / ctx.lengthUnitsPerMeter;
                }
            }

            float dx = boxPos.x - targetPos.x;
            float dy = boxPos.y - targetPos.y;
            float distSq = dx * dx + dy * dy;

            // Check collision (simple radius check)
            float boxRadius = (box.transform.extent.x + box.transform.extent.y) * 0.5f / ctx.lengthUnitsPerMeter;
            float threshold = (targetRadius + boxRadius) * 1.25f; // Slight margin for easier collision

            if (distSq < threshold * threshold)
            {
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
                        auto *spikeCtx = static_cast<ChainContext *>(spike.script.user);
                        hook = spikeCtx->hookBody;
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
                    box.impaled.jointId = b2CreateDistanceJoint(ctx.worldId, &jointDef);
                    box.impaled.frozen = true; // mark as captured
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
                    box.impaled.jointId = b2CreateRevoluteJoint(ctx.worldId, &jointDef);
                    box.impaled.frozen = true;
                    break;
                }
                }
                break; // only attach to first spike hit
            }
        }
    }

    // Update thrower aim and charging
    Vector2 mouseScreen = GetMousePosition();

    if (!ctx.throwers.empty())
    {
        auto &thrower = ctx.throwers[0];
        auto *throwerCtx = static_cast<ThrowerContext *>(thrower.script.user);
        if (throwerCtx)
        {
            // Calculate aim direction from thrower to mouse
            b2Vec2 throwerPos = b2Body_GetPosition(thrower.body.id);
            Vector2 throwerScreen = {throwerPos.x * ctx.lengthUnitsPerMeter, throwerPos.y * ctx.lengthUnitsPerMeter};
            float dx = mouseScreen.x - throwerScreen.x;
            float dy = mouseScreen.y - throwerScreen.y;
            float len = sqrtf(dx * dx + dy * dy);
            if (len > 1.0f)
            {
                throwerCtx->aimDir = {dx / len, dy / len};
            }

            // Handle charging (ThrowerUpdate handles the actual charge accumulation)
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                throwerCtx->isCharging = true;
                throwerCtx->currentCharge = 0.0f; // Reset charge at start
            }

            if (throwerCtx->isCharging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            {
                // Fire!
                throwerCtx->isCharging = false;

                if (throwerCtx->currentCharge > 10.0f) // minimum power threshold
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

                    GameEntity proj = makeBoxEntity(ctx.entityManager, ctx.worldId, ctx.boxTexture,
                                                    ctx.boxPolygon, ctx.boxExtent, throwerPos,
                                                    true, boxPhysics, boxVisual);

                    // Apply impulse in aim direction
                    // Scale impulse to physics units (divide by lengthUnitsPerMeter to convert power from pixel-based to meter-based)
                    float impulseScale = (throwerCtx->currentCharge / ctx.lengthUnitsPerMeter) * throwerCtx->impulseMultiplier;
                    b2Vec2 impulse = {throwerCtx->aimDir.x * impulseScale, throwerCtx->aimDir.y * impulseScale};
                    b2Body_ApplyLinearImpulse(proj.body.id, impulse, throwerPos, true);

                    ctx.boxes.push_back(proj);

                    throwerCtx->currentCharge = 0.0f; // Reset after firing
                }
            }
        }
    }

    // Per-entity logic update
    auto updateEntities = [deltaTime](std::vector<GameEntity> &entities)
    {
        for (auto &e : entities)
        {
            if (e.script.update)
                e.script.update(e, deltaTime);
        }
    };

    updateEntities(ctx.boxes);
    updateEntities(ctx.obstacles);
    updateEntities(ctx.spikes);
    updateEntities(ctx.throwers);
}
