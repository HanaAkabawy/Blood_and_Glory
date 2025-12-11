#pragma once

#include "../ecs/component.hpp"
#include <glm/glm.hpp>
#include <algorithm>

namespace our {

    // This component adds health and damage properties to an entity
    class HealthComponent : public Component {
    public:
        float maxHealth = 100.0f;
        float currentHealth = 100.0f;
        bool isAlive = true;
        bool destroyOnDeath = true;  // Automatically destroy entity when health reaches 0

        // The ID of this component type is "Health"
        static std::string getID() { return "Health"; }

        // Apply damage to this entity
        void takeDamage(float damage) {
            if(!isAlive) return;
            currentHealth -= damage;
            if(currentHealth <= 0) {
                currentHealth = 0;
                isAlive = false;
            }
        }

        // Heal this entity
        void heal(float amount) {
            if(!isAlive) return;
            currentHealth = std::min(currentHealth + amount, maxHealth);
        }

        // Reads parameters from the given json object
        void deserialize(const nlohmann::json& data) override;
    };

}
