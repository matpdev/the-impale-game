#include "raylib.h"
#include "box2d/box2d.h"

#include <assert.h>
#include <vector>
#include <cmath>
#include <cstdio>

typedef struct Entity
{
    b2BodyId bodyId;
    b2Vec2 extent;
    Texture texture;
} Entity;

void DrawEntity(const Entity &entity, float lengthUnitsPerMeter)
{

    b2Vec2 extentMeters = {entity.extent.x / lengthUnitsPerMeter, entity.extent.y / lengthUnitsPerMeter};
    b2Vec2 p = b2Body_GetWorldPoint(entity.bodyId, (b2Vec2){-extentMeters.x, -extentMeters.y});
    b2Rot rotation = b2Body_GetRotation(entity.bodyId);
    float radians = b2Rot_GetAngle(rotation);

    Vector2 ps = {p.x * lengthUnitsPerMeter, p.y * lengthUnitsPerMeter};
    DrawTextureEx(entity.texture, ps, RAD2DEG * radians, 1.0f, WHITE);
}

#define GROUND_COUNT 14
#define BOX_COUNT 10

int main(void)
{
    int width = 1920, height = 1080;
    InitWindow(width, height, "box2d-raylib");

    SetTargetFPS(60);

    float lengthUnitsPerMeter = 50.0f;
    b2SetLengthUnitsPerMeter(lengthUnitsPerMeter);

    b2WorldDef worldDef = b2DefaultWorldDef();

    worldDef.gravity.y = 3.8f * lengthUnitsPerMeter;
    b2WorldId worldId = b2CreateWorld(&worldDef);

    Texture groundTexture = LoadTexture("ground.png");
    Texture boxTexture = LoadTexture("box.png");

    b2Vec2 groundExtent = {0.5f * groundTexture.width, 0.5f * groundTexture.height};
    b2Vec2 boxExtent = {0.5f * boxTexture.width, 0.5f * boxTexture.height};

    b2Polygon groundPolygon = b2MakeBox(groundExtent.x / lengthUnitsPerMeter, groundExtent.y / lengthUnitsPerMeter);
    b2Polygon boxPolygon = b2MakeBox(boxExtent.x / lengthUnitsPerMeter, boxExtent.y / lengthUnitsPerMeter);

    std::vector<Entity> groundEntities;
    groundEntities.reserve(GROUND_COUNT);
    for (int i = 0; i < GROUND_COUNT; ++i)
    {
        Entity entity;
        b2BodyDef bodyDef = b2DefaultBodyDef();

        bodyDef.position = (b2Vec2){(2.0f * i + 2.0f) * groundExtent.x / lengthUnitsPerMeter, (height - groundExtent.y - 100.0f) / lengthUnitsPerMeter};

        entity.bodyId = b2CreateBody(worldId, &bodyDef);
        entity.extent = groundExtent;
        entity.texture = groundTexture;
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        b2CreatePolygonShape(entity.bodyId, &shapeDef, &groundPolygon);
        groundEntities.push_back(entity);
    }

    std::vector<Entity> boxEntities;
    boxEntities.reserve(BOX_COUNT);
    for (int i = 0; i < 4; ++i)
    {
        float y = height - groundExtent.y - 100.0f - (2.5f * i + 2.0f) * boxExtent.y - 20.0f;

        for (int j = i; j < 4; ++j)
        {
            float x = 0.5f * width + (3.0f * j - i - 3.0f) * boxExtent.x;
            assert(boxEntities.size() < BOX_COUNT);

            Entity entity;
            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = b2_dynamicBody;

            bodyDef.position = (b2Vec2){x / lengthUnitsPerMeter, y / lengthUnitsPerMeter};
            entity.bodyId = b2CreateBody(worldId, &bodyDef);
            entity.texture = boxTexture;
            entity.extent = boxExtent;
            b2ShapeDef shapeDef = b2DefaultShapeDef();
            b2CreatePolygonShape(entity.bodyId, &shapeDef, &boxPolygon);
            boxEntities.push_back(entity);
        }
    }

    bool pause = false;
    bool isDragging = false;
    Vector2 mousePosStart = {0.0f, 0.0f};
    Vector2 mousePosCurrent = {0.0f, 0.0f};

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_P))
        {
            pause = !pause;
        }

        if (pause == false)
        {
            float deltaTime = GetFrameTime();
            b2World_Step(worldId, deltaTime, 4);
        }

        Vector2 mouseScreen = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            mousePosStart = mouseScreen;
            isDragging = true;
        }

        if (isDragging)
        {
            mousePosCurrent = mouseScreen;

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            {
                isDragging = false;

                float startX = mousePosStart.x / lengthUnitsPerMeter;
                float startY = mousePosStart.y / lengthUnitsPerMeter;
                float endX = mousePosCurrent.x / lengthUnitsPerMeter;
                float endY = mousePosCurrent.y / lengthUnitsPerMeter;

                printf("Mouse start: (%.2f, %.2f) pixels -> (%.2f, %.2f) meters\n",
                       mousePosStart.x, mousePosStart.y, startX, startY);
                printf("Mouse end: (%.2f, %.2f) pixels -> (%.2f, %.2f) meters\n",
                       mousePosCurrent.x, mousePosCurrent.y, endX, endY);

                b2BodyDef bodyDef = b2DefaultBodyDef();
                bodyDef.type = b2_dynamicBody;
                bodyDef.position = (b2Vec2){startX, startY};
                b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

                Entity entity;
                entity.bodyId = bodyId;
                entity.texture = boxTexture;
                entity.extent = boxExtent;
                b2ShapeDef shapeDef = b2DefaultShapeDef();
                b2CreatePolygonShape(entity.bodyId, &shapeDef, &boxPolygon);

                float dx = endX - startX;
                float dy = endY - startY;
                float distance = sqrtf(dx * dx + dy * dy);

                if (distance > 0.001f)
                {

                    float dirX = dx / distance;
                    float dirY = dy / distance;

                    float forceStrength = distance * 50.0f;

                    float forceX = -dirX * forceStrength;
                    float forceY = -dirY * forceStrength;

                    b2Vec2 impulse = {forceX, forceY};
                    b2Vec2 point = {startX, startY};
                    b2Body_ApplyLinearImpulse(bodyId, impulse, point, true);

                    printf("Forca aplicada: (%.2f, %.2f), distancia: %.2f\n", forceX, forceY, distance);
                }

                boxEntities.push_back(entity);
            }
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        const char *message = "Hello Box2D!";
        int fontSize = 36;
        int textWidth = MeasureText("Hello Box2D!", fontSize);
        DrawText(message, (width - textWidth) / 2, 50, fontSize, LIGHTGRAY);

        for (size_t i = 0; i < groundEntities.size(); ++i)
        {
            DrawEntity(groundEntities[i], lengthUnitsPerMeter);
        }

        for (size_t i = 0; i < boxEntities.size(); ++i)
        {
            DrawEntity(boxEntities[i], lengthUnitsPerMeter);
        }

        if (isDragging)
        {

            float dx = mousePosCurrent.x - mousePosStart.x;
            float dy = mousePosCurrent.y - mousePosStart.y;
            float distance = sqrtf(dx * dx + dy * dy);

            DrawLineV(mousePosStart, mousePosCurrent, YELLOW);

            if (distance > 10.0f)
            {
                float arrowLength = 50.0f;
                float dirX = -dx / distance;
                float dirY = -dy / distance;

                Vector2 arrowStart = mousePosStart;
                Vector2 arrowEnd = {
                    mousePosStart.x + dirX * arrowLength,
                    mousePosStart.y + dirY * arrowLength};

                DrawLineV(arrowStart, arrowEnd, GREEN);

                float arrowAngle = atan2f(dirY, dirX);
                Vector2 arrowTip1 = {
                    arrowEnd.x - cosf(arrowAngle - 0.5f) * 10.0f,
                    arrowEnd.y - sinf(arrowAngle - 0.5f) * 10.0f};
                Vector2 arrowTip2 = {
                    arrowEnd.x - cosf(arrowAngle + 0.5f) * 10.0f,
                    arrowEnd.y - sinf(arrowAngle + 0.5f) * 10.0f};
                DrawLineV(arrowEnd, arrowTip1, GREEN);
                DrawLineV(arrowEnd, arrowTip2, GREEN);
            }

            DrawCircleV(mousePosStart, 10.0f, RED);

            DrawCircleV(mousePosCurrent, 10.0f, YELLOW);

            char forceText[50];
            snprintf(forceText, sizeof(forceText), "Distancia: %.0f px", distance);
            DrawText(forceText, (int)mousePosStart.x + 20, (int)mousePosStart.y - 30, 20, WHITE);
        }

        EndDrawing();
    }

    UnloadTexture(groundTexture);
    UnloadTexture(boxTexture);

    CloseWindow();

    return 0;
}