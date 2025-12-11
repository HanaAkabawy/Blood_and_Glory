#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp>

namespace our {

    // This component adds collision detection to an entity using a sphere collider
    class CollisionComponent : public Component {
    public:
        float radius = 1.0f;                    // Collision sphere radius
        bool isStatic = false;                  // Static objects don't move when collided
        bool canCollideWithWalls = true;        // Can collide with static objects
        bool canCollideWithEnemies = true;      // Can collide with enemies
        bool canCollideWithPlayer = true;       // Can collide with player
        glm::vec3 offset = {0, 0, 0};          // Offset from entity position

        // The ID of this component type is "Collision"
        static std::string getID() { return "Collision"; }

        // Reads parameters from the given json object
        void deserialize(const nlohmann::json& data) override;
    };

}
