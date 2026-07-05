#pragma once
#include "EntityController.hpp"
#include <box2d/box2d.h>

struct LevelDimensions {
    size_t width;
    size_t height;
};

using LevelTileVector = std::vector<std::vector<GameAssets::Textures>>;

class Level {
  private:
    std::vector<std::unique_ptr<Entity>> entities;
    LevelTileVector tiles;
    std::vector<std::unique_ptr<EntityController>> players;
    b2WorldId world;

    uint64_t currentTime = 0;
    uint64_t lastTime = 0;
    float accumulator = 0.f;
    const float physicsStep = 1.0f / 60.0f;

    void drawTile(
        GameAssets::Textures textureId,
        size_t x,
        size_t y,
        AssetManager& assets,
        WindowManager& window,
        float scaleFactor
    );

  public:
    Camera camera;
    const char* levelName;
    LevelDimensions size;
    const bool showFanTriangulation = false;
    bool showHitBoxes = false;
    Level(const char* levelName, LevelDimensions size, WindowManager& window);
    ~Level();
    float update(); // Returns alpha
    void handleInput(GameEventTypes::Input event);
    void draw(WindowManager& window, AssetManager& assets, float alpha);
    void addEntity(
        b2WorldId world,
        b2Polygon polygon,
        b2Vec2 position,
        bool isStatic,
        b2BodyDef bodyDef = b2DefaultBodyDef(),
        b2ShapeDef shapeDef = b2DefaultShapeDef(),
        SDL_FColor hitboxColor = colorToFColor(Colors.Yellow),
        SDL_Texture* texture = nullptr,
        std::optional<b2Vec2> textureSize = std::nullopt
    );
    void addTile(GameAssets::Textures textureId, size_t x, size_t y);
    void addPlayer(AssetManager& assets, std::optional<SDL_JoystickID> joystickId);
    void updatePlayers(const std::vector<SDL_JoystickID>& activeGamepads, AssetManager& assets);
    const std::vector<std::unique_ptr<Entity>>& getEntities() const;
    const LevelTileVector& getTiles() const;
    const std::vector<std::unique_ptr<EntityController>>& getPlayers() const;
    b2WorldId getWorldId() const;
};

std::unique_ptr<Level> getTemplateLevel(AssetManager& assets, WindowManager& window);