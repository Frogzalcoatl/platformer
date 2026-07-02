#pragma once
#include "Entity.hpp"
#include "EntityController.hpp"
#include "Tile.hpp"
#include <box2d/box2d.h>

class Level {
  private:
    std::vector<std::unique_ptr<Entity>> entities;
    std::vector<std::unique_ptr<Tile>> tiles;
    std::vector<std::unique_ptr<EntityController>> players;
    b2WorldId world;

    uint64_t currentTime = 0;
    uint64_t lastTime = 0;
    float accumulator = 0.f;
    const float physicsStep = 1.0f / 60.0f;

  public:
    const char* levelName;
    Level(const char* levelName);
    ~Level();
    // Returns alpha
    float update();
    void handleInput(GameEventTypes::Input event, Camera* camera);
    void draw(WindowManager& window, float alpha, bool showFanTriangulation = false);
    void addEntity(std::unique_ptr<Entity> entity);
    void addTile(std::unique_ptr<Tile> tile);
    void addPlayer(std::unique_ptr<EntityController> player);
    const std::vector<std::unique_ptr<Entity>>& getEntities() const;
    const std::vector<std::unique_ptr<Tile>>& getTiles() const;
    const std::vector<std::unique_ptr<EntityController>>& getPlayers() const;
    b2WorldId getWorldId() const;
};

std::unique_ptr<Level> getTemplateLevel(AssetManager& assets);