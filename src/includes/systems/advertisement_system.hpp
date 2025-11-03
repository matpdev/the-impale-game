#pragma once

#include "../components/advertisement.hpp"
#include "camera_system.hpp"
#include "raylib.h"
#include <toml.hpp>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cmath>

// Sistema de gerenciamento de anúncios
class AdvertisementSystem
{
public:
    struct Config
    {
        std::string logFile = "ads_log.txt";
        std::string cacheDir = "cache/ads";
        int maxCacheAgeDays = 7;
        float rotationInterval = 10.0f;
    };

    AdvertisementSystem() = default;
    ~AdvertisementSystem() { Cleanup(); }

    // Carrega configuração do TOML
    bool LoadFromTOML(const std::string &tomlPath);

    // Atualiza sistema (delta time)
    void Update(float deltaTime);

    // Renderiza todos os anúncios ativos
    void Render();

    // Renderiza com câmera (para anúncios em mundo/parallax)
    void RenderWithCamera(const GameCamera &camera);

    // Remove anúncios que estão longe da câmera (economiza memória)
    void CleanupOffscreenAds(const GameCamera &camera, float cleanupDistance = 2000.0f);

    // Gera anúncios em parallax ao longo de uma distância
    void GenerateParallaxAds(const std::string &templateAdId, float startX, float endX, float spacing);

    // Define a câmera para anúncios em mundo
    void SetCamera(GameCamera *camera) { camera_ = camera; } // Gerenciamento
    void ActivateAd(const std::string &id);
    void DeactivateAd(const std::string &id);
    void ToggleAd(const std::string &id);

    // Limpeza
    void Cleanup();

    // Interação
    bool CheckClick(Vector2 mousePos);

    // Logging
    void LogImpression(const Advertisement &ad);
    void LogClick(const Advertisement &ad);
    void FlushLogs();

private:
    std::vector<Advertisement> ads_;
    Config config_;
    std::ofstream logStream_;
    GameCamera *camera_ = nullptr; // Referência para a câmera do jogo

    // Helpers de carregamento
    bool LoadLocalTexture(Advertisement &ad);
    bool LoadRemoteTexture(Advertisement &ad);
    bool LoadAnimatedFrames(Advertisement &ad);

    // Cache
    std::string GetCachePath(const std::string &url);
    bool IsCacheValid(const std::string &cachePath);
    bool DownloadToCache(const std::string &url, const std::string &cachePath);

    // Parsing
    AdType ParseAdType(const std::string &typeStr);
    AdSource ParseAdSource(const std::string &sourceStr);
    AdPlacementMode ParsePlacementMode(const std::string &modeStr);

    // Logging interno
    void OpenLogFile();
    std::string GetTimestamp();
};

// Implementação inline (pode ser movida para .cpp)
inline bool AdvertisementSystem::LoadFromTOML(const std::string &tomlPath)
{
    try
    {
        auto data = toml::parse(tomlPath);

        // Carrega configurações globais
        if (data.contains("settings"))
        {
            auto settings = toml::find(data, "settings");
            config_.logFile = toml::find_or<std::string>(settings, "log_file", "ads_log.txt");
            config_.cacheDir = toml::find_or<std::string>(settings, "cache_dir", "cache/ads");
            config_.maxCacheAgeDays = toml::find_or<int>(settings, "max_cache_age_days", 7);
            config_.rotationInterval = toml::find_or<float>(settings, "rotation_interval", 10.0f);
        }

        // Abre arquivo de log
        OpenLogFile();

// Cria diretório de cache se não existir
// Nota: raylib não tem função para criar diretórios, usar std::filesystem ou system()
#ifdef _WIN32
        system(("mkdir " + config_.cacheDir).c_str());
#else
        system(("mkdir -p " + config_.cacheDir).c_str());
#endif

        // Carrega anúncios
        if (data.contains("advertisement"))
        {
            auto adsArray = toml::find<std::vector<toml::table>>(data, "advertisement");

            for (const auto &adTable : adsArray)
            {
                Advertisement ad;

                // Identificação
                ad.id = adTable.at("id").as_string();
                ad.name = adTable.at("name").as_string();
                ad.sponsor = adTable.at("sponsor").as_string();

                // Tipo e fonte
                ad.type = ParseAdType(adTable.at("type").as_string());
                ad.source = ParseAdSource(adTable.at("source").as_string());
                ad.assetPath = adTable.at("asset_path").as_string();

                // Modo de posicionamento (parse primeiro para saber como lidar com position)
                if (adTable.count("placement_mode") > 0)
                {
                    ad.placementMode = ParsePlacementMode(adTable.at("placement_mode").as_string());
                }

                // Tamanho
                auto size = adTable.at("size").as_table();
                ad.bounds.width = size.at("width").as_floating();
                ad.bounds.height = size.at("height").as_floating();

                // Posição (screen para FIXED_SCREEN, world para outros modos)
                if (ad.placementMode == AdPlacementMode::FIXED_SCREEN && adTable.count("position") > 0)
                {
                    auto pos = adTable.at("position").as_table();
                    ad.bounds.x = pos.at("x").as_floating();
                    ad.bounds.y = pos.at("y").as_floating();
                }
                else
                {
                    ad.bounds.x = 0.0f;
                    ad.bounds.y = 0.0f;
                }

                ad.rotation = (adTable.count("rotation") > 0) ? adTable.at("rotation").as_floating() : 0.0f;
                ad.opacity = (adTable.count("opacity") > 0) ? adTable.at("opacity").as_floating() : 1.0f;

                // Parallax
                if (adTable.count("parallax_factor") > 0)
                {
                    ad.parallaxFactor = adTable.at("parallax_factor").as_floating();
                }

                if (adTable.count("world_spacing") > 0)
                {
                    ad.worldSpacing = adTable.at("world_spacing").as_floating();
                }

                if (adTable.count("repeat_count") > 0)
                {
                    ad.repeatCount = adTable.at("repeat_count").as_integer();
                }

                if (adTable.count("max_visible") > 0)
                {
                    ad.maxVisible = adTable.at("max_visible").as_integer();
                }

                // Posição no mundo (para WORLD_SPACE/PARALLAX)
                if (ad.placementMode != AdPlacementMode::FIXED_SCREEN && adTable.count("world_position") > 0)
                {
                    auto worldPos = adTable.at("world_position").as_table();
                    ad.worldPosition.x = worldPos.at("x").as_floating();
                    ad.worldPosition.y = worldPos.at("y").as_floating();
                }

                // Timing
                ad.displayDuration = (adTable.count("display_duration") > 0) ? adTable.at("display_duration").as_floating() : 5.0f;
                ad.loop = (adTable.count("loop") > 0) ? adTable.at("loop").as_boolean() : true;

                // Interatividade
                ad.clickable = (adTable.count("clickable") > 0) ? adTable.at("clickable").as_boolean() : false;
                if (ad.clickable && adTable.count("click_url") > 0)
                {
                    ad.clickUrl = adTable.at("click_url").as_string();

                    if (adTable.count("click_area") > 0)
                    {
                        auto clickArea = adTable.at("click_area").as_table();
                        ad.clickArea.x = clickArea.at("x").as_floating();
                        ad.clickArea.y = clickArea.at("y").as_floating();
                        ad.clickArea.width = clickArea.at("width").as_floating();
                        ad.clickArea.height = clickArea.at("height").as_floating();
                    }
                    else
                    {
                        ad.clickArea = ad.bounds;
                    }
                }

                // Animação (se aplicável)
                if (ad.type == AdType::ANIMATED_GIF && adTable.count("animation") > 0)
                {
                    auto anim = adTable.at("animation").as_table();
                    ad.frameCount = anim.at("frame_count").as_integer();
                    ad.frameTime = anim.at("frame_time").as_floating();
                }

                // Carrega asset
                bool loadSuccess = false;
                if (ad.source == AdSource::LOCAL)
                {
                    loadSuccess = (ad.type == AdType::ANIMATED_GIF)
                                      ? LoadAnimatedFrames(ad)
                                      : LoadLocalTexture(ad);
                }
                else
                {
                    loadSuccess = LoadRemoteTexture(ad);
                }

                if (loadSuccess)
                {
                    ad.loaded = true;
                    ads_.push_back(ad);
                    TraceLog(LOG_INFO, "Ad loaded: %s (%s)", ad.id.c_str(), ad.name.c_str());

                    // Geração automática de anúncios em parallax
                    if (adTable.count("auto_generate") > 0 && adTable.at("auto_generate").as_boolean())
                    {
                        float startX = adTable.at("start_x").as_floating();
                        float endX = adTable.at("end_x").as_floating();
                        float spacing = adTable.at("spacing").as_floating();

                        TraceLog(LOG_INFO, "Auto-generating parallax ads from %.1f to %.1f with spacing %.1f",
                                 startX, endX, spacing);

                        // Gera múltiplos anúncios baseados no template
                        int count = 0;
                        for (float x = startX; x <= endX; x += spacing)
                        {
                            if (x == startX)
                                continue; // Pula o primeiro (já foi adicionado como template)

                            Advertisement generatedAd = ad; // Copia o template
                            generatedAd.id = ad.id + "_gen_" + std::to_string(count++);
                            generatedAd.worldPosition.x = x;
                            generatedAd.active = true; // Já ativa automaticamente

                            ads_.push_back(generatedAd);
                        }

                        TraceLog(LOG_INFO, "Generated %d parallax ads from template %s", count, ad.id.c_str());
                    }
                }
                else
                {
                    TraceLog(LOG_WARNING, "Failed to load ad: %s", ad.id.c_str());
                }
            }
        }

        TraceLog(LOG_INFO, "Loaded %d advertisements from %s", (int)ads_.size(), tomlPath.c_str());
        return true;
    }
    catch (const std::exception &e)
    {
        TraceLog(LOG_ERROR, "Failed to parse ads TOML: %s", e.what());
        return false;
    }
}

inline void AdvertisementSystem::Update(float deltaTime)
{
    for (auto &ad : ads_)
    {
        if (!ad.active)
            continue;

        // Atualiza tempo de exibição
        ad.currentTime += deltaTime;

        // Verifica se deve desativar (se não for loop e passou o tempo)
        if (!ad.loop && ad.displayDuration > 0.0f && ad.currentTime >= ad.displayDuration)
        {
            ad.active = false;
            continue;
        }

        // Reseta tempo se for loop
        if (ad.loop && ad.displayDuration > 0.0f && ad.currentTime >= ad.displayDuration)
        {
            ad.currentTime = 0.0f;
        }

        // Atualiza animação (se aplicável)
        if (ad.type == AdType::ANIMATED_GIF && ad.frames != nullptr)
        {
            ad.frameTimer += deltaTime;

            if (ad.frameTimer >= ad.frameTime)
            {
                ad.frameTimer = 0.0f;
                ad.currentFrame = (ad.currentFrame + 1) % ad.frameCount;
            }
        }
    }
}

inline void AdvertisementSystem::Render()
{
    for (const auto &ad : ads_)
    {
        if (!ad.active || !ad.loaded)
            continue;

        // Renderiza apenas anúncios fixos na tela
        if (ad.placementMode != AdPlacementMode::FIXED_SCREEN)
            continue;

        Color tintWithOpacity = ad.tint;
        tintWithOpacity.a = (unsigned char)(ad.opacity * 255);

        if (ad.type == AdType::ANIMATED_GIF && ad.frames != nullptr)
        {
            // Renderiza frame atual
            DrawTexturePro(
                ad.frames[ad.currentFrame],
                {0, 0, (float)ad.frames[ad.currentFrame].width, (float)ad.frames[ad.currentFrame].height},
                ad.bounds,
                {0, 0},
                ad.rotation,
                tintWithOpacity);
        }
        else
        {
            // Renderiza textura estática
            DrawTexturePro(
                ad.texture,
                {0, 0, (float)ad.texture.width, (float)ad.texture.height},
                ad.bounds,
                {0, 0},
                ad.rotation,
                tintWithOpacity);
        }

// Debug: desenha área clicável
#ifdef DEBUG
        if (ad.clickable)
        {
            DrawRectangleLinesEx(ad.clickArea, 1, GREEN);
        }
#endif
    }
}

inline void AdvertisementSystem::RenderWithCamera(const GameCamera &camera)
{
    // Mapa para contar anúncios visíveis por sponsor (para agrupar anúncios do mesmo tipo)
    std::map<std::string, int> visibleCountPerSponsor;

    for (const auto &ad : ads_)
    {
        if (!ad.active || !ad.loaded)
            continue;

        // Renderiza apenas anúncios em mundo/parallax
        if (ad.placementMode == AdPlacementMode::FIXED_SCREEN)
            continue;

        // Verifica se já atingiu o limite para este tipo de anúncio
        int currentCount = visibleCountPerSponsor[ad.sponsor];
        if (currentCount >= ad.maxVisible)
            continue;

        // Calcula posição na tela baseado no modo
        Vector2 screenPos;
        if (ad.placementMode == AdPlacementMode::PARALLAX_BACKGROUND)
        {
            // Aplica parallax
            Vector2 parallaxPos = camera.ApplyParallax(ad.worldPosition, ad.parallaxFactor);
            screenPos = camera.WorldToScreen(parallaxPos);

            Rectangle screenRect = {
                screenPos.x,
                screenPos.y,
                ad.bounds.width,
                ad.bounds.height};

            // Só renderiza se estiver visível
            if (!CheckCollisionRecs(screenRect, camera.viewport))
                continue;

            // Incrementa contador global para este tipo de anúncio
            visibleCountPerSponsor[ad.sponsor]++;

            Color tintWithOpacity = ad.tint;
            tintWithOpacity.a = (unsigned char)(ad.opacity * 255);

            if (ad.type == AdType::ANIMATED_GIF && ad.frames != nullptr)
            {
                DrawTexturePro(
                    ad.frames[ad.currentFrame],
                    {0, 0, (float)ad.frames[ad.currentFrame].width, (float)ad.frames[ad.currentFrame].height},
                    screenRect,
                    {0, 0},
                    ad.rotation,
                    tintWithOpacity);
            }
            else
            {
                DrawTexturePro(
                    ad.texture,
                    {0, 0, (float)ad.texture.width, (float)ad.texture.height},
                    screenRect,
                    {0, 0},
                    ad.rotation,
                    tintWithOpacity);
            }

#ifdef DEBUG
            if (ad.clickable)
            {
                DrawRectangleLinesEx(screenRect, 1, GREEN);
            }
#endif
        }
        else // WORLD_SPACE
        {
            // Posição fixa no mundo
            screenPos = camera.WorldToScreen(ad.worldPosition);

            Rectangle screenRect = {
                screenPos.x,
                screenPos.y,
                ad.bounds.width,
                ad.bounds.height};

            if (!CheckCollisionRecs(screenRect, camera.viewport))
                continue;

            // Incrementa contador global
            visibleCountPerSponsor[ad.sponsor]++;

            Color tintWithOpacity = ad.tint;
            tintWithOpacity.a = (unsigned char)(ad.opacity * 255);

            if (ad.type == AdType::ANIMATED_GIF && ad.frames != nullptr)
            {
                DrawTexturePro(
                    ad.frames[ad.currentFrame],
                    {0, 0, (float)ad.frames[ad.currentFrame].width, (float)ad.frames[ad.currentFrame].height},
                    screenRect,
                    {0, 0},
                    ad.rotation,
                    tintWithOpacity);
            }
            else
            {
                DrawTexturePro(
                    ad.texture,
                    {0, 0, (float)ad.texture.width, (float)ad.texture.height},
                    screenRect,
                    {0, 0},
                    ad.rotation,
                    tintWithOpacity);
            }

#ifdef DEBUG
            if (ad.clickable)
            {
                DrawRectangleLinesEx(screenRect, 1, GREEN);
            }
#endif
        }
    }
}

inline void AdvertisementSystem::GenerateParallaxAds(const std::string &templateAdId, float startX, float endX, float spacing)
{
    // Encontra o template
    Advertisement *templateAd = nullptr;
    for (auto &ad : ads_)
    {
        if (ad.id == templateAdId)
        {
            templateAd = &ad;
            break;
        }
    }

    if (!templateAd || !templateAd->loaded)
    {
        TraceLog(LOG_WARNING, "Template ad '%s' not found or not loaded", templateAdId.c_str());
        return;
    }

    // Gera anúncios ao longo da distância
    int count = 0;
    for (float x = startX; x <= endX; x += spacing)
    {
        Advertisement newAd = *templateAd;
        newAd.id = templateAdId + "_parallax_" + std::to_string(count);
        newAd.worldPosition = {x, templateAd->worldPosition.y};
        newAd.active = true;
        newAd.impressions = 0;

        ads_.push_back(newAd);
        count++;
    }

    TraceLog(LOG_INFO, "Generated %d parallax ads from template '%s'", count, templateAdId.c_str());
}

inline void AdvertisementSystem::ActivateAd(const std::string &id)
{
    for (auto &ad : ads_)
    {
        if (ad.id == id && ad.loaded)
        {
            if (!ad.active)
            {
                ad.active = true;
                ad.currentTime = 0.0f;
                ad.impressions++;
                ad.lastShown = std::chrono::system_clock::now();

                if (ad.impressions == 1)
                {
                    ad.firstShown = ad.lastShown;
                }

                LogImpression(ad);
            }
            break;
        }
    }
}

inline void AdvertisementSystem::DeactivateAd(const std::string &id)
{
    for (auto &ad : ads_)
    {
        if (ad.id == id)
        {
            ad.active = false;
            break;
        }
    }
}

inline void AdvertisementSystem::ToggleAd(const std::string &id)
{
    for (auto &ad : ads_)
    {
        if (ad.id == id && ad.loaded)
        {
            if (ad.active)
            {
                DeactivateAd(id);
            }
            else
            {
                ActivateAd(id);
            }
            break;
        }
    }
}

inline bool AdvertisementSystem::CheckClick(Vector2 mousePos)
{
    for (auto &ad : ads_)
    {
        if (!ad.active || !ad.clickable)
            continue;

        if (CheckCollisionPointRec(mousePos, ad.clickArea))
        {
            ad.clicks++;
            LogClick(ad);

// Abre URL (plataforma específica)
#ifdef _WIN32
            system(("start " + ad.clickUrl).c_str());
#elif __APPLE__
            system(("open " + ad.clickUrl).c_str());
#else
            system(("xdg-open " + ad.clickUrl).c_str());
#endif

            return true;
        }
    }
    return false;
}

inline void AdvertisementSystem::Cleanup()
{
    for (auto &ad : ads_)
    {
        if (ad.texture.id > 0)
        {
            UnloadTexture(ad.texture);
        }
        if (ad.frames != nullptr)
        {
            for (int i = 0; i < ad.frameCount; i++)
            {
                if (ad.frames[i].id > 0)
                {
                    UnloadTexture(ad.frames[i]);
                }
            }
            delete[] ad.frames;
            ad.frames = nullptr;
        }
    }
    ads_.clear();

    if (logStream_.is_open())
    {
        logStream_.close();
    }
}

inline bool AdvertisementSystem::LoadLocalTexture(Advertisement &ad)
{
    std::string fullPath = ad.assetPath;

    if (FileExists(fullPath.c_str()))
    {
        ad.texture = LoadTexture(fullPath.c_str());
        return ad.texture.id > 0;
    }

    ad.loadFailed = true;
    ad.loadError = "File not found: " + fullPath;
    return false;
}

inline bool AdvertisementSystem::LoadRemoteTexture(Advertisement &ad)
{
    // Verifica cache primeiro
    std::string cachePath = GetCachePath(ad.assetPath);
    ad.cachedPath = cachePath;

    if (IsCacheValid(cachePath))
    {
        TraceLog(LOG_INFO, "Loading from cache: %s", cachePath.c_str());
        ad.texture = LoadTexture(cachePath.c_str());
        ad.source = AdSource::CACHED;
        return ad.texture.id > 0;
    }

    // Download para cache
    TraceLog(LOG_INFO, "Downloading ad from: %s", ad.assetPath.c_str());
    if (DownloadToCache(ad.assetPath, cachePath))
    {
        ad.texture = LoadTexture(cachePath.c_str());
        ad.source = AdSource::CACHED;
        return ad.texture.id > 0;
    }

    ad.loadFailed = true;
    ad.loadError = "Failed to download: " + ad.assetPath;
    return false;
}

inline bool AdvertisementSystem::LoadAnimatedFrames(Advertisement &ad)
{
    ad.frames = new Texture2D[ad.frameCount];

    for (int i = 0; i < ad.frameCount; i++)
    {
        std::string framePath = ad.assetPath + "_" + std::to_string(i) + ".png";

        if (!FileExists(framePath.c_str()))
        {
            ad.loadFailed = true;
            ad.loadError = "Frame not found: " + framePath;

            // Limpa frames já carregados
            for (int j = 0; j < i; j++)
            {
                UnloadTexture(ad.frames[j]);
            }
            delete[] ad.frames;
            ad.frames = nullptr;
            return false;
        }

        ad.frames[i] = LoadTexture(framePath.c_str());
        if (ad.frames[i].id == 0)
        {
            ad.loadFailed = true;
            ad.loadError = "Failed to load frame: " + framePath;

            for (int j = 0; j < i; j++)
            {
                UnloadTexture(ad.frames[j]);
            }
            delete[] ad.frames;
            ad.frames = nullptr;
            return false;
        }
    }

    return true;
}

inline std::string AdvertisementSystem::GetCachePath(const std::string &url)
{
    // Cria hash simples da URL para nome do arquivo
    size_t hash = std::hash<std::string>{}(url);
    return config_.cacheDir + "/ad_" + std::to_string(hash) + ".png";
}

inline bool AdvertisementSystem::IsCacheValid(const std::string &cachePath)
{
    if (!FileExists(cachePath.c_str()))
        return false;

    // Verifica idade do cache (simplificado - melhor usar filesystem)
    // Por ora, assume válido se existe
    return true;
}

inline bool AdvertisementSystem::DownloadToCache(const std::string &url, const std::string &cachePath)
{
    // Nota: raylib não tem função de download HTTP built-in
    // Opções: usar curl, libcurl, ou implementar com sockets
    // Por ora, usar system() com curl como exemplo

#ifdef __EMSCRIPTEN__
    // No WASM, usar emscripten_fetch (implementar depois)
    TraceLog(LOG_WARNING, "HTTP download not implemented for WASM");
    return false;
#else
    std::string cmd = "curl -s -o \"" + cachePath + "\" \"" + url + "\"";
    int result = system(cmd.c_str());
    return result == 0 && FileExists(cachePath.c_str());
#endif
}

inline AdType AdvertisementSystem::ParseAdType(const std::string &typeStr)
{
    if (typeStr == "static_image")
        return AdType::STATIC_IMAGE;
    if (typeStr == "animated_gif")
        return AdType::ANIMATED_GIF;
    if (typeStr == "video")
        return AdType::VIDEO;
    if (typeStr == "interactive")
        return AdType::INTERACTIVE;
    return AdType::STATIC_IMAGE;
}

inline AdSource AdvertisementSystem::ParseAdSource(const std::string &sourceStr)
{
    if (sourceStr == "local")
        return AdSource::LOCAL;
    if (sourceStr == "remote")
        return AdSource::REMOTE;
    if (sourceStr == "cached")
        return AdSource::CACHED;
    return AdSource::LOCAL;
}

inline AdPlacementMode AdvertisementSystem::ParsePlacementMode(const std::string &modeStr)
{
    if (modeStr == "fixed_screen")
        return AdPlacementMode::FIXED_SCREEN;
    if (modeStr == "world_space")
        return AdPlacementMode::WORLD_SPACE;
    if (modeStr == "parallax_background")
        return AdPlacementMode::PARALLAX_BACKGROUND;
    return AdPlacementMode::FIXED_SCREEN;
}

inline void AdvertisementSystem::OpenLogFile()
{
    logStream_.open(config_.logFile, std::ios::app);
    if (!logStream_.is_open())
    {
        TraceLog(LOG_WARNING, "Failed to open ad log file: %s", config_.logFile.c_str());
    }
}

inline std::string AdvertisementSystem::GetTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

inline void AdvertisementSystem::LogImpression(const Advertisement &ad)
{
    if (logStream_.is_open())
    {
        logStream_ << "[IMPRESSION] "
                   << GetTimestamp() << " | "
                   << "ID: " << ad.id << " | "
                   << "Name: " << ad.name << " | "
                   << "Sponsor: " << ad.sponsor << " | "
                   << "Total Impressions: " << ad.impressions << "\n";
        logStream_.flush();
    }
}

inline void AdvertisementSystem::LogClick(const Advertisement &ad)
{
    if (logStream_.is_open())
    {
        logStream_ << "[CLICK] "
                   << GetTimestamp() << " | "
                   << "ID: " << ad.id << " | "
                   << "Name: " << ad.name << " | "
                   << "Sponsor: " << ad.sponsor << " | "
                   << "URL: " << ad.clickUrl << " | "
                   << "Total Clicks: " << ad.clicks << "\n";
        logStream_.flush();
    }
}

inline void AdvertisementSystem::CleanupOffscreenAds(const GameCamera &camera, float cleanupDistance)
{
    // Remove anúncios que estão muito longe da câmera (economiza memória)
    // Isso é especialmente útil para anúncios em parallax/world-space que já passaram

    ads_.erase(
        std::remove_if(ads_.begin(), ads_.end(),
                       [&camera, cleanupDistance](const Advertisement &ad)
                       {
                           // Não remove anúncios fixos na tela
                           if (ad.placementMode == AdPlacementMode::FIXED_SCREEN)
                               return false;

                           // Não remove anúncios que nunca foram carregados
                           if (!ad.loaded)
                               return false;

                           // Calcula distância do anúncio em relação à câmera
                           Vector2 adPos = ad.worldPosition;

                           // Para anúncios em parallax, aplica o fator de parallax
                           if (ad.placementMode == AdPlacementMode::PARALLAX_BACKGROUND)
                           {
                               adPos = camera.ApplyParallax(ad.worldPosition, ad.parallaxFactor);
                           }

                           float distanceX = std::abs(adPos.x - camera.position.x);
                           float distanceY = std::abs(adPos.y - camera.position.y);

                           // Remove se estiver além da distância de limpeza em qualquer direção
                           bool shouldRemove = (distanceX > cleanupDistance || distanceY > cleanupDistance);

                           if (shouldRemove)
                           {
                               TraceLog(LOG_INFO, "Cleaning up offscreen ad: %s (distance: %.1f, %.1f)",
                                        ad.id.c_str(), distanceX, distanceY);
                           }

                           return shouldRemove;
                       }),
        ads_.end());
}

inline void AdvertisementSystem::FlushLogs()
{
    if (logStream_.is_open())
    {
        logStream_.flush();
    }
}
