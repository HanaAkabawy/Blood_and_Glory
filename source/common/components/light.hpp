#pragma once

#include "../ecs/component.hpp"
#include <glm/glm.hpp>

namespace our {
enum class LightType{
    DIRECTIONAL,
    POINT,
    SPOT
};

class LightComponent: public Component {
    public:
    LightType lightType;
    glm::vec3 color;
    glm::vec3 attenuation;
    glm::vec2 cone;

    static std::string getID() {
        return "Light";
    }
    void deserialize(const nlohmann::json& data) override;
};

}