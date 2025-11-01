#pragma once
#include <string>
#include <vector>
#include "raylib.h"
#include "box2d/box2d.h"

#include "../entities/types.hpp"
#include "../core/entity_manager.hpp"

// Level loader from TOML configuration
// Schema (example):
// [thrower]
// x = 600
// y = 800
// interval = 1.2
// power = 180.0
// dir = [1.0, -0.4]
//
// [[obstacles]]
// x = 300
// y = 950
// w = 300
// h = 40
//
// [[spikes]]
// x = 1200
// y = 860
// r = 24

namespace level
{
    struct BuildContext
    {
        EntityManager &em;
        b2WorldId world;
        float unitsPerMeter{50.0f};
        const Texture &groundTexture;
        const Texture &boxTexture;
        const b2Polygon &groundPolygon;
        const b2Polygon &boxPolygon;
        const b2Vec2 &groundExtentPx;
        const b2Vec2 &boxExtentPx;
        std::vector<GameEntity> &grounds;
        std::vector<GameEntity> &boxes;
        std::vector<GameEntity> &obstacles;
        std::vector<GameEntity> &spikes;
        std::vector<GameEntity> &throwers;
    };

    // Loads scenario and populates entity vectors. Returns true on success.
    bool LoadScenarioFromToml(const std::string &path, BuildContext &ctx);
}
