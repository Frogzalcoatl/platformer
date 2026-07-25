#pragma once
#include "assets/AssetPaths.hpp"
#include "assets/Colors.hpp"
#include "assets/LevelAssets.hpp"
#include "events/EventQueue.hpp"
#include "platformer/EntityController.hpp"
#include "system/AudioManager.hpp"
#include "system/InputManager.hpp"
#include <box2d/box2d.h>
#include <optional>

struct LevelDimensions {
    size_t width;
    size_t height;
};

struct LevelDrawInfo {
    size_t tiles = 0;
    size_t entities = 0;
};

struct LevelDrawDimensions {
    size_t minX = 0;
    size_t maxX = 0;
    size_t minY = 0;
    size_t maxY = 0;
};

struct Player {
    InputSource source;
    std::unique_ptr<EntityController> controller;
};

using LevelTileVector = std::vector<std::vector<AssetPaths::Textures::TileTypes>>;
using EntitiesVector = std::vector<std::unique_ptr<Entity>>;

class Level {
  private:
    LevelTileVector tiles;
    EntitiesVector entities;
    std::vector<Player> players;
    b2WorldId world;
    LevelDimensions levelSize;
    const char* levelName;
    LevelAssetsVector requiredAssets;
    Camera camera;
    LevelDrawInfo drawInfo;
    size_t tileCount = 0; // Incremented when addTile is run

    std::array<SDL_Texture*, static_cast<size_t>(AssetPaths::Textures::TileTypes::TileCount)>
        tileTextureCache{};

    uint64_t currentTime = 0;
    uint64_t lastTime = 0;
    float accumulator = 0.f;
    const float physicsStep = 1.0f / 60.0f;
    float alpha = 0.f; // Value between 0.0 and 1.0 representing how far the game is between the
                       // last physics step and the next.

    void drawTile(
        AssetPaths::Textures::TileTypes tileId,
        size_t x,
        size_t y,
        WindowManager& window,
        float cameraScale
    );

    void
    loadLevelAsset(const LevelAsset& asset, AssetManager& assetManager, AudioManager& audioManager);

    void unloadLevelAsset(const LevelAsset& asset, AssetManager& assetManager);

    void
    handlePreviousAssetsVector(const LevelAssetsVector& previousAssets, AssetManager& assetManager);

  public:
    Level(
        const char* levelName,
        LevelDimensions size,
        WindowManager& window,
        AssetManager& assetManager,
        AudioManager& audioManager,
        LevelAssetsVector requiredAssets,
        std::optional<const LevelAssetsVector> previousAssets = std::nullopt
    );
    ~Level();
    bool showFanTriangulation = false;
    bool showHitBoxes = false;
    bool showLevelBounds = false;
    SDL_Color backgroundColor = Colors::SkyBlue;

    void update();

    void handleInput(GameEventTypes::Input event);

    void draw(WindowManager& window, AssetManager& assetManager);

    b2WorldId getWorldId() const;

    LevelDimensions getSize() const;

    Camera* getCamera();

    std::string_view getName() const;

    size_t getTileCount() const;

    const EntitiesVector& getEntities() const;

    void addEntity(
        b2Polygon polygon,
        b2Vec2 position,
        b2BodyDef bodyDef = b2DefaultBodyDef(),
        b2ShapeDef shapeDef = b2DefaultShapeDef(),
        SDL_FColor hitboxColor = colorToFColor(Colors::Yellow),
        SDL_Texture* texture = nullptr,
        std::optional<b2Vec2> textureSize = std::nullopt
    );

    const LevelTileVector& getTiles() const;

    void addTile(AssetPaths::Textures::TileTypes tileId, size_t x, size_t y);

    void removeTile(size_t x, size_t y);

    const std::vector<Player>& getPlayers() const;

    void addPlayer(InputSource playerSource, AssetManager& assets);

    void updatePlayers(const PlayerSources& playerSources, AssetManager& assets);

    const LevelDrawInfo& drawnLastFrame() const;

    void loadRequiredAssets(AssetManager& assetManager, AudioManager& audioManager);

    void unloadRequiredAssets(AssetManager& assetManager);

    const LevelAssetsVector& getRequiredAssets() const {
        return requiredAssets;
    }

    Entity* getPlayerEntity(size_t playerIndex);
};

std::unique_ptr<Level> getTestLevel(
    AssetManager& assetManager,
    WindowManager& window,
    AudioManager& audioManager,
    const LevelAssetsVector& previousAssets
);