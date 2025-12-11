#include "collision.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our {
    // Reads collision parameters from the given json object
    void CollisionComponent::deserialize(const nlohmann::json& data) {
        if(!data.is_object()) return;
        radius = data.value("radius", radius);
        isStatic = data.value("isStatic", isStatic);
        canCollideWithWalls = data.value("canCollideWithWalls", canCollideWithWalls);
        canCollideWithEnemies = data.value("canCollideWithEnemies", canCollideWithEnemies);
        canCollideWithPlayer = data.value("canCollideWithPlayer", canCollideWithPlayer);
        offset = data.value("offset", offset);
    }
}
