/**
 * Exemplo de Uso do Sistema de Anúncios
 *
 * Este arquivo demonstra como integrar o sistema de anúncios
 * em um jogo raylib simples.
 *
 * Compile com:
 * g++ advertisement_example.cpp -lraylib -std=c++17 -o ad_demo
 */

#include "raylib.h"
#include "../src/includes/systems/advertisement_system.hpp"
#include <vector>
#include <string>

// Estado do exemplo
enum class GameState
{
    LOADING,
    MENU,
    PLAYING,
    PAUSED
};

int main()
{
    // Configuração da janela
    const int screenWidth = 800;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Advertisement System Demo");
    SetTargetFPS(60);

    // Sistema de anúncios
    AdvertisementSystem adSystem;

    // Carrega configuração
    if (!adSystem.LoadFromTOML("assets/ads/ads_config.toml"))
    {
        TraceLog(LOG_ERROR, "Failed to load ads config!");
    }

    // Estado do jogo
    GameState state = GameState::LOADING;
    float loadingTime = 0.0f;
    const float loadingDuration = 3.0f;

    // Anúncios ativos por estado
    std::vector<std::string> menuAds = {"banner_top_001", "banner_side_002"};
    std::vector<std::string> gameplayAds = {"ingame_object_004"};

    // Rotação de anúncios
    float rotationTimer = 0.0f;
    const float rotationInterval = 10.0f;
    int currentRotationIndex = 0;
    std::vector<std::string> rotatingAds = {
        "rotation_slot1_a",
        "rotation_slot1_b",
        "rotation_slot1_c"};

    // ========== GAME LOOP ==========
    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        // ========== UPDATE ==========
        switch (state)
        {
        case GameState::LOADING:
        {
            // Mostra anúncio de loading
            if (loadingTime == 0.0f)
            {
                adSystem.ActivateAd("loading_screen_005");
            }

            loadingTime += deltaTime;

            if (loadingTime >= loadingDuration)
            {
                adSystem.DeactivateAd("loading_screen_005");
                state = GameState::MENU;

                // Ativa anúncios do menu
                for (const auto &adId : menuAds)
                {
                    adSystem.ActivateAd(adId);
                }

                // Inicia rotação
                adSystem.ActivateAd(rotatingAds[currentRotationIndex]);
            }
            break;
        }

        case GameState::MENU:
        {
            // Sistema de rotação
            rotationTimer += deltaTime;
            if (rotationTimer >= rotationInterval)
            {
                adSystem.DeactivateAd(rotatingAds[currentRotationIndex]);
                currentRotationIndex = (currentRotationIndex + 1) % rotatingAds.size();
                adSystem.ActivateAd(rotatingAds[currentRotationIndex]);
                rotationTimer = 0.0f;
            }

            // Verifica clique em "PLAY"
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                Vector2 mousePos = GetMousePosition();
                Rectangle playButton = {300, 250, 200, 50};

                if (CheckCollisionPointRec(mousePos, playButton))
                {
                    // Desativa anúncios do menu
                    for (const auto &adId : menuAds)
                    {
                        adSystem.DeactivateAd(adId);
                    }
                    adSystem.DeactivateAd(rotatingAds[currentRotationIndex]);

                    // Ativa anúncios do gameplay
                    for (const auto &adId : gameplayAds)
                    {
                        adSystem.ActivateAd(adId);
                    }

                    state = GameState::PLAYING;
                }
            }
            break;
        }

        case GameState::PLAYING:
        {
            // Verifica ESC para pausar
            if (IsKeyPressed(KEY_ESCAPE))
            {
                state = GameState::PAUSED;
            }

            // Simula gameplay...
            break;
        }

        case GameState::PAUSED:
        {
            if (IsKeyPressed(KEY_ESCAPE))
            {
                state = GameState::PLAYING;
            }
            break;
        }
        }

        // Atualiza sistema de anúncios
        adSystem.Update(deltaTime);

        // Verifica cliques em anúncios (todas as telas)
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Vector2 mousePos = GetMousePosition();
            if (adSystem.CheckClick(mousePos))
            {
                TraceLog(LOG_INFO, "Ad clicked!");
            }
        }

        // ========== RENDER ==========
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (state)
        {
        case GameState::LOADING:
        {
            // Tela de loading
            DrawText("LOADING...", screenWidth / 2 - 80, screenHeight / 2, 30, DARKGRAY);

            // Barra de progresso
            float progress = loadingTime / loadingDuration;
            Rectangle progressBar = {200, screenHeight / 2 + 50, 400, 30};
            DrawRectangleRec(progressBar, LIGHTGRAY);
            DrawRectangle(200, screenHeight / 2 + 50, 400 * progress, 30, GREEN);
            DrawRectangleLinesEx(progressBar, 2, DARKGRAY);

            // Anúncio de loading renderizado por cima
            adSystem.Render();
            break;
        }

        case GameState::MENU:
        {
            // Menu principal
            DrawText("IMPALE GAME", screenWidth / 2 - 150, 100, 40, DARKBLUE);

            // Botões
            Rectangle playButton = {300, 250, 200, 50};
            Rectangle optionsButton = {300, 320, 200, 50};
            Rectangle quitButton = {300, 390, 200, 50};

            DrawRectangleRec(playButton, GREEN);
            DrawRectangleRec(optionsButton, BLUE);
            DrawRectangleRec(quitButton, RED);

            DrawText("PLAY", 370, 265, 20, WHITE);
            DrawText("OPTIONS", 350, 335, 20, WHITE);
            DrawText("QUIT", 370, 405, 20, WHITE);

            // Anúncios do menu
            adSystem.Render();

            // Info de rotação
            DrawText(TextFormat("Rotating ad: %d/%d",
                                currentRotationIndex + 1, (int)rotatingAds.size()),
                     10, screenHeight - 30, 20, DARKGRAY);
            break;
        }

        case GameState::PLAYING:
        {
            // Simulação de gameplay
            DrawText("GAMEPLAY", 20, 20, 30, DARKBLUE);
            DrawText("Press ESC to pause", 20, 60, 20, GRAY);

            // Simula objeto com patrocínio
            DrawRectangle(300, 250, 200, 100, BROWN);
            DrawText("SPONSORED BOX", 320, 285, 20, WHITE);

            // Anúncios in-game
            adSystem.Render();
            break;
        }

        case GameState::PAUSED:
        {
            // Tela de pausa
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.8f));
            DrawText("PAUSED", screenWidth / 2 - 80, screenHeight / 2 - 50, 40, WHITE);
            DrawText("Press ESC to resume", screenWidth / 2 - 120, screenHeight / 2 + 20, 20, LIGHTGRAY);
            break;
        }
        }

        // Debug info
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
        DrawText("Click on ads to open URL", 10, screenHeight - 60, 16, DARKGRAY);

        EndDrawing();
    }

    // ========== CLEANUP ==========
    adSystem.Cleanup();
    CloseWindow();

    return 0;
}
