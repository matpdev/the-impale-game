#include "../includes/core/world_loader.hpp"

#include <stdexcept>
#include <cstdio>

#include "toml.hpp" // toml11

#include "../includes/entities/factory.hpp"
#include "../includes/components/visual_style.hpp"
#include "../includes/components/physics_material.hpp"
#include "../includes/components/spike_properties.hpp"

namespace level
{
    static b2Vec2 toMeters(float x, float y, float uom)
    {
        return {x / uom, y / uom};
    }

    // Helper to read numeric value as float (handles both int and float TOML types)
    static float getFloat(const toml::value &v, const char *key)
    {
        const auto &val = toml::find(v, key);
        if (val.is_floating())
            return toml::get<double>(val);
        else if (val.is_integer())
            return static_cast<float>(toml::get<std::int64_t>(val));
        else
            throw toml::type_error("Expected numeric type", val.location());
    }

    // Optional float with default
    static float getFloatOr(const toml::value &v, const char *key, float defaultVal)
    {
        if (!v.contains(key))
            return defaultVal;
        return getFloat(v, key);
    }

    // Parse Color from RGB array [r, g, b] (0-255)
    static Color parseColor(const toml::value &v, const char *key, Color defaultColor)
    {
        if (!v.contains(key))
            return defaultColor;

        const auto &arr = toml::find<toml::array>(v, key);
        if (arr.size() < 3)
            return defaultColor;

        int r = arr[0].is_integer() ? toml::get<int>(arr[0]) : static_cast<int>(toml::get<double>(arr[0]));
        int g = arr[1].is_integer() ? toml::get<int>(arr[1]) : static_cast<int>(toml::get<double>(arr[1]));
        int b = arr[2].is_integer() ? toml::get<int>(arr[2]) : static_cast<int>(toml::get<double>(arr[2]));
        int a = arr.size() > 3 ? (arr[3].is_integer() ? toml::get<int>(arr[3]) : static_cast<int>(toml::get<double>(arr[3]))) : 255;

        return Color{static_cast<unsigned char>(r), static_cast<unsigned char>(g),
                     static_cast<unsigned char>(b), static_cast<unsigned char>(a)};
    }

    // Parse PhysicsMaterial from TOML
    static PhysicsMaterial parsePhysicsMaterial(const toml::value &v)
    {
        PhysicsMaterial mat;
        mat.density = getFloatOr(v, "density", 1.0f);
        mat.friction = getFloatOr(v, "friction", 0.3f);
        mat.restitution = getFloatOr(v, "restitution", 0.2f);
        mat.linearDamping = getFloatOr(v, "linearDamping", 0.0f);
        mat.angularDamping = getFloatOr(v, "angularDamping", 0.05f);
        mat.affectedByGravity = v.contains("gravity") ? toml::find<bool>(v, "gravity") : true;
        return mat;
    }

    // Parse VisualStyle from TOML
    static VisualStyle parseVisualStyle(const toml::value &v, Color defaultColor = WHITE)
    {
        VisualStyle style;
        style.color = parseColor(v, "color", defaultColor);
        style.roundness = getFloatOr(v, "roundness", 0.0f);
        style.useTexture = v.contains("useTexture") ? toml::find<bool>(v, "useTexture") : true;
        return style;
    }

    // Parse SpikeType from string
    static SpikeType parseSpikeType(const std::string &typeStr)
    {
        if (typeStr == "saw")
            return SpikeType::SAW;
        else if (typeStr == "chain")
            return SpikeType::CHAIN;
        else
            return SpikeType::NORMAL;
    }

    // Parse SpikeProperties from TOML
    static SpikeProperties parseSpikeProperties(const toml::value &v)
    {
        SpikeProperties props;
        if (v.contains("type"))
        {
            std::string typeStr = toml::find<std::string>(v, "type");
            props.type = parseSpikeType(typeStr);
        }
        props.rotationSpeed = getFloatOr(v, "rotationSpeed", 90.0f); // default 90 deg/sec for saws
        props.chainLength = getFloatOr(v, "chainLength", 50.0f);
        // Optional chain-specific tuning
        props.linkLengthPx = getFloatOr(v, "linkLengthPx", props.linkLengthPx);
        props.linkThicknessPx = getFloatOr(v, "linkThicknessPx", props.linkThicknessPx);
        props.linkDensity = getFloatOr(v, "linkDensity", props.linkDensity);
        props.linkFriction = getFloatOr(v, "linkFriction", props.linkFriction);
        props.linkRestitution = getFloatOr(v, "linkRestitution", props.linkRestitution);
        props.hookScaleW = getFloatOr(v, "hookScaleW", props.hookScaleW);
        props.hookScaleH = getFloatOr(v, "hookScaleH", props.hookScaleH);
        props.jointHertz = getFloatOr(v, "jointHertz", props.jointHertz);
        props.jointDamping = getFloatOr(v, "jointDamping", props.jointDamping);
        if (v.contains("chainSelfCollide"))
            props.chainSelfCollide = toml::find<bool>(v, "chainSelfCollide");
        return props;
    }

    // Parse texture path from TOML with default fallback
    static std::string parseTexturePath(const toml::value &v, const char *key, const std::string &defaultPath)
    {
        if (!v.contains(key))
            return defaultPath;
        return toml::find<std::string>(v, key);
    }

    bool LoadScenarioFromToml(const std::string &path, BuildContext &ctx)
    {
        toml::value data;
        try
        {
            data = toml::parse(path);
        }
        catch (const std::exception &e)
        {
            std::fprintf(stderr, "Failed to parse %s: %s\n", path.c_str(), e.what());
            return false;
        }

        // obstacles
        {
            toml::array arr = toml::find_or(data, "obstacles", toml::array{});
            for (auto &v : arr)
            {
                float x = getFloat(v, "x");
                float y = getFloat(v, "y");
                float w = getFloat(v, "w");
                float h = getFloat(v, "h");
                b2Vec2 posM = toMeters(x, y, ctx.unitsPerMeter);
                b2Vec2 extentPx = {0.5f * w, 0.5f * h};

                VisualStyle visual = parseVisualStyle(v, DARKGRAY);

                // Load texture for this obstacle (fallback to default)
                std::string texturePath = parseTexturePath(v, "texture", "ground.png");
                Texture obstacleTexture = ctx.textureLoader ? ctx.textureLoader(texturePath) : ctx.groundTexture;

                // Create polygon for this obstacle size
                b2Polygon obstaclePoly = b2MakeBox(extentPx.x / ctx.unitsPerMeter, extentPx.y / ctx.unitsPerMeter);

                ctx.obstacles.push_back(makeObstacleEntity(ctx.em, ctx.world, ctx.unitsPerMeter, extentPx, posM, obstacleTexture, visual));
            }
        }

        // spikes
        {
            toml::array arr = toml::find_or(data, "spikes", toml::array{});
            for (auto &v : arr)
            {
                float x = getFloat(v, "x");
                float y = getFloat(v, "y");
                float r = getFloat(v, "r");
                b2Vec2 posM = toMeters(x, y, ctx.unitsPerMeter);

                VisualStyle visual = parseVisualStyle(v, RED);
                SpikeProperties spikeProps = parseSpikeProperties(v);

                // Load texture for this spike (fallback to box texture)
                std::string texturePath = parseTexturePath(v, "texture", "box.png");
                Texture spikeTexture = ctx.textureLoader ? ctx.textureLoader(texturePath) : ctx.boxTexture;

                ctx.spikes.push_back(makeSpikeEntity(ctx.em, ctx.world, ctx.unitsPerMeter, r, posM, spikeTexture, spikeProps, visual));
            }
        }

        // thrower (single for now)
        if (data.is_table())
        {
            const auto &tbl = toml::get<toml::table>(data);
            auto it = tbl.find("thrower");
            if (it != tbl.end())
            {
                const auto &t = it->second;
                float x = getFloat(t, "x");
                float y = getFloat(t, "y");
                float power = getFloat(t, "power");
                float impulseMult = getFloatOr(t, "impulseMultiplier", 8.0f);

                b2Vec2 posM = toMeters(x, y, ctx.unitsPerMeter);
                // visual size for thrower block
                b2Vec2 extentPx = {32.0f, 32.0f};

                // Load texture for thrower (fallback to box texture)
                std::string texturePath = parseTexturePath(t, "texture", "box.png");
                Texture throwerTexture = ctx.textureLoader ? ctx.textureLoader(texturePath) : ctx.boxTexture;

                ctx.throwers.push_back(makeThrowerEntity(ctx.em, ctx.world, ctx.unitsPerMeter, extentPx, posM, power, impulseMult, throwerTexture, ctx.boxPolygon, ctx.boxExtentPx, ctx.boxes));
            }
        }

        return true;
    }
}
