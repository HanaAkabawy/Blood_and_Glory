#pragma once

#include "../ecs/component.hpp"

#include <glm/glm.hpp>

namespace our {

    // This component denotes that the PlayerControllerSystem will move the owning entity
    // based on WASD input relative to the camera's view direction
    class PlayerControllerComponent : public Component {
    public:
        float movementSpeed = 5.0f;        // Units per second
        float rotationSpeed = 10.0f;       // How fast the player rotates to face movement direction
        float sprintMultiplier = 2.0f;     // Speed multiplier when shift is held
        bool smoothRotation = true;        // Enable smooth rotation towards movement direction

        // The ID of this component type is "Player Controller"
        static std::string getID() { return "Player Controller"; }

        // Reads parameters from the given json object
        void deserialize(const nlohmann::json& data) override;
    };

}
