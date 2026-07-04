#include "Level.hpp"
#include "Drawing.hpp"

Level::Level(const char* levelName, LevelDimensions size, WindowManager& window)
    : camera(nullptr, window), levelName(levelName), size(size) {
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
    for (size_t x = 0; x < tiles.size(); x++) {
        for (size_t y = 0; y < tiles[x].size(); y++) {
            if (tiles[x][y] != GameAssets::Textures::None) {
                drawTile(tiles[x][y], x, y, assets, window, camera.getScaleFactor());
            }
        }
    }
    for (const auto& entity : entities) {
        if (entity) {
            entity->draw(window, alpha, camera.getScaleFactor(), camera.getOffsetPixels());
            if (showFanTriangulation) {
                b2Transform transform;
                transform.p = entity->getInterpolatedPosition(alpha);
                transform.q = entity->getInterpolatedRotation(alpha);
                Drawing::showFanTriangulation(
                    entity->getPolygon(),
                    window,
                    transform,
                    camera.getScaleFactor(),
                    camera.getOffsetPixels()
                );
            }
            if (showHitBoxes) {
                entity->drawHitbox(
                    window, alpha, camera.getScaleFactor(), camera.getOffsetPixels()
                );
            }
        }
    }
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

void Level::addEntity(std::unique_ptr<Entity> entity) {
    entities.push_back(std::move(entity));
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
}

void Level::drawTile(
    GameAssets::Textures textureId,
    size_t x,
    size_t y,
    AssetManager& assets,
    WindowManager& window,
    float scaleFactor
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
        scaleFactor,
        camera.getOffsetPixels()
    );
}

void Level::addPlayer(std::unique_ptr<EntityController> player) {
    players.push_back(std::move(player));
}

const std::vector<std::unique_ptr<Entity>>& Level::getEntities() const {
    return entities;
}
const LevelTileVector& Level::getTiles() const {
    return tiles;
}
const std::vector<std::unique_ptr<EntityController>>& Level::getPlayers() const {
    return players;
}

b2WorldId Level::getWorldId() const {
    return world;
}

std::unique_ptr<Level> getTemplateLevel(AssetManager& assets, WindowManager& window) {
    auto level = std::make_unique<Level>("Template", LevelDimensions{100, 40}, window);
    b2BodyDef playerBodyDef = b2DefaultBodyDef();
    playerBodyDef.fixedRotation = true;
    b2ShapeDef playerShapeDef = b2DefaultShapeDef();
    playerShapeDef.material.friction = 0.f;
    playerShapeDef.density = 4.f;
    b2WorldId world = level->getWorldId();
    auto playerEntity = std::make_unique<Entity>(
        world,
        b2MakeBox(0.5f, 1.f),
        b2Vec2{10.f, 4.f},
        false,
        assets.getTexture(GameAssets::Textures::Player),
        b2Vec2{1.f, 2.f},
        playerBodyDef,
        playerShapeDef
    );
    level->camera.entityToFollow = playerEntity.get();
    auto controller = std::make_unique<EntityController>(*playerEntity);
    controller->spawnPoint = b2Vec2{4.f, 4.f};
    level->addPlayer(std::move(controller));
    level->addEntity(std::move(playerEntity));
    const int GroundWidth = 50;
    const int GroundHeight = 2;
    level->addEntity(
        std::make_unique<Entity>(
            world,
            b2MakeBox(static_cast<float>(GroundWidth) / 2.f, static_cast<float>(GroundHeight)),
            b2Vec2{static_cast<float>(GroundWidth) / 2.f, 0.f},
            true,
            nullptr
        )
    );
    const int WallHeight = 20;
    const int WallPosLeft = 0;
    const int WallPosRight = GroundWidth;
    level->addEntity(
        std::make_unique<Entity>(
            world,
            b2MakeBox(0.5f, static_cast<float>(WallHeight) / 2.f),
            b2Vec2{WallPosLeft + 0.5f, static_cast<float>(WallHeight) / 2.f},
            true,
            nullptr
        )
    );
    level->addEntity(
        std::make_unique<Entity>(
            world,
            b2MakeBox(0.5f, static_cast<float>(WallHeight) / 2.f),
            b2Vec2{WallPosRight + 0.5f, static_cast<float>(WallHeight) / 2.f},
            true,
            nullptr
        )
    );
    b2BodyDef dynamicBodyDef = b2DefaultBodyDef();
    dynamicBodyDef.type = b2_dynamicBody;
    level->addEntity(
        std::make_unique<Entity>(
            world,
            b2MakeBox(0.5f, 2.f),
            b2Vec2{28.f, 4.f},
            false,
            assets.getTexture(GameAssets::Textures::Log),
            b2Vec2{1.f, 4.f},
            dynamicBodyDef,
            b2DefaultShapeDef()
        )
    );
    level->addEntity(
        std::make_unique<Entity>(
            world,
            b2MakeBox(0.5f, 1.f),
            b2Vec2{8.f, 3.f},
            false,
            assets.getTexture(GameAssets::Textures::Log),
            b2Vec2{1.f, 2.f},
            dynamicBodyDef,
            b2DefaultShapeDef()
        )
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