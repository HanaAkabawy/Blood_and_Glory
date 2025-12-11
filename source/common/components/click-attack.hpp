#pragma once

#include "../ecs/component.hpp"
#include <glm/glm.hpp>

namespace our {

    // This component holds the parameters for an entity that performs a proximity attack
    // when triggered by player input (e.g., mouse click).
    class ClickAttackComponent : public Component {
    public:
        // Parameters read from JSON
        float damage = 10.0f;       // Base damage dealt by the attack
        float range = 2.0f;         // Max distance to target to successfully hit
        float attackCooldown = 0.5f;// Time between successful attacks

        // Runtime State
        float attackTimer = 0.0f;   // Current time remaining until the next attack can occur

        // The ID of this component type is "Click Attack" (must match the JSON string)
        static std::string getID() { return "Click Attack"; }

        // Reads parameters from the given json object
        void deserialize(const nlohmann::json& data) override;
    };

}