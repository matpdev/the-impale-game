#pragma once
#include "raylib.h"
#include "box2d/box2d.h"

#include "types.hpp"
#include <vector>
#include "../components/physics_body.hpp"
#include "../components/transform.hpp"
#include "../components/sprite.hpp"
#include "../components/script.hpp"
#include "../systems/render_system.hpp"
#include "../core/entity_manager.hpp"

// Internal helper to construct a generic entity with given body type
inline void DefaultUpdate(GameEntity & /*e*/, float /*dt*/)
{
    // no-op by default
}

inline void SpikeUpdate(GameEntity &e, float dt)
{
    // Rotate saw blades
    if (e.spikeProps.type == SpikeType::SAW && e.spikeProps.rotationSpeed != 0.0f)
    {
        e.spikeProps.currentRotation += e.spikeProps.rotationSpeed * dt;
        if (e.spikeProps.currentRotation >= 360.0f)
            e.spikeProps.currentRotation -= 360.0f;
        if (e.spikeProps.currentRotation < 0.0f)
            e.spikeProps.currentRotation += 360.0f;
    }
}

inline void DefaultRender(const GameEntity &e, float unitsPerMeter)
{
    DrawSprite(e.body, e.sprite, e.transform, e.visual, unitsPerMeter);
}

inline void DrawSolidBox(const GameEntity &e, float unitsPerMeter, Color color)
{
    // Draw axis-aligned rectangle at body's transform using extent from transform
    b2Vec2 p = b2Body_GetPosition(e.body.id);
    b2Rot rot = b2Body_GetRotation(e.body.id);
    float radians = b2Rot_GetAngle(rot);
    Vector2 center = {p.x * unitsPerMeter, p.y * unitsPerMeter};
    Vector2 size = {2.0f * e.transform.extent.x, 2.0f * e.transform.extent.y};
    DrawRectanglePro((Rectangle){center.x, center.y, size.x, size.y},
                     (Vector2){e.transform.extent.x, e.transform.extent.y},
                     RAD2DEG * radians, color);
}

inline void ObstacleRender(const GameEntity &e, float unitsPerMeter)
{
    // Draw textured rectangle at body's transform
    b2Vec2 p = b2Body_GetPosition(e.body.id);
    b2Rot rot = b2Body_GetRotation(e.body.id);
    float radians = b2Rot_GetAngle(rot);
    Vector2 center = {p.x * unitsPerMeter, p.y * unitsPerMeter};
    Vector2 size = {2.0f * e.transform.extent.x, 2.0f * e.transform.extent.y};

    Rectangle source = {0, 0, (float)e.sprite.texture.width, (float)e.sprite.texture.height};
    Rectangle dest = {center.x, center.y, size.x, size.y};
    Vector2 origin = {e.transform.extent.x, e.transform.extent.y};

    DrawTexturePro(e.sprite.texture, source, dest, origin, RAD2DEG * radians, WHITE);
}

inline void SpikeRender(const GameEntity &e, float unitsPerMeter)
{
    b2Vec2 p = b2Body_GetPosition(e.body.id);
    Vector2 center = {p.x * unitsPerMeter, p.y * unitsPerMeter};
    float r = 0.5f * (e.transform.extent.x + e.transform.extent.y);

    switch (e.spikeProps.type)
    {
    case SpikeType::NORMAL:
        // Draw textured square for spike
        {
            b2Rot rot = b2Body_GetRotation(e.body.id);
            float radians = b2Rot_GetAngle(rot);
            Vector2 size = {2.0f * e.transform.extent.x, 2.0f * e.transform.extent.y};

            Rectangle source = {0, 0, (float)e.sprite.texture.width, (float)e.sprite.texture.height};
            Rectangle dest = {center.x, center.y, size.x, size.y};
            Vector2 origin = {e.transform.extent.x, e.transform.extent.y};

            DrawTexturePro(e.sprite.texture, source, dest, origin, RAD2DEG * radians, WHITE);
        }
        break;

    case SpikeType::SAW:
        // Rotating saw blade
        DrawCircleV(center, r, e.visual.color);
        for (int i = 0; i < 8; ++i)
        {
            float a = (2.0f * PI * i) / 8.0f + DEG2RAD * e.spikeProps.currentRotation;
            Vector2 tooth1 = {center.x + cosf(a) * r, center.y + sinf(a) * r};
            Vector2 tooth2 = {center.x + cosf(a + 0.3f) * (r * 1.3f), center.y + sinf(a + 0.3f) * (r * 1.3f)};
            Vector2 tooth3 = {center.x + cosf(a + 0.6f) * r, center.y + sinf(a + 0.6f) * r};
            DrawTriangle(tooth1, tooth2, tooth3, DARKGRAY);
        }
        DrawCircleV(center, r * 0.3f, GRAY);
        break;

    case SpikeType::CHAIN:
    {
        // Rope + hook: script.user holds the context
        struct ChainContext
        {
            b2BodyId hookBody;
            float halfW;
            float halfH;
        };
        auto *ctx = (e.script.user != nullptr) ? static_cast<ChainContext *>(e.script.user) : nullptr;
        if (ctx)
        {
            // Draw rope as a line from spike bottom to hook top
            float spikeHalfM = (e.transform.extent.y / unitsPerMeter);

            b2RevoluteJointDef jointDef = b2DefaultRevoluteJointDef();

            // Vector2 anchorBot = {center.x, center.y + e.transform.extent.y};
            // b2Vec2 hp = b2Body_GetPosition(ctx->hookBody);
            // b2Rot hr = b2Body_GetRotation(ctx->hookBody);
            // float ha = b2Rot_GetAngle(hr);
            // Vector2 hc = {hp.x * unitsPerMeter, hp.y * unitsPerMeter};
            // Vector2 hookTopOffset = {0.0f, -ctx->halfH * e.spikeProps.hookScaleH};
            // // rotate offset by hook angle
            // float ca = cosf(ha), sa = sinf(ha);
            // Vector2 hookTop = {hc.x + hookTopOffset.x * ca - hookTopOffset.y * sa, hc.y + hookTopOffset.x * sa + hookTopOffset.y * ca};
            // DrawLineEx(anchorBot, hookTop, 3.0f, DARKGRAY);

            // Draw hook rectangle
            // Vector2 hsize = {2.0f * ctx->halfW * e.spikeProps.hookScaleW, 2.0f * ctx->halfH * e.spikeProps.hookScaleH};
            // DrawRectanglePro((Rectangle){hc.x, hc.y, hsize.x, hsize.y}, (Vector2){hsize.x * 0.5f, hsize.y * 0.5f}, RAD2DEG * ha, e.visual.color);
        }
        else
        {
            // Fallback simple render
            if (e.spikeProps.chainLength > 0.0f)
            {
                Vector2 chainTop = {center.x, center.y - e.spikeProps.chainLength};
                DrawLineEx(chainTop, center, 3.0f, DARKGRAY);
            }
            DrawCircleV(center, r, e.visual.color);
        }
        break;
    }
    }
}

inline GameEntity makeEntity(
    EntityManager &em,
    b2WorldId world,
    const Texture &texture,
    const b2Polygon &polygon,
    const b2Vec2 &extentPx,
    const b2Vec2 &posMeters,
    b2BodyType bodyType,
    const PhysicsMaterial &physicsMat = PhysicsMaterial{})
{

    b2BodyDef def = b2DefaultBodyDef();
    def.type = bodyType;
    def.position = posMeters;
    def.linearDamping = physicsMat.linearDamping;
    def.angularDamping = physicsMat.angularDamping;
    def.gravityScale = physicsMat.affectedByGravity ? 1.0f : 0.0f;

    GameEntity e{};
    e.id = em.create();
    e.body.id = b2CreateBody(world, &def);
    e.sprite.texture = texture;
    e.transform.extent = extentPx;
    e.script.update = &DefaultUpdate;
    e.script.render = &DefaultRender;
    e.physics = physicsMat;

    b2ShapeDef sdef = b2DefaultShapeDef();
    sdef.density = physicsMat.density;
    sdef.material.friction = physicsMat.friction;
    sdef.material.restitution = physicsMat.restitution;
    b2CreatePolygonShape(e.body.id, &sdef, &polygon);

    // Apply gravity scale to the body (0 = no gravity, 1 = normal gravity)
    b2Body_SetGravityScale(e.body.id, physicsMat.affectedByGravity ? 1.0f : 0.0f);

    return e;
}

inline GameEntity makeGroundEntity(
    EntityManager &em,
    b2WorldId world,
    const Texture &texture,
    const b2Polygon &polygon,
    const b2Vec2 &extentPx,
    const b2Vec2 &posMeters)
{
    return makeEntity(em, world, texture, polygon, extentPx, posMeters, b2_staticBody);
}

inline GameEntity makeBoxEntity(
    EntityManager &em,
    b2WorldId world,
    const Texture &texture,
    const b2Polygon &polygon,
    const b2Vec2 &extentPx,
    const b2Vec2 &posMeters,
    bool dynamic = true,
    const PhysicsMaterial &physicsMat = PhysicsMaterial{},
    const VisualStyle &visualStyle = VisualStyle{})
{
    auto e = makeEntity(em, world, texture, polygon, extentPx, posMeters, dynamic ? b2_dynamicBody : b2_staticBody, physicsMat);
    e.visual = visualStyle;
    return e;
}

// Variable-sized static obstacle (box). Uses custom render.
inline GameEntity makeObstacleEntity(
    EntityManager &em,
    b2WorldId world,
    float unitsPerMeter,
    const b2Vec2 &extentPx,
    const b2Vec2 &posMeters,
    const Texture &texture,
    const VisualStyle &visualStyle = VisualStyle{{DARKGRAY}, 0.0f, false})
{
    b2BodyDef def = b2DefaultBodyDef();
    def.type = b2_staticBody;
    def.position = posMeters;
    GameEntity e{};
    e.id = em.create();
    e.body.id = b2CreateBody(world, &def);
    e.transform.extent = extentPx;
    e.sprite.texture = texture;
    e.visual = visualStyle;
    // physics shape sized to extent
    b2Polygon poly = b2MakeBox(extentPx.x / unitsPerMeter, extentPx.y / unitsPerMeter);
    b2ShapeDef sdef = b2DefaultShapeDef();
    b2CreatePolygonShape(e.body.id, &sdef, &poly);
    e.script.update = &DefaultUpdate;
    e.script.render = &ObstacleRender;
    return e;
}

// Spike hazard with customizable type and visual
inline GameEntity makeSpikeEntity(
    EntityManager &em,
    b2WorldId world,
    float unitsPerMeter,
    float radiusPx,
    const b2Vec2 &posMeters,
    const Texture &texture,
    const SpikeProperties &spikeProps = SpikeProperties{},
    const VisualStyle &visualStyle = VisualStyle{{RED}, 0.0f, false})
{
    b2BodyDef def = b2DefaultBodyDef();
    def.type = b2_dynamicBody; // Dynamic so joints work
    def.position = posMeters;
    def.gravityScale = 0.0f; // Zero gravity to keep spike stationary
    GameEntity e{};
    e.id = em.create();
    e.body.id = b2CreateBody(world, &def);
    e.transform.extent = {radiusPx, radiusPx};
    e.sprite.texture = texture;
    e.visual = visualStyle;
    e.spikeProps = spikeProps;
    // Create heavy static-like shape so spike doesn't move
    b2Polygon poly = b2MakeBox(radiusPx / unitsPerMeter, radiusPx / unitsPerMeter);
    b2ShapeDef sdef = b2DefaultShapeDef();
    sdef.density = 10000.0f; // Very heavy to resist movement
    sdef.material.friction = 1.0f;
    // If this is a chain spike and self-collide is disabled, keep anchor out of chain collisions
    if (spikeProps.type == SpikeType::CHAIN && !spikeProps.chainSelfCollide)
    {
        sdef.filter.groupIndex = -1;
    }
    b2CreatePolygonShape(e.body.id, &sdef, &poly);

    // Add high damping to prevent any movement
    b2Body_SetLinearDamping(e.body.id, 100.0f);
    b2Body_SetAngularDamping(e.body.id, 100.0f);
    // Optional: lock anchor rotation for extra stability
    b2Body_SetFixedRotation(e.body.id, true);

    e.script.update = &SpikeUpdate;
    e.script.render = &SpikeRender;

    // If chain type, create only a rope (distance joint) and a rectangular hook
    if (spikeProps.type == SpikeType::CHAIN && spikeProps.chainLength > 0.0f)
    {
        struct ChainContext
        {
            b2BodyId hookBody;
            float halfW;
            float halfH;
        };
        auto *ctx = new ChainContext{};
        const float baseHalfW = spikeProps.linkThicknessPx * 0.5f; // reuse link dims as hook base
        const float baseHalfH = spikeProps.linkLengthPx * 0.5f;
        ctx->halfW = baseHalfW;
        ctx->halfH = baseHalfH;

        const float spikeHalfM = (radiusPx / unitsPerMeter);
        const float ropeLenM = (spikeProps.chainLength / unitsPerMeter);
        const b2Vec2 anchorPos = posMeters;

        // Create the hook body at rope length below the spike bottom
        b2BodyDef hdef = b2DefaultBodyDef();
        hdef.type = b2_dynamicBody;
        hdef.position = {anchorPos.x, anchorPos.y + spikeHalfM + ropeLenM};
        hdef.linearDamping = 0.6f;
        hdef.angularDamping = 0.8f;
        ctx->hookBody = b2CreateBody(world, &hdef);

        // Hook rectangle shape
        b2Polygon hpoly = b2MakeBox((ctx->halfW * spikeProps.hookScaleW) / unitsPerMeter,
                                    (ctx->halfH * spikeProps.hookScaleH) / unitsPerMeter);
        b2ShapeDef hsdef = b2DefaultShapeDef();
        hsdef.density = std::max(spikeProps.linkDensity * 1.5f, 1.0f);
        hsdef.material.friction = spikeProps.linkFriction;
        hsdef.material.restitution = spikeProps.linkRestitution;
        if (!spikeProps.chainSelfCollide)
            hsdef.filter.groupIndex = -1; // keep hook from colliding with spike
        b2CreatePolygonShape(ctx->hookBody, &hsdef, &hpoly);

        // Rope via distance joint, configured like the debug rope: center anchors,
        // no spring, and a maxLength clamp (behaves like a rope with slack)
        b2DistanceJointDef jdef = b2DefaultDistanceJointDef();
        jdef.bodyIdA = e.body.id;
        jdef.bodyIdB = ctx->hookBody;
        jdef.localAnchorA = {0.0f, 0.0f};
        jdef.localAnchorB = {0.0f, 0.0f};
        jdef.length = ropeLenM;    // nominal
        jdef.minLength = 0.0f;     // allow slack
        jdef.maxLength = ropeLenM; // rope limit
        jdef.enableSpring = false; // pure rope behavior
        jdef.hertz = 0.0f;         // ignored when spring disabled
        jdef.dampingRatio = 0.0f;  // ignored when spring disabled
        b2CreateDistanceJoint(world, &jdef);

        e.script.user = ctx;
        e.script.freeFn = [](void *p)
        { delete static_cast<ChainContext *>(p); };
    }
    return e;
}

// Thrower: player-controlled launcher. Aims toward mouse, charges power by hold duration.
struct ThrowerContext
{
    EntityManager *em{};
    b2WorldId world{};
    const Texture *boxTexture{};
    b2Polygon boxPolygon{};
    b2Vec2 boxExtentPx{};
    float maxPower{300.0f};
    float chargeRate{150.0f}; // power increase per second
    float currentCharge{0.0f};
    bool isCharging{false};
    Vector2 aimDir{1.0f, 0.0f};             // normalized aim direction
    std::vector<GameEntity> *projectiles{}; // destination container
    float unitsPerMeter{50.0f};
    float impulseMultiplier{8.0f};
};

inline void FreeThrowerCtx(void *p)
{
    delete static_cast<ThrowerContext *>(p);
}

inline void ThrowerUpdate(GameEntity &e, float dt)
{
    auto *ctx = static_cast<ThrowerContext *>(e.script.user);
    if (!ctx)
        return;

    // Update charge if charging
    if (ctx->isCharging)
    {
        ctx->currentCharge += ctx->chargeRate * dt;
        if (ctx->currentCharge > ctx->maxPower)
            ctx->currentCharge = ctx->maxPower;
    }
}

inline void ThrowerRender(const GameEntity &e, float unitsPerMeter)
{
    auto *ctx = static_cast<ThrowerContext *>(e.script.user);
    DrawSolidBox(e, unitsPerMeter, ORANGE);

    if (ctx && ctx->isCharging)
    {
        // Draw aim line from thrower to mouse direction
        b2Vec2 pos = b2Body_GetPosition(e.body.id);
        Vector2 throwerScreen = {pos.x * unitsPerMeter, pos.y * unitsPerMeter};
        Vector2 aimEnd = {
            throwerScreen.x + ctx->aimDir.x * 200.0f,
            throwerScreen.y + ctx->aimDir.y * 200.0f};
        DrawLineEx(throwerScreen, aimEnd, 3.0f, YELLOW);

        // Draw power indicator
        float chargeRatio = ctx->currentCharge / ctx->maxPower;
        Color powerColor = chargeRatio < 0.5f ? YELLOW : (chargeRatio < 0.8f ? ORANGE : RED);
        DrawCircle(throwerScreen.x, throwerScreen.y, 10.0f + chargeRatio * 15.0f, powerColor);
    }
}

inline GameEntity makeThrowerEntity(
    EntityManager &em,
    b2WorldId world,
    float unitsPerMeter,
    const b2Vec2 &extentPx,
    const b2Vec2 &posMeters,
    float maxPower,
    float impulseMultiplier,
    const Texture &boxTexture,
    const b2Polygon &boxPolygon,
    const b2Vec2 &boxExtentPx,
    std::vector<GameEntity> &projectiles)
{
    b2BodyDef def = b2DefaultBodyDef();
    def.type = b2_staticBody;
    def.position = posMeters;
    GameEntity e{};
    e.id = em.create();
    e.body.id = b2CreateBody(world, &def);
    e.transform.extent = extentPx;
    // Make thrower a sensor (non-colliding) so it doesn't block projectiles
    b2Polygon poly = b2MakeBox(extentPx.x / unitsPerMeter, extentPx.y / unitsPerMeter);
    b2ShapeDef sdef = b2DefaultShapeDef();
    sdef.isSensor = true;
    b2CreatePolygonShape(e.body.id, &sdef, &poly);
    // script hooks
    auto *ctx = new ThrowerContext{&em, world, &boxTexture, boxPolygon, boxExtentPx, maxPower, 150.0f, 0.0f, false, {1.0f, 0.0f}, &projectiles, unitsPerMeter, impulseMultiplier};
    e.script.user = ctx;
    e.script.freeFn = &FreeThrowerCtx;
    e.script.update = &ThrowerUpdate;
    e.script.render = &ThrowerRender;
    return e;
}
