#include "health.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our {
    // Reads health parameters from the given json object
    void HealthComponent::deserialize(const nlohmann::json& data) {
        if(!data.is_object()) return;
        maxHealth = data.value("maxHealth", maxHealth);
        currentHealth = data.value("currentHealth", maxHealth); // Default to max if not specified
        isAlive = data.value("isAlive", isAlive);
        destroyOnDeath = data.value("destroyOnDeath", destroyOnDeath);
    }
}
