#include "enemy-ai.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our {
    // Reads enemy AI parameters from the given json object
    void EnemyAIComponent::deserialize(const nlohmann::json& data) {
        if(!data.is_object()) return;
        
        detectionRange = data.value("detectionRange", detectionRange);
        loseTargetRange = data.value("loseTargetRange", loseTargetRange);
        moveSpeed = data.value("moveSpeed", moveSpeed);
        rotationSpeed = data.value("rotationSpeed", rotationSpeed);
        attackRange = data.value("attackRange", attackRange);
        attackDamage = data.value("attackDamage", attackDamage);
        attackCooldown = data.value("attackCooldown", attackCooldown);
        idleTime = data.value("idleTime", idleTime);
        
        if(data.contains("patrolPoint")) {
            patrolPoint = data.value("patrolPoint", patrolPoint);
            hasPatrolPoint = true;
        }
        
        // Set initial state
        std::string stateStr = data.value("initialState", "IDLE");
        if(stateStr == "CHASE") currentState = EnemyState::CHASE;
        else if(stateStr == "ATTACK") currentState = EnemyState::ATTACK;
        else currentState = EnemyState::IDLE;
    }
}
