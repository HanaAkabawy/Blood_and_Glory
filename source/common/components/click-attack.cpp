#include "click-attack.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our {
    
    // Reads attack parameters from the given json object
    void ClickAttackComponent::deserialize(const nlohmann::json& data) {
        if(!data.is_object()) return;
        
        // Deserialize attack properties
        damage = data.value("damage", damage);
        range = data.value("range", range);
        attackCooldown = data.value("attackCooldown", attackCooldown);

        // attackTimer starts at 0.0 by default, no need to deserialize
    }
}