# Integração de Anúncios com Parallax

## Visão Geral

O sistema de anúncios agora suporta três modos de posicionamento:

1. **FIXED_SCREEN**: Anúncios fixos na tela (overlay HUD) - modo padrão
2. **WORLD_SPACE**: Anúncios posicionados no mundo do jogo (movem com câmera)
3. **PARALLAX_BACKGROUND**: Anúncios em backgrounds com efeito parallax

## Configuração no TOML

### Anúncio Fixo na Tela (padrão)
```toml
[[advertisement]]
id = "banner_top_001"
type = "static_image"
source = "local"
asset_path = "ads/banner_top.png"
position = { x = 10.0, y = 10.0 }  # Posição na tela
size = { width = 300.0, height = 60.0 }
# placement_mode não especificado = FIXED_SCREEN
```

### Anúncio no Mundo do Jogo
```toml
[[advertisement]]
id = "world_sign_001"
type = "static_image"
source = "local"
asset_path = "ads/banner_side.png"
placement_mode = "world_space"
world_position = { x = 500.0, y = 300.0 }  # Posição no mundo (metros)
size = { width = 200.0, height = 100.0 }
```

### Template de Anúncio Parallax
```toml
[[advertisement]]
id = "parallax_bg_template"
type = "static_image"
source = "local"
asset_path = "ads/banner_top.png"
placement_mode = "parallax_background"
parallax_factor = 0.5                      # Fator de parallax (0 = fixo, 1 = move 100% com câmera)
world_spacing = 200.0                      # Espaçamento entre anúncios (usado por GenerateParallaxAds)
world_position = { x = 0.0, y = 400.0 }   # Posição Y do template
size = { width = 300.0, height = 60.0 }
opacity = 0.7
```

## Integração no main.cpp

### 1. Incluir Headers Necessários
```cpp
#include "includes/systems/advertisement_system.hpp"
#include "includes/systems/camera_system.hpp"
```

### 2. Inicialização
```cpp
int main()
{
    InitWindow(1920, 1080, "Impale Game");
    
    // Criar sistema de câmera
    GameCamera gameCamera;
    gameCamera.position = {0.0f, 0.0f};
    gameCamera.zoom = 1.0f;
    gameCamera.UpdateViewport(1920, 1080);
    
    // Criar sistema de anúncios
    AdvertisementSystem adSystem;
    adSystem.LoadFromTOML(ASSET_PATH("ads_config.toml"));
    adSystem.SetCamera(&gameCamera);
    
    // Ativar anúncios fixos na tela
    adSystem.ActivateAd("banner_top_001");
    adSystem.ActivateAd("banner_side_002");
    
    // Gerar anúncios parallax ao longo do mapa
    // GenerateParallaxAds(templateId, startX, endX, spacing)
    adSystem.GenerateParallaxAds("parallax_bg_template", 0.0f, 5000.0f, 200.0f);
    
    // Ativar anúncios no mundo
    adSystem.ActivateAd("world_sign_001");
    
    // Loop principal...
}
```

### 3. Loop Principal
```cpp
while (!WindowShouldClose())
{
    float deltaTime = GetFrameTime();
    
    // Atualizar posição da câmera (exemplo)
    if (IsKeyDown(KEY_RIGHT)) gameCamera.position.x += 200.0f * deltaTime;
    if (IsKeyDown(KEY_LEFT))  gameCamera.position.x -= 200.0f * deltaTime;
    if (IsKeyDown(KEY_DOWN))  gameCamera.position.y += 200.0f * deltaTime;
    if (IsKeyDown(KEY_UP))    gameCamera.position.y -= 200.0f * deltaTime;
    
    // Atualizar anúncios
    adSystem.Update(deltaTime);
    
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // Renderizar conteúdo do jogo aqui...
    
    // Renderizar anúncios com câmera (world_space e parallax)
    adSystem.RenderWithCamera(gameCamera);
    
    // Renderizar anúncios fixos na tela (overlay HUD)
    adSystem.Render();
    
    EndDrawing();
}
```

### 4. Gerenciar Cliques
```cpp
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
{
    Vector2 mousePos = GetMousePosition();
    adSystem.CheckClick(mousePos);
}
```

## Características do Sistema

### Parallax Factor
- `0.0`: Anúncio completamente fixo no background (não move com câmera)
- `0.5`: Anúncio move metade da velocidade da câmera (efeito parallax clássico)
- `1.0`: Anúncio move junto com a câmera (equivalente a world_space)

### Culling Automático
O sistema `RenderWithCamera()` automaticamente detecta quais anúncios estão visíveis no viewport e renderiza apenas esses, melhorando performance.

### Geração Dinâmica
`GenerateParallaxAds()` cria cópias do template ao longo de uma distância:
```cpp
// Cria anúncios a cada 200 pixels, de x=0 até x=5000
adSystem.GenerateParallaxAds("parallax_bg_template", 0.0f, 5000.0f, 200.0f);

// Resultado: anúncios em x = 0, 200, 400, 600, ..., 4800, 5000
```

## Exemplo Completo: Background com Múltiplas Camadas

```cpp
// Camada de fundo distante (move devagar)
adSystem.GenerateParallaxAds("far_background_template", 0, 10000, 400);

// Camada intermediária
adSystem.GenerateParallaxAds("mid_background_template", 0, 10000, 250);

// Camada próxima (move mais rápido)
adSystem.GenerateParallaxAds("near_background_template", 0, 10000, 150);
```

Configure os templates no TOML com diferentes `parallax_factor`:
- Far: `parallax_factor = 0.2`
- Mid: `parallax_factor = 0.5`
- Near: `parallax_factor = 0.8`

## Debug

Para visualizar áreas clicáveis, compile com flag `DEBUG`:
```fish
xmake f -m debug
xmake
```

Isso desenhará retângulos verdes ao redor das áreas clicáveis dos anúncios.

## Logging

Todas as impressões e cliques são registrados em `ads_log.txt`:
```
[2024-01-15 10:30:15] IMPRESSION: banner_top_001 (Sponsor: Impale Sponsor)
[2024-01-15 10:30:42] IMPRESSION: parallax_bg_template_parallax_3 (Sponsor: Background Sponsor)
[2024-01-15 10:31:05] CLICK: world_sign_001 (Sponsor: In-World Brand)
```

## Performance

- Anúncios fora da tela são automaticamente descartados (culling)
- Texturas são compartilhadas entre instâncias geradas (economia de memória)
- Cache HTTP evita downloads repetidos
- Sistema otimizado para milhares de anúncios sem impacto no FPS
