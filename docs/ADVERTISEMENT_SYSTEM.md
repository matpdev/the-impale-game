# Sistema de Anúncios e Patrocínio

Sistema dinâmico para exibir anúncios e patrocínios em jogos desenvolvidos com raylib.

## 📋 Características

- ✅ **Configuração via TOML** - Fácil de editar sem recompilar
- ✅ **Assets locais e remotos** - Suporte para URLs HTTP/HTTPS
- ✅ **Cache automático** - Downloads são cacheados localmente
- ✅ **Múltiplos tipos** - Imagens estáticas, GIFs animados, interativos
- ✅ **Logging de impressões** - Rastreamento completo de visualizações e cliques
- ✅ **Sistema de rotação** - Múltiplos anúncios no mesmo espaço
- ✅ **Áreas clicáveis** - Abrir URLs ao clicar
- ✅ **ECS compatível** - Integra com arquitetura Entity-Component-System

## 🚀 Como Usar

### 1. Incluir o Sistema

```cpp
#include "includes/systems/advertisement_system.hpp"

// No seu jogo
AdvertisementSystem adSystem;
```

### 2. Carregar Configuração

```cpp
// Inicialização
if (!adSystem.LoadFromTOML("assets/ads/ads_config.toml")) {
    TraceLog(LOG_ERROR, "Failed to load ads configuration");
}

// Ativa alguns anúncios
adSystem.ActivateAd("banner_top_001");
adSystem.ActivateAd("banner_side_002");
```

### 3. Game Loop

```cpp
// No loop principal
void Update() {
    float deltaTime = GetFrameTime();
    
    // Atualiza sistema de anúncios
    adSystem.Update(deltaTime);
    
    // Verifica cliques
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        adSystem.CheckClick(mousePos);
    }
}

void Render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // Renderiza seu jogo...
    
    // Renderiza anúncios por cima
    adSystem.Render();
    
    EndDrawing();
}
```

### 4. Cleanup

```cpp
// Ao fechar o jogo
adSystem.Cleanup();
```

## 📝 Configuração TOML

### Estrutura Básica

```toml
# Configurações globais
[settings]
log_file = "ads_log.txt"
cache_dir = "cache/ads"
max_cache_age_days = 7
rotation_interval = 10.0

# Definir um anúncio
[[advertisement]]
id = "unique_id"
name = "Descrição do Anúncio"
sponsor = "Nome do Patrocinador"
type = "static_image"          # static_image, animated_gif, video, interactive
source = "local"                # local, remote
asset_path = "path/to/image.png"

position = { x = 10, y = 10 }
size = { width = 300, height = 60 }
rotation = 0.0
opacity = 1.0

display_duration = 5.0          # Segundos (0 = sempre)
loop = true

clickable = true
click_url = "https://exemplo.com"
click_area = { x = 10, y = 10, width = 300, height = 60 }
```

### Tipos de Anúncios

#### 1. Imagem Estática Local

```toml
[[advertisement]]
id = "banner_local"
type = "static_image"
source = "local"
asset_path = "ads/banner.png"
# ...resto da config
```

#### 2. Imagem Remota (HTTP)

```toml
[[advertisement]]
id = "banner_remote"
type = "static_image"
source = "remote"
asset_path = "https://cdn.example.com/ads/banner.png"
# ...resto da config
```

#### 3. GIF Animado

```toml
[[advertisement]]
id = "animated_ad"
type = "animated_gif"
source = "local"
asset_path = "ads/animated"     # Carrega animated_0.png, animated_1.png, etc.

animation = { frame_count = 5, frame_time = 0.2 }
# ...resto da config
```

#### 4. Anúncio Clicável

```toml
[[advertisement]]
id = "clickable_banner"
clickable = true
click_url = "https://www.sponsor.com/promo"
click_area = { x = 10, y = 10, width = 300, height = 60 }
# ...resto da config
```

## 📊 Sistema de Logging

### Formato do Log

O arquivo `ads_log.txt` registra:

```
[IMPRESSION] 2025-11-03 14:30:45 | ID: banner_top_001 | Name: Banner Superior | Sponsor: Empresa XYZ | Total Impressions: 1
[CLICK] 2025-11-03 14:31:12 | ID: banner_top_001 | Name: Banner Superior | Sponsor: Empresa XYZ | URL: https://exemplo.com | Total Clicks: 1
[IMPRESSION] 2025-11-03 14:35:45 | ID: banner_top_001 | Name: Banner Superior | Sponsor: Empresa XYZ | Total Impressions: 2
```

### Análise de Logs

```bash
# Contar impressões totais
grep "\[IMPRESSION\]" ads_log.txt | wc -l

# Contar cliques totais
grep "\[CLICK\]" ads_log.txt | wc -l

# Impressões de um anúncio específico
grep "ID: banner_top_001" ads_log.txt | grep IMPRESSION | wc -l

# CTR (Click-Through Rate) manual
impressions=$(grep "ID: banner_top_001" ads_log.txt | grep IMPRESSION | wc -l)
clicks=$(grep "ID: banner_top_001" ads_log.txt | grep CLICK | wc -l)
echo "CTR: $(echo "scale=2; $clicks * 100 / $impressions" | bc)%"
```

## 🎨 Casos de Uso

### 1. Banner de Carregamento

```cpp
// Mostra patrocínio durante loading
void ShowLoadingScreen() {
    adSystem.ActivateAd("loading_screen_005");
    
    while (IsLoading()) {
        adSystem.Update(GetFrameTime());
        
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText("Loading...", 400, 300, 20, WHITE);
        adSystem.Render();
        EndDrawing();
    }
    
    adSystem.DeactivateAd("loading_screen_005");
}
```

### 2. Rotação de Banners

```cpp
// Alterna entre múltiplos anúncios no mesmo slot
float rotationTimer = 0.0f;
int currentAdIndex = 0;
std::vector<std::string> rotatingAds = {
    "rotation_slot1_a",
    "rotation_slot1_b",
    "rotation_slot1_c"
};

void UpdateRotation(float deltaTime) {
    rotationTimer += deltaTime;
    
    if (rotationTimer >= 10.0f) {  // Troca a cada 10 segundos
        adSystem.DeactivateAd(rotatingAds[currentAdIndex]);
        currentAdIndex = (currentAdIndex + 1) % rotatingAds.size();
        adSystem.ActivateAd(rotatingAds[currentAdIndex]);
        rotationTimer = 0.0f;
    }
}
```

### 3. Patrocínio em Objeto do Jogo

```cpp
// Aplica logo de patrocinador em um objeto
void RenderSponsoredBox() {
    // Renderiza o objeto normalmente
    DrawRectangle(100, 100, 64, 64, BROWN);
    
    // Aplica logo do patrocinador por cima
    // Busca anúncio específico e renderiza manualmente
    for (const auto& ad : adSystem.GetAds()) {
        if (ad.id == "ingame_object_004" && ad.loaded) {
            DrawTexturePro(
                ad.texture,
                {0, 0, (float)ad.texture.width, (float)ad.texture.height},
                {100, 100, 64, 64},
                {0, 0}, 0, WHITE
            );
        }
    }
}
```

### 4. Menu com Patrocínio

```cpp
void RenderMainMenu() {
    // Menu normal
    DrawText("PLAY", 400, 200, 40, WHITE);
    DrawText("OPTIONS", 400, 260, 40, WHITE);
    DrawText("QUIT", 400, 320, 40, WHITE);
    
    // Banner de patrocinador na lateral
    adSystem.Render();  // Renderiza banners configurados
}
```

## 🔧 Integração com ECS

Se você usa um sistema ECS, pode criar entities para anúncios:

```cpp
// Entity de anúncio
Entity CreateAdEntity(EntityManager& em, const std::string& adId) {
    Entity e = em.CreateEntity();
    
    // Busca config do anúncio no sistema
    Advertisement* ad = adSystem.GetAdById(adId);
    if (ad) {
        em.AddComponent<Transform>(e, {ad->bounds.x, ad->bounds.y});
        em.AddComponent<Sprite>(e, {ad->texture, ad->bounds.width, ad->bounds.height});
        em.AddComponent<Advertisement>(e, *ad);
    }
    
    return e;
}
```

## 📦 Assets Remotos

### Download Automático

O sistema automaticamente:
1. Verifica se existe em cache
2. Se não, faz download via HTTP
3. Salva no diretório de cache
4. Carrega a textura

### Dependências

Para download HTTP funcionar, você precisa de `curl` instalado:

```bash
# Linux
sudo apt install curl

# macOS
brew install curl

# Windows
# curl já vem no Windows 10+
```

### WASM/Web

Para builds web, o download HTTP precisa de implementação específica:

```cpp
#ifdef __EMSCRIPTEN__
    // Usar emscripten_fetch API
    // Veja: https://emscripten.org/docs/api_reference/fetch.html
#endif
```

## 🛡️ Boas Práticas

### 1. Performance

- Carregue anúncios na inicialização, não durante gameplay
- Use cache para assets remotos
- Limite número de anúncios ativos simultaneamente

### 2. UX (Experiência do Usuário)

- Não cubra áreas críticas do jogo
- Respeite opacidade para não distrair
- Permita desativar anúncios (modo premium/pago)

### 3. Privacidade

- Informe usuários sobre coleta de dados (impressões/cliques)
- Não rastreie informações pessoais
- Respeite LGPD/GDPR se aplicável

### 4. Monetização

```cpp
// Exemplo: desativa anúncios para usuários premium
if (user.IsPremium()) {
    adSystem.DeactivateAll();
}
```

## 🐛 Debug

Ative modo debug para ver áreas clicáveis:

```cpp
// No advertisement_system.hpp, defina DEBUG
#define DEBUG

// Ou compile com flag
g++ -DDEBUG ...
```

## 📈 Métricas

Cada anúncio rastreia:

- `impressions` - Quantas vezes foi exibido
- `clicks` - Quantas vezes foi clicado
- `firstShown` - Primeira vez que foi mostrado
- `lastShown` - Última vez que foi mostrado

Acesse via:

```cpp
const Advertisement* ad = adSystem.GetAdById("banner_top_001");
if (ad) {
    std::cout << "Impressions: " << ad->impressions << "\n";
    std::cout << "Clicks: " << ad->clicks << "\n";
    
    if (ad->impressions > 0) {
        float ctr = (float)ad->clicks / ad->impressions * 100;
        std::cout << "CTR: " << ctr << "%\n";
    }
}
```

## 🔮 Futuro / Melhorias

- [ ] Suporte a vídeos (MP4)
- [ ] Analytics em tempo real
- [ ] API para servidor de anúncios externo
- [ ] A/B testing
- [ ] Targeting (baseado em perfil de jogador)
- [ ] Animações de transição entre anúncios
- [ ] Suporte a JSON além de TOML
- [ ] Dashboard web para visualizar métricas

## 📄 Licença

Este sistema é parte do projeto Impale e segue a mesma licença MIT.
