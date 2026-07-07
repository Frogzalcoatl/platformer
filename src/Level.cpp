#include "Level.hpp"
#include "Drawing.hpp"
#include <algorithm>

Level::Level(const char* levelName, LevelDimensions size, WindowManager& window)
    : size(size), levelName(levelName), camera(nullptr, window) {
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

float Level::update() {
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
            if (player) {
                player->update();
            }
        }
        b2World_Step(world, physicsStep, 4);
        accumulator -= physicsStep;
    }
    return accumulator / physicsStep;
}

void Level::draw(WindowManager& window, AssetManager& assets, float alpha) {
    camera.run(alpha);
    if (tiles.empty()) {
        return;
    }
    const size_t worldHeight = tiles[0].size();
    const size_t worldWidth = tiles.size();
    const float cameraScale = camera.getScaleFactor();
    const WindowVec2 cameraOffsetPixels = camera.getOffsetPixels();
    const b2Vec2 cameraSizeWorld = camera.getSize();
    const b2Vec2 cameraOffsetWorld = camera.getOffsetWorld();
    const size_t minX = static_cast<size_t>(SDL_max(SDL_floorf(cameraOffsetWorld.x), 0.f));
    const size_t maxX = static_cast<size_t>(
        SDL_min(SDL_ceilf(cameraOffsetWorld.x + cameraSizeWorld.x), worldWidth - 1)
    );
    const size_t minY = static_cast<size_t>(SDL_max(SDL_floorf(cameraOffsetWorld.y), 0.f));
    const size_t maxY = static_cast<size_t>(
        SDL_min(SDL_ceilf(cameraOffsetWorld.y + cameraSizeWorld.y), worldHeight - 1)
    );
    drawInfo = LevelDrawInfo{};
    for (size_t x = minX; x <= maxX; x++) {
        for (size_t y = minY; y <= maxY; y++) {
            if (tiles[x][y] != GameAssets::Textures::None) {
                drawTile(tiles[x][y], x, y, assets, window, cameraScale);
                drawInfo.tiles++;
            }
        }
    }
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
        if (!Drawing::shouldDrawObject(entityAABB.lowerBound, entitySize, minX, maxX, minY, maxY)) {
            continue;
        }
        if (entity->draw(window, alpha, cameraScale, cameraOffsetPixels)) {
            didDrawEntity = true;
        }
        if (showFanTriangulation) {
            b2Transform transform;
            transform.p = entity->getInterpolatedPosition(alpha);
            transform.q = entity->getInterpolatedRotation(alpha);
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
    for (const auto& player : players) {
        if (!player) {
            continue;
        }
        const b2Vec2 nametagPosCenter = player.get()->getNametagWorldPos(alpha);
        const b2Vec2 nametagSize = player.get()->getNametagWorldSize(
            assets.TextRenderScale, assets.TextWorldSizeMultiplier
        );
        b2Vec2 nametagPosBottomLeft = b2Vec2{
            nametagPosCenter.x - nametagSize.x / 2.f, nametagPosCenter.y - nametagSize.y / 2.f
        };
        if (Drawing::shouldDrawObject(nametagPosBottomLeft, nametagSize, minX, maxX, minY, maxY)) {
            player->drawNameTag(window, assets, cameraScale, cameraOffsetPixels, alpha);
            drawInfo.nametags++;
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
            colorToFColor(Colors.Blue)
        );
    }
}

void Level::drawTile(
    GameAssets::Textures textureId,
    size_t x,
    size_t y,
    AssetManager& assets,
    WindowManager& window,
    float cameraScale
) {
    SDL_Texture* texture = assets.getTexture(textureId);
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

void Level::handleInput(GameEventTypes::Input event) {
    EntityController* playerForInput = nullptr;
    for (auto& player : players) {
        if (!player) {
            continue;
        }
        if (player->joystickId == event.joystickId) {
            playerForInput = player.get();
        }
    }
    if (!playerForInput) {
        return;
    }
    playerForInput->handleInput(event, &camera);
}

void Level::addEntity(
    b2WorldId world,
    b2Polygon polygon,
    b2Vec2 position,
    bool isStatic,
    b2BodyDef bodyDef,
    b2ShapeDef shapeDef,
    SDL_FColor hitboxColor,
    SDL_Texture* texture,
    std::optional<b2Vec2> textureSize
) {
    auto entity = std::make_unique<Entity>(
        world, polygon, position, isStatic, bodyDef, shapeDef, hitboxColor, texture, textureSize
    );
    entities.push_back(std::move(entity));
    b2World_Step(world, physicsStep, 4);
}

void Level::addTile(GameAssets::Textures textureId, size_t x, size_t y) {
    assert(textureId < GameAssets::Textures::TexturesCount);
    if (x >= size.width || y >= size.height) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Invalid tile %s position (%zu, %zu). Level size is (%zu, %zu)",
            GameAssets::FileNames.Textures[static_cast<size_t>(textureId)],
            x,
            y,
            size.width,
            size.height
        );
        return;
    }
    tiles[x][y] = textureId;
    if (textureId != GameAssets::Textures::None) {
        tileCount++;
    }
    if (textureId == GameAssets::Textures::None && tileCount > 0) {
        tileCount--;
    }
}

void Level::removeTile(size_t x, size_t y) {
    if (x >= size.width || y >= size.height) {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Unable to remove tile at position (%zu, %zu). Level size is (%zu, %zu)",
            x,
            y,
            size.width,
            size.height
        );
        return;
    }
    tiles[x][y] = GameAssets::Textures::None;
}

void Level::addPlayer(AssetManager& assets, std::optional<SDL_JoystickID> joystickId) {
    if (!joystickId.has_value()) {
        for (const auto& player : players) {
            if (!player) {
                continue;
            }
            if (player.get()->joystickId == joystickId) {
                SDL_LogWarn(
                    SDL_LOG_CATEGORY_APPLICATION,
                    "Adding a second player with no joystick id is not allowed"
                );
                return;
            }
        }
    }
    b2BodyDef playerBodyDef = b2DefaultBodyDef();
    playerBodyDef.fixedRotation = true;
    b2ShapeDef playerShapeDef = b2DefaultShapeDef();
    playerShapeDef.material.friction = 0.f;
    playerShapeDef.density = 4.f;
    auto playerEntity = std::make_unique<Entity>(
        world,
        b2MakeBox(0.5f, 1.f),
        b2Vec2{10.f, 4.f},
        false,
        playerBodyDef,
        playerShapeDef,
        colorToFColor(Colors.Yellow),
        assets.getTexture(GameAssets::Textures::Player),
        b2Vec2{1.f, 2.f}
    );
    if (!joystickId.has_value()) {
        camera.entityToFollow = playerEntity.get();
    }
    auto controller = std::make_unique<EntityController>(*playerEntity, assets, joystickId);
    controller->spawnPoint = b2Vec2{4.f, 4.f};
    controller->joystickId = joystickId;
    players.push_back(std::move(controller));
    entities.push_back(std::move(playerEntity));
    b2World_Step(world, physicsStep, 4);
    SDL_Log(
        "Spawned new player with joystickId \"%s\"", std::to_string(joystickId.value_or(1)).c_str()
    );
}

void Level::updatePlayers(const std::vector<SDL_JoystickID>& activeGamepads, AssetManager& assets) {
    auto iterator = players.begin();
    while (iterator != players.end()) {
        if ((*iterator)->joystickId.has_value()) {
            SDL_JoystickID currentId = (*iterator)->joystickId.value();
            // if currentId is not in activeGamepads type shi
            if (std::find(activeGamepads.begin(), activeGamepads.end(), currentId) ==
                activeGamepads.end()) {
                Entity* entityPtr = (*iterator)->getEntity();
                if (camera.entityToFollow == entityPtr) {
                    camera.entityToFollow = nullptr;
                }
                if (entityPtr) {
                    auto entityIterator = std::find_if(
                        entities.begin(), entities.end(), [entityPtr](const auto& entity) {
                            return entity.get() == entityPtr;
                        }
                    );
                    if (entityIterator != entities.end()) {
                        entities.erase(entityIterator);
                    }
                }
                iterator = players.erase(iterator);
                SDL_Log("Removed player with joystickId %d", static_cast<int>(currentId));
                continue;
            }
        }
        iterator++;
    }
    for (const auto gamepadId : activeGamepads) {
        bool alreadyExists =
            std::any_of(players.begin(), players.end(), [gamepadId](const auto& controller) {
                return controller->joystickId == gamepadId;
            });
        if (!alreadyExists) {
            addPlayer(assets, gamepadId);
        }
    }
}

const EntitiesVector& Level::getEntities() const {
    return entities;
}
const LevelTileVector& Level::getTiles() const {
    return tiles;
}
const PlayersVector& Level::getPlayers() const {
    return players;
}

b2WorldId Level::getWorldId() const {
    return world;
}

const LevelDrawInfo& Level::drawnLastFrame() const {
    return drawInfo;
}

LevelDimensions Level::getSize() const {
    size_t width = tiles.size();
    size_t height = 0;
    if (!tiles.empty()) {
        height = tiles[0].size();
    }
    return LevelDimensions{width, height};
}

Camera& Level::getCamera() {
    return camera;
}

std::string_view Level::getName() const {
    return levelName;
}

size_t Level::getTileCount() const {
    return tileCount;
}

std::unique_ptr<Level> getTestLevel(AssetManager& assets, WindowManager& window) {
    auto level = std::make_unique<Level>("Test", LevelDimensions{100, 40}, window);
    level->showLevelBounds = true;
    b2WorldId world = level->getWorldId();
    const int GroundWidth = 50;
    const int GroundHeight = 2;
    const int WallHeight = 20;
    const int WallPosLeft = 0;
    const int WallPosRight = GroundWidth;
    level->addPlayer(assets, std::nullopt);
    level->addEntity(
        world,
        b2MakeBox(static_cast<float>(GroundWidth) / 2.f, static_cast<float>(GroundHeight) / 2.f),
        b2Vec2{static_cast<float>(GroundWidth) / 2.f, 1.f},
        true
    );
    level->addEntity(
        world,
        b2MakeBox(0.5f, static_cast<float>(WallHeight) / 2.f),
        b2Vec2{WallPosLeft + 0.5f, static_cast<float>(WallHeight) / 2.f},
        true
    );
    level->addEntity(
        world,
        b2MakeBox(0.5f, static_cast<float>(WallHeight) / 2.f),
        b2Vec2{WallPosRight + 0.5f, static_cast<float>(WallHeight) / 2.f},
        true
    );
    b2BodyDef dynamicBodyDef = b2DefaultBodyDef();
    dynamicBodyDef.type = b2_dynamicBody;
    level->addEntity(
        world,
        b2MakeBox(0.5f, 2.f),
        b2Vec2{28.f, 4.f},
        false,
        dynamicBodyDef,
        b2DefaultShapeDef(),
        colorToFColor(Colors.Yellow),
        assets.getTexture(GameAssets::Textures::Log),
        b2Vec2{1.f, 4.f}
    );
    level->addEntity(
        world,
        b2MakeBox(0.5f, 1.f),
        b2Vec2{8.f, 3.f},
        false,
        dynamicBodyDef,
        b2DefaultShapeDef(),
        colorToFColor(Colors.Yellow),
        assets.getTexture(GameAssets::Textures::Log),
        b2Vec2{1.f, 2.f}
    );
    for (int i = 1; i < GroundWidth; i++) {
        level->addTile(GameAssets::Textures::Dirt, i, 0);
        level->addTile(GameAssets::Textures::Grass, i, 1);
    }
    for (int i = 0; i < WallHeight; i++) {
        level->addTile(GameAssets::Textures::Stone, WallPosLeft, i);
        level->addTile(GameAssets::Textures::Stone, WallPosRight, i);
    }
    return level;
}