#pragma once

// Forward declaration to avoid circular include between Script and GameEntity
struct GameEntity;

// Script component: function hooks for per-entity logic and rendering
// - update runs outside the render phase
// - render runs inside the render phase
struct Script
{
    using UpdateFn = void (*)(GameEntity &e, float dt);
    using RenderFn = void (*)(const GameEntity &e, float unitsPerMeter);
    using FreeFn = void (*)(void *);

    UpdateFn update{nullptr};
    RenderFn render{nullptr};
    void *user{nullptr};    // optional user context
    FreeFn freeFn{nullptr}; // optional deleter for user context
};
