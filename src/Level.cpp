#include "Level.hpp"
#include "Drawing.hpp"
#include <algorithm>

using namespace AssetPaths;

Level::Level(
    const char* levelName,
    LevelDimensions size,
    WindowManager& window,
    AssetManager& assetManager,
    AudioManager& audioManager,
    LevelAssetsVector requiredAssets,
    std::optional<const LevelAssetsVector> previousAssetsOpt
)
    : levelSize(size), levelName(levelName), requiredAssets(requiredAssets),
      camera(nullptr, window) {
    if (previousAssetsOpt.has_value()) {
        handlePreviousAssetsVector(previousAssetsOpt.value(), assetManager);
    }
    loadRequiredAssets(assetManager, audioManager);
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -60.f};
    world = b2CreateWorld(&worldDef);
    SDL_Log("Created box2d world for level \"%s\"", levelName);
    tiles.resize(size.width);
    for (auto& column : tiles) {
        column.resize(size.height);
    }
}

Level::~Level() {
    entities.clear();
    players.clear();
    b2DestroyWorld(world);
    SDL_Log("Destroyed box2d world for level \"%s\"", levelName);
}

void Level::loadLevelAsset(
    const LevelAsset& asset, AssetManager& assetManager, AudioManager& audioManager
) {
    switch (asset.type) {
    case AssetTypes::Audio: {
        MIX_Mixer* mixerDevice = audioManager.getMixerDevice();
        if (mixerDevice) {
            assetManager.getAudio(asset.relativePath, mixerDevice, asset.shouldBePredecodedAudio);
        }
    }; break;
    case AssetTypes::FontSdl: {
        if (asset.fontInfo.has_value()) {
            const FontInfo& fontInfo = asset.fontInfo.value();
            assetManager.getSDLFont(asset.relativePath, fontInfo.ptSize, fontInfo.style);
        } else {
            SDL_LogWarn(
                SDL_LOG_CATEGORY_APPLICATION,
                "Unable to unload font \"%.*s\": No font info provided.",
                static_cast<int>(asset.relativePath.length()),
                asset.relativePath.data()
            );
        }
    }; break;
    case AssetTypes::Texture: {
        assetManager.getTexture(asset.relativePath);
    }; break;
    }
}

void Level::unloadLevelAsset(const LevelAsset& asset, AssetManager& assetManager) {
    switch (asset.type) {
    case AssetTypes::Audio: {
        assetManager.unloadAudio(asset.relativePath, asset.shouldBePredecodedAudio);
    }; break;
    case AssetTypes::FontSdl: {
        if (asset.fontInfo.has_value()) {
            const FontInfo& fontInfo = asset.fontInfo.value();
            assetManager.unloadSDLFont(asset.relativePath, fontInfo.ptSize, fontInfo.style);
        } else {
            SDL_LogWarn(
                SDL_LOG_CATEGORY_APPLICATION,
                "Unable to unload font \"%.*s\": No font info provided.",
                static_cast<int>(asset.relativePath.length()),
                asset.relativePath.data()
            );
        }
    }; break;
    case AssetTypes::Texture: {
        assetManager.unloadTexture(asset.relativePath);
    }; break;
    }
}

void Level::loadRequiredAssets(AssetManager& assetManager, AudioManager& audioManager) {
    for (const auto& asset : requiredAssets) {
        loadLevelAsset(asset, assetManager, audioManager);
    }
}

void Level::unloadRequiredAssets(AssetManager& assetManager) {
    for (const auto& asset : requiredAssets) {
        unloadLevelAsset(asset, assetManager);
    }
}

Entity* Level::getPlayerEntity(size_t playerIndex) {
    if (playerIndex >= players.size()) {
        return nullptr;
    }
    Player& player = players[playerIndex];
    if (!player.controller) {
        return nullptr;
    }
    return player.controller->getEntity();
}

void Level::handlePreviousAssetsVector(
    const LevelAssetsVector& previousAssets, AssetManager& assetManager
) {
    LevelAssetsVector assetsToUnload;
    for (const auto& asset : previousAssets) {
        bool notInRequiredAssets =
            std::find(requiredAssets.begin(), requiredAssets.end(), asset) == requiredAssets.end();
        if (notInRequiredAssets) {
            assetsToUnload.push_back(asset);
        }
    }
    for (const auto& asset : assetsToUnload) {
        unloadLevelAsset(asset, assetManager);
    }
}

void Level::drawTile(
    AssetPaths::Textures::TileTypes tileId,
    size_t x,
    size_t y,
    AssetManager& assets,
    WindowManager& window,
    float cameraScale
) {
    assert(tileId < Textures::TileTypes::TileCount);
    if (tileId >= Textures::TileTypes::TileCount) {
        return;
    }
    std::string_view relativePath = Textures::TilePaths[static_cast<size_t>(tileId)];
    SDL_Texture* texture = assets.getTexture(relativePath);
    if (!texture) {
        return;
    }
    Drawing::texture(
        texture,
        window,
        b2Vec2{static_cast<float>(x + 0.5f), static_cast<float>(y + 0.5f)},
        b2Vec2{1.f, 1.f},
        cameraScale,
        camera.getOffsetPixels()
    );
}

void Level::update() {
    currentTime = SDL_GetTicks();
    float deltaTime = (float)(currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    if (deltaTime > 0.1f) {
        deltaTime = 0.1f;
    }
    accumulator += deltaTime;
    while (accumulator >= physicsStep) {
        for (const auto& entity : entities) {
            if (entity) {
                entity->savePreviousState();
            }
        }
        for (auto& player : players) {
            if (player.controller) {
                player.controller->update();
            }
        }
        b2World_Step(world, physicsStep, 4);
        accumulator -= physicsStep;
    }
    alpha = accumulator / physicsStep;
}

void Level::handleInput(GameEventTypes::Input event) {
    for (auto& player : players) {
        if (!player.controller) {
            continue;
        }
        if (player.source == event.sourceInfo) {
            player.controller->handleInput(event, &camera, alpha);
            return;
        }
    }
}

void Level::draw(WindowManager& window, AssetManager& assets) {
    camera.run(alpha);
    if (tiles.empty()) {
        return;
    }
    const float cameraScale = camera.getScaleFactor();
    const WindowVec2 cameraOffsetPixels = camera.getOffsetPixels();
    const b2Vec2 cameraSizeWorld = camera.getSize();
    const b2Vec2 cameraOffsetWorld = camera.getOffsetWorld();
    const size_t minX = static_cast<size_t>(SDL_max(SDL_floorf(cameraOffsetWorld.x), 0.f));
    const size_t maxX = static_cast<size_t>(
        SDL_min(SDL_ceilf(cameraOffsetWorld.x + cameraSizeWorld.x), levelSize.width - 1)
    );
    const size_t minY = static_cast<size_t>(SDL_max(SDL_floorf(cameraOffsetWorld.y), 0.f));
    const size_t maxY = static_cast<size_t>(
        SDL_min(SDL_ceilf(cameraOffsetWorld.y + cameraSizeWorld.y), levelSize.height - 1)
    );
    drawInfo = LevelDrawInfo{};
    for (size_t x = minX; x <= maxX; x++) {
        for (size_t y = minY; y <= maxY; y++) {
            if (tiles[x][y] != Textures::TileTypes::Air) {
                drawTile(tiles[x][y], x, y, assets, window, cameraScale);
                drawInfo.tiles++;
            }
        }
    }
    float minXFloat = static_cast<float>(minX);
    float maxXFloat = static_cast<float>(maxX);
    float minYFloat = static_cast<float>(minY);
    float maxYFloat = static_cast<float>(maxY);
    for (const auto& entity : entities) {
        bool didDrawEntity = false;
        if (!entity) {
            continue;
        }
        b2Transform transform;
        transform.p = entity->getInterpolatedPosition(alpha);
        transform.q = entity->getInterpolatedRotation(alpha);
        b2AABB entityAABB = b2ComputePolygonAABB(&entity->getPolygon(), transform);
        b2Vec2 entitySize;
        entitySize.x = entityAABB.upperBound.x - entityAABB.lowerBound.x;
        entitySize.y = entityAABB.upperBound.y - entityAABB.lowerBound.y;
        if (!Drawing::shouldDrawObject(
                entityAABB.lowerBound, entitySize, minXFloat, maxXFloat, minYFloat, maxYFloat
            )) {
            b2Vec2 nametagPos = entity->getNametagWorldPos(alpha);
            b2Vec2 nametagSize =
                entity->getNametagWorldSize(assets.TextRenderScale, assets.TextWorldSizeMultiplier);
            b2Vec2 posBottomLeft{
                nametagPos.x - nametagSize.x / 2.f, nametagPos.y - nametagSize.y / 2.f
            };
            if (Drawing::shouldDrawObject(
                    posBottomLeft, nametagSize, minXFloat, maxXFloat, minYFloat, maxYFloat
                )) {
                entity->drawNametag(window, alpha, cameraScale, cameraOffsetPixels, assets);
            }
            continue;
        }
        if (entity->draw(window, alpha, cameraScale, cameraOffsetPixels, assets)) {
            didDrawEntity = true;
        }
        if (showFanTriangulation) {
            Drawing::showFanTriangulation(
                entity->getPolygon(), window, transform, cameraScale, cameraOffsetPixels
            );
            didDrawEntity = true;
        }
        if (showHitBoxes) {
            entity->drawHitbox(window, alpha, cameraScale, cameraOffsetPixels);
            didDrawEntity = true;
        }
        if (didDrawEntity) {
            drawInfo.entities++;
        }
    }
    if (showLevelBounds) {
        LevelDimensions bounds = getSize();
        Drawing::rectangleBorders(
            b2Vec2{0.f, 0.f},
            b2Vec2{static_cast<float>(bounds.width), static_cast<float>(bounds.height)},
            window,
            cameraScale,
            cameraOffsetPixels,
            colorToFColor(Colors::Blue)
        );
    }
}

b2WorldId Level::getWorldId() const {
    return world;
}

LevelDimensions Level::getSize() const {
    size_t width = tiles.size();
    size_t height = 0;
    if (!tiles.empty()) {
        height = tiles[0].size();
    }
    return LevelDimensions{width, height};
}

Camera* Level::getCamera() {
    return &camera;
}

std::string_view Level::getName() const {
    return levelName;
}

size_t Level::getTileCount() const {
    return tileCount;
}

const EntitiesVector& Level::getEntities() const {
    return entities;
}

void Level::addEntity(
    b2Polygon polygon,
    b2Vec2 position,
    b2BodyDef bodyDef,
    b2ShapeDef shapeDef,
    SDL_FColor hitboxColor,
    SDL_Texture* texture,
    std::optional<b2Vec2> textureSize
) {
    auto entity = std::make_unique<Entity>(
        world, polygon, position, bodyDef, shapeDef, hitboxColor, texture, textureSize
    );
    entities.push_back(std::move(entity));
    b2World_Step(world, physicsStep, 4);
}

const LevelTileVector& Level::getTiles() const {
    return tiles;
}

void Level::addTile(Textures::TileTypes tileId, size_t x, size_t y) {
    assert(tileId < Textures::TileTypes::TileCount);
    if (tileId >= Textures::TileTypes::TileCount) {
        return;
    }
    std::string_view relativePathStr = Textures::TilePaths[static_cast<size_t>(tileId)];
    if (x >= levelSize.width || y >= levelSize.height) {
        std::filesystem::path relativePath = relativePathStr;
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Invalid tile %s position (%zu, %zu). Level size is (%zu, %zu)",
            relativePath.filename().string().c_str(),
            x,
            y,
            levelSize.width,
            levelSize.height
        );
        return;
    }
    tiles[x][y] = tileId;
    if (tileId != Textures::TileTypes::Air) {
        tileCount++;
    }
    if (tileId == Textures::TileTypes::Air && tileCount > 0) {
        tileCount--;
    }
}

void Level::removeTile(size_t x, size_t y) {
    if (x >= levelSize.width || y >= levelSize.height) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to remove tile at position (%zu, %zu). Level size is (%zu, %zu)",
            x,
            y,
            levelSize.width,
            levelSize.height
        );
        return;
    }
    tiles[x][y] = Textures::TileTypes::Air;
}

const std::vector<Player>& Level::getPlayers() const {
    return players;
}

void Level::addPlayer(InputSource playerSource, AssetManager& assets) {
    b2BodyDef playerBodyDef = b2DefaultBodyDef();
    playerBodyDef.fixedRotation = true;
    playerBodyDef.type = b2_dynamicBody;
    b2ShapeDef playerShapeDef = b2DefaultShapeDef();
    playerShapeDef.material.friction = 0.f;
    playerShapeDef.density = 4.f;
    std::unique_ptr<Entity> playerEntity = std::make_unique<Entity>(
        world,
        b2MakeBox(0.5f, 1.f),
        b2Vec2{10.f, 4.f},
        playerBodyDef,
        playerShapeDef,
        colorToFColor(Colors::Yellow),
        assets.getTexture(Textures::Player),
        b2Vec2{1.f, 2.f}
    );
    const size_t currentPlayerCount = players.size();
    if (currentPlayerCount == 0) {
        camera.entityToFollow = playerEntity.get();
    }
    std::string nametag = "Player " + std::to_string(currentPlayerCount + 1);
    playerEntity->setNametag(nametag, assets);
    std::unique_ptr<EntityController> entityController =
        std::make_unique<EntityController>(*playerEntity);
    entityController->spawnPoint = b2Vec2{4.f, 4.f};
    players.push_back(Player{playerSource, std::move(entityController)});
    entities.push_back(std::move(playerEntity));
    b2World_Step(world, physicsStep, 4);
    SDL_Log("Added new player \"%s\" to level \"%s\"", nametag.c_str(), levelName);
}

void Level::updatePlayers(const PlayerSources& playerSources, AssetManager& assets) {
    auto playerIt = players.begin();
    while (playerIt != players.end()) {
        bool stillActive =
            std::find(playerSources.begin(), playerSources.end(), playerIt->source) !=
            playerSources.end();
        if (stillActive) {
            playerIt++;
        } else {
            Entity* entityPtr = playerIt->controller->getEntity();
            if (camera.entityToFollow == entityPtr) {
                // TODO: Placeholder logic until camera following is improved
                camera.entityToFollow = nullptr;
            }
            std::string nametag = entityPtr->getNametagStr();
            if (entityPtr) {
                auto entityIt =
                    std::find_if(entities.begin(), entities.end(), [entityPtr](const auto& entity) {
                        return entity.get() == entityPtr;
                    });
                if (entityIt != entities.end()) {
                    entities.erase(entityIt);
                }
            }
            playerIt = players.erase(playerIt); // Returns the next iterator.
            SDL_Log("Removed player \"%s\"", nametag.c_str());
        }
    }
    // Update nametags in case indices were shifted.
    for (size_t i = 0; i < players.size(); i++) {
        Entity* playerEntity = players[i].controller->getEntity();
        if (playerEntity) {
            std::string nametag = "Player " + std::to_string(i + 1);
            playerEntity->setNametag(nametag, assets);
        }
    }
    // Add players for new player sources
    for (const auto playerSourceOpt : playerSources) {
        if (!playerSourceOpt.has_value()) {
            continue;
        }
        const InputSource& playerSource = playerSourceOpt.value();
        // In lambda functions [] is the capture clause.
        // [&] means the lambda can access any variable in its outer scope.
        bool alreadyExists =
            std::any_of(players.begin(), players.end(), [&playerSource](const Player& p) {
                return playerSource == p.source;
            });
        if (!alreadyExists) {
            addPlayer(playerSource, assets);
        }
    }
    if (!players.empty()) {
        camera.entityToFollow = players[0].controller->getEntity();
    }
}

const LevelDrawInfo& Level::drawnLastFrame() const {
    return drawInfo;
}

std::unique_ptr<Level> getTestLevel(
    AssetManager& assetManager,
    WindowManager& window,
    AudioManager& audioManager,
    const LevelAssetsVector& previousAssets
) {
    std::unique_ptr<Level> level = std::make_unique<Level>(
        "Test",
        LevelDimensions{100, 40},
        window,
        assetManager,
        audioManager,
        LevelAssets::Template,
        previousAssets
    );
    level->showLevelBounds = true;
    const int GroundWidth = 50;
    const int GroundHeight = 2;
    const int WallHeight = 20;
    const int WallPosLeft = 0;
    const int WallPosRight = GroundWidth;
    level->addEntity(
        b2MakeBox(static_cast<float>(GroundWidth) / 2.f, static_cast<float>(GroundHeight) / 2.f),
        b2Vec2{static_cast<float>(GroundWidth) / 2.f, 1.f}
    );
    level->addEntity(
        b2MakeBox(0.5f, static_cast<float>(WallHeight) / 2.f),
        b2Vec2{WallPosLeft + 0.5f, static_cast<float>(WallHeight) / 2.f}
    );
    level->addEntity(
        b2MakeBox(0.5f, static_cast<float>(WallHeight) / 2.f),
        b2Vec2{WallPosRight + 0.5f, static_cast<float>(WallHeight) / 2.f}
    );
    b2BodyDef dynamicBodyDef = b2DefaultBodyDef();
    dynamicBodyDef.type = b2_dynamicBody;
    level->addEntity(
        b2MakeBox(0.5f, 2.f),
        b2Vec2{28.f, 4.f},
        dynamicBodyDef,
        b2DefaultShapeDef(),
        colorToFColor(Colors::Yellow),
        assetManager.getTexture(Textures::Log),
        b2Vec2{1.f, 4.f}
    );
    level->addEntity(
        b2MakeBox(0.5f, 1.f),
        b2Vec2{8.f, 3.f},
        dynamicBodyDef,
        b2DefaultShapeDef(),
        colorToFColor(Colors::Yellow),
        assetManager.getTexture(Textures::Log),
        b2Vec2{1.f, 2.f}
    );
    for (int i = 1; i < GroundWidth; i++) {
        level->addTile(Textures::TileTypes::Dirt, i, 0);
        level->addTile(Textures::TileTypes::Grass, i, 1);
    }
    for (int i = 0; i < WallHeight; i++) {
        level->addTile(Textures::TileTypes::Stone, WallPosLeft, i);
        level->addTile(Textures::TileTypes::Stone, WallPosRight, i);
    }
    return level;
}