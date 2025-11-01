#pragma once

#include "../components/physics_body.hpp"
#include "../components/transform.hpp"
#include "../components/sprite.hpp"
#include "../components/script.hpp"
#include "../components/impaled.hpp"
#include "../components/visual_style.hpp"
#include "../components/physics_material.hpp"
#include "../components/spike_properties.hpp"
#include "../core/entity_manager.hpp"

// Aggregate type used before introducing a full EntityManager
struct GameEntity
{
    EntityId id; // stable entity id
    PhysicsBody body;
    SpriteTransform transform;
    Sprite sprite;
    Script script;              // per-entity logic/render hooks
    Impaled impaled;            // spike collision state
    VisualStyle visual;         // color, roundness, texture usage
    PhysicsMaterial physics;    // density, friction, restitution, damping
    SpikeProperties spikeProps; // spike-specific properties (type, rotation, etc)
};
