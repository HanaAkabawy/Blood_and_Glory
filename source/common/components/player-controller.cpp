#include "player-controller.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our {
    // Reads player controller parameters from the given json object
    void PlayerControllerComponent::deserialize(const nlohmann::json& data) {
        if(!data.is_object()) return;
        movementSpeed = data.value("movementSpeed", movementSpeed);
        rotationSpeed = data.value("rotationSpeed", rotationSpeed);
        sprintMultiplier = data.value("sprintMultiplier", sprintMultiplier);
        smoothRotation = data.value("smoothRotation", smoothRotation);
        jumpForce = data.value("jumpForce", jumpForce);
    }
}
