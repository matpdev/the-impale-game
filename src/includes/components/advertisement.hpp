#pragma once

#include "raylib.h"
#include <string>
#include <chrono>

enum class AdType
{
    STATIC_IMAGE, // Imagem estática
    ANIMATED_GIF, // GIF animado (sequência de frames)
    VIDEO,        // Vídeo (futuro)
    INTERACTIVE   // Clicável com ação
};

enum class AdSource
{
    LOCAL,  // Arquivo local
    REMOTE, // URL remota (HTTP/HTTPS)
    CACHED  // Cache de arquivo remoto
};

enum class AdPlacementMode
{
    FIXED_SCREEN,       // Fixo na tela (UI overlay)
    WORLD_SPACE,        // Posicionado no mundo do jogo (parallax)
    PARALLAX_BACKGROUND // Background com parallax scrolling
};

struct Advertisement
{
    // Identificação
    std::string id;      // ID único do anúncio
    std::string name;    // Nome/descrição
    std::string sponsor; // Nome do patrocinador

    // Tipo e fonte
    AdType type = AdType::STATIC_IMAGE;
    AdSource source = AdSource::LOCAL;
    std::string assetPath;  // Caminho local ou URL
    std::string cachedPath; // Caminho do cache (se remoto)

    // Visual
    Texture2D texture = {0};         // Textura carregada
    Rectangle bounds = {0, 0, 0, 0}; // Posição e tamanho na tela
    float rotation = 0.0f;           // Rotação em graus
    Color tint = WHITE;              // Cor de tinta
    float opacity = 1.0f;            // Opacidade (0.0 - 1.0)

    // Posicionamento
    AdPlacementMode placementMode = AdPlacementMode::FIXED_SCREEN;
    Vector2 worldPosition = {0, 0}; // Posição no mundo (para WORLD_SPACE/PARALLAX)
    float parallaxFactor = 1.0f;    // Fator de parallax (0.0 = estático, 1.0 = velocidade normal)
    float worldSpacing = 200.0f;    // Espaçamento entre anúncios no mundo (pixels)
    int repeatCount = 1;            // Quantas vezes o anúncio aparece por ciclo de wrap (rotação da câmera)
    int maxVisible = 1;             // NOVO: Máximo de anúncios visíveis na tela simultaneamente

    // Timing
    float displayDuration = 5.0f; // Duração em segundos
    float currentTime = 0.0f;     // Tempo atual de exibição
    bool active = false;          // Se está visível
    bool loop = true;             // Se deve repetir

    // Interatividade
    bool clickable = false;             // Se pode ser clicado
    std::string clickUrl;               // URL ao clicar
    Rectangle clickArea = {0, 0, 0, 0}; // Área clicável

    // Métricas (para logging)
    int impressions = 0;                              // Número de vezes exibido
    int clicks = 0;                                   // Número de cliques
    std::chrono::system_clock::time_point firstShown; // Primeira exibição
    std::chrono::system_clock::time_point lastShown;  // Última exibição

    // Animação (para GIFs)
    int frameCount = 1;          // Número de frames
    int currentFrame = 0;        // Frame atual
    float frameTime = 0.1f;      // Tempo entre frames
    float frameTimer = 0.0f;     // Timer do frame atual
    Texture2D *frames = nullptr; // Array de frames (se animado)

    // Estado de carregamento
    bool loaded = false;     // Se o asset foi carregado
    bool loadFailed = false; // Se falhou ao carregar
    std::string loadError;   // Mensagem de erro
};
