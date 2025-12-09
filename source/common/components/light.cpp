#include "light.hpp"

#include "../deserialize-utils.hpp"

namespace our {
    void LightComponent::deserialize(const nlohmann::json& data){
        color = data.value("color", glm::vec3(1.0f, 1.0f,1.0f));
        std::string typeStr = data.value("lightType", "directional");
        if(typeStr == "directional"){
            lightType = LightType::DIRECTIONAL;
        } else if(typeStr =="point")
        {
            lightType = LightType::POINT;
        } else if (typeStr == "spot"){
            lightType = LightType::SPOT;
        }
        // 3. Read attenuation (default: no falloff for directional)
        attenuation = data.value("attenuation", glm::vec3(1.0f,0.0f,0.0f));
         // 4. Read cone angles (default: 45° inner, 60° outer)
        cone = data.value("cone", glm::vec2(0.78f, 1.05f));
    };
}