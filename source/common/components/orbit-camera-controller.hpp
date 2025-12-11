#pragma once

#include "../ecs/component.hpp"
#include "../ecs/entity.hpp"

#include <glm/glm.hpp>

namespace our {

    // This component denotes that the OrbitCameraControllerSystem will make the camera orbit around a target entity
    // It supports zooming in/out and rotating around the target
    class OrbitCameraControllerComponent : public Component {
    public:
        Entity* target = nullptr;          // The entity to orbit around (usually the player)
        float distance = 10.0f;             // Distance from the target
        float minDistance = 2.0f;           // Minimum zoom distance
        float maxDistance = 20.0f;          // Maximum zoom distance
        float zoomSensitivity = 1.0f;       // How fast to zoom in/out
        float orbitSensitivity = 0.01f;     // How fast to orbit (rotation sensitivity)
        float yaw = 0.0f;                   // Horizontal rotation angle
        float pitch = 30.0f;                // Vertical rotation angle (in degrees)
        float minPitch = -89.0f;            // Minimum pitch angle
        float maxPitch = 89.0f;             // Maximum pitch angle
        glm::vec3 offset = {0, 0, 0};      // Offset from target position
        bool enableMouseOrbit = true;       // Enable mouse dragging to orbit

        // The ID of this component type is "Orbit Camera Controller"
        static std::string getID() { return "Orbit Camera Controller"; }

        // Reads parameters from the given json object
        void deserialize(const nlohmann::json& data) override;
    };

}
