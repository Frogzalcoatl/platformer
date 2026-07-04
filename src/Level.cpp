#include "Level.hpp"
#include "Drawing.hpp"

Level::Level(const char* levelName, WindowManager& window)
    : camera(nullptr, window), levelName(levelName) {
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = {0.0f, -60.f};
    world = b2CreateWorld(&worldDef);
    SDL_Log("Created box2d world for level \"%s\"", levelName);
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

void Level::draw(WindowManager& window, float alpha, bool showFanTriangulation) {
    camera.run(alpha);
    for (const auto& tile : tiles) {
        if (tile) {
            tile->draw(window, camera.getScaleFactor(), camera.getOffsetPixels());
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

void Level::addTile(std::unique_ptr<Tile> tile) {
    tiles.push_back(std::move(tile));
}

void Level::addPlayer(std::unique_ptr<EntityController> player) {
    players.push_back(std::move(player));
}

const std::vector<std::unique_ptr<Entity>>& Level::getEntities() const {
    return entities;
}
const std::vector<std::unique_ptr<Tile>>& Level::getTiles() const {
    return tiles;
}
const std::vector<std::unique_ptr<EntityController>>& Level::getPlayers() const {
    return players;
}

b2WorldId Level::getWorldId() const {
    return world;
}

std::unique_ptr<Level> getTemplateLevel(AssetManager& assets, WindowManager& window) {
    auto level = std::make_unique<Level>("Template", window);
    b2BodyDef playerBodyDef = b2DefaultBodyDef();
    playerBodyDef.fixedRotation = true;
    b2ShapeDef playerShapeDef = b2DefaultShapeDef();
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
        level->addTile(
            std::make_unique<Tile>(
                Vec2Int{static_cast<int>(i), 1}, assets, GameAssets::Textures::Grass
            )
        );
        level->addTile(
            std::make_unique<Tile>(
                Vec2Int{static_cast<int>(i), 0}, assets, GameAssets::Textures::Dirt
            )
        );
    }
    for (int i = 0; i < WallHeight; i++) {
        level->addTile(
            std::make_unique<Tile>(
                Vec2Int{WallPosLeft, static_cast<int>(i)}, assets, GameAssets::Textures::Stone
            )
        );
        level->addTile(
            std::make_unique<Tile>(
                Vec2Int{WallPosRight, static_cast<int>(i)}, assets, GameAssets::Textures::Stone
            )
        );
    }
    return level;
}