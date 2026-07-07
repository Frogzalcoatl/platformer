#pragma once
#include "EntityController.hpp"
#include <box2d/box2d.h>

struct LevelDimensions {
    size_t width;
    size_t height;
};

struct LevelDrawInfo {
    size_t tiles = 0;
    size_t entities = 0;
    size_t nametags = 0;
};

struct LevelDrawDimensions {
    size_t minX = 0;
    size_t maxX = 0;
    size_t minY = 0;
    size_t maxY = 0;
};

using LevelTileVector = std::vector<std::vector<GameAssets::Textures>>;
using EntitiesVector = std::vector<std::unique_ptr<Entity>>;
using PlayersVector = std::vector<std::unique_ptr<EntityController>>;

class Level {
  private:
    LevelTileVector tiles;
    EntitiesVector entities;
    PlayersVector players;
    b2WorldId world;
    LevelDimensions size;
    const char* levelName;
    Camera camera;
    LevelDrawInfo drawInfo;
    size_t tileCount = 0; // Incremented when addTile is run

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
        float cameraScale
    );

  public:
    Level(const char* levelName, LevelDimensions size, WindowManager& window);
    ~Level();
    bool showFanTriangulation = false;
    bool showHitBoxes = false;
    bool showLevelBounds = false;

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
    void removeTile(size_t x, size_t y);
    void addPlayer(AssetManager& assets, std::optional<SDL_JoystickID> joystickId);
    void updatePlayers(const std::vector<SDL_JoystickID>& activeGamepads, AssetManager& assets);

    const EntitiesVector& getEntities() const;
    const LevelTileVector& getTiles() const;
    const PlayersVector& getPlayers() const;
    b2WorldId getWorldId() const;
    const LevelDrawInfo& drawnLastFrame() const;
    LevelDimensions getSize() const;
    Camera& getCamera();
    std::string_view getName() const;
    size_t getTileCount() const;
};

std::unique_ptr<Level> getTestLevel(AssetManager& assets, WindowManager& window);