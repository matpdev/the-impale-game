#pragma once

#include "raylib.h"

// Sistema de câmera para controlar viewport e parallax
struct GameCamera
{
    Vector2 position;   // Posição da câmera no mundo
    Vector2 offset;     // Offset da câmera (geralmente centro da tela)
    float zoom;         // Zoom da câmera
    float rotation;     // Rotação da câmera
    Rectangle viewport; // Área visível da câmera

    GameCamera()
        : position({0, 0}), offset({0, 0}), zoom(1.0f), rotation(0.0f), viewport({0, 0, 1920, 1080})
    {
    }

    // Converte posição do mundo para posição na tela
    Vector2 WorldToScreen(Vector2 worldPos) const
    {
        return {
            (worldPos.x - position.x) * zoom + offset.x,
            (worldPos.y - position.y) * zoom + offset.y};
    }

    // Converte posição da tela para posição no mundo
    Vector2 ScreenToWorld(Vector2 screenPos) const
    {
        return {
            (screenPos.x - offset.x) / zoom + position.x,
            (screenPos.y - offset.y) / zoom + position.y};
    }

    // Verifica se um retângulo no mundo está visível
    bool IsRectVisible(Rectangle worldRect) const
    {
        Rectangle screenRect = {
            worldRect.x - position.x,
            worldRect.y - position.y,
            worldRect.width,
            worldRect.height};

        return CheckCollisionRecs(screenRect, viewport);
    }

    // Aplica parallax a uma posição
    // parallaxFactor: 0.0 = fixo no fundo (não move), 1.0 = move com câmera (foreground)
    Vector2 ApplyParallax(Vector2 worldPos, float parallaxFactor) const
    {
        return {
            worldPos.x + (position.x * (1.0f - parallaxFactor)),
            worldPos.y + (position.y * (1.0f - parallaxFactor))};
    }

    // Atualiza viewport
    void UpdateViewport(int screenWidth, int screenHeight)
    {
        viewport.width = screenWidth;
        viewport.height = screenHeight;
        offset.x = screenWidth / 2.0f;
        offset.y = screenHeight / 2.0f;
    }
};
