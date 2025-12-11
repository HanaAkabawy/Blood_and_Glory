#include "orbit-camera-controller.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our {
    // Reads orbit camera controller parameters from the given json object
    void OrbitCameraControllerComponent::deserialize(const nlohmann::json& data) {
        if(!data.is_object()) return;
        distance = data.value("distance", distance);
        minDistance = data.value("minDistance", minDistance);
        maxDistance = data.value("maxDistance", maxDistance);
        zoomSensitivity = data.value("zoomSensitivity", zoomSensitivity);
        orbitSensitivity = data.value("orbitSensitivity", orbitSensitivity);
        yaw = data.value("yaw", yaw);
        pitch = data.value("pitch", pitch);
        minPitch = data.value("minPitch", minPitch);
        maxPitch = data.value("maxPitch", maxPitch);
        offset = data.value("offset", offset);
        enableMouseOrbit = data.value("enableMouseOrbit", enableMouseOrbit);
    }
}
