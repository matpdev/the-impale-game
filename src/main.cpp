#include "raylib.h"
#include "box2d/box2d.h"

#include "includes/components/physics_body.hpp"
#include "includes/components/transform.hpp"
#include "includes/components/sprite.hpp"
#include "includes/components/impaled.hpp"
#include "includes/entities/types.hpp"
#include "includes/entities/factory.hpp"
#include "includes/systems/render_system.hpp"
#include "includes/systems/logic_system.hpp"
#include "includes/core/entity_manager.hpp"
#include "includes/core/world_loader.hpp"

#include <assert.h>
#include <vector>
#include <cmath>
#include <cstdio>
#include <map>
#include <string>

// Platform-specific asset path prefix
#ifdef __EMSCRIPTEN__
#define ASSET_PATH(path) ("/assets/" path)
#else
#define ASSET_PATH(path) (path)
#endif

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

    Texture groundTexture = textureCache.load(ASSET_PATH("ground.png"));
    Texture boxTexture = textureCache.load(ASSET_PATH("block.png"));

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
                            {
#ifdef __EMSCRIPTEN__
                                // If path doesn't start with /assets/, prepend it
                                std::string fullPath = path;
                                if (fullPath.find("/assets/") != 0)
                                {
                                    fullPath = "/assets/" + path;
                                }
                                return textureCache.load(fullPath);
#else
                                return textureCache.load(path);
#endif
                            }};
    level::LoadScenarioFromToml(ASSET_PATH("levels/demo.toml"), ctx);

    bool pause = false;
    bool showDebugWireframe = true; // toggle with 'D' key

    // Debug rope toggle state
    DebugRope debugRope;

    // Create logic context
    LogicContext logicCtx{
        worldId, lengthUnitsPerMeter,
        boxEntities, obstacleEntities, spikeEntities, throwerEntities,
        entityManager,
        boxTexture, boxPolygon, boxExtent,
        pause};

    // Create render context
    RenderContext renderCtx{
        width, height, lengthUnitsPerMeter,
        boxEntities, obstacleEntities, spikeEntities, throwerEntities,
        showDebugWireframe, &debugRope};

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_P))
        {
            pause = !pause;
            logicCtx.isPaused = pause;
        }

        if (IsKeyPressed(KEY_D))
        {
            showDebugWireframe = !showDebugWireframe;
            renderCtx.showDebugWireframe = showDebugWireframe;
        }

        // Update game logic
        UpdateLogic(logicCtx, GetFrameTime());

        // Render frame
        RenderFrame(renderCtx);
    }

    textureCache.unloadAll();
    CloseWindow();

    return 0;
}