// components/enemy-ai.cpp

#include "enemy-ai.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp" 
#include <iostream>

namespace our {

    // Reads enemy AI parameters from the given json object
    void EnemyAIComponent::deserialize(const nlohmann::json& data) {
        if(!data.is_object()) return;
        
        // Read Detection and Movement Parameters
        detectionRange = data.value("detectionRange", detectionRange);
        loseTargetRange = data.value("loseTargetRange", loseTargetRange);
        moveSpeed = data.value("moveSpeed", moveSpeed);
        rotationSpeed = data.value("rotationSpeed", rotationSpeed);
        
        // Read Attack Parameters
        attackRange = data.value("attackRange", attackRange);
        attackDamage = data.value("attackDamage", attackDamage);
        attackCooldown = data.value("attackCooldown", attackCooldown);
        
        // Read Idle/Patrol Parameters
        idleTime = data.value("idleTime", idleTime);
        
        if(data.contains("patrolPoint")) {
             patrolPoint = data.value("patrolPoint", patrolPoint);
             hasPatrolPoint = true;
        }
        
        // Read Control Parameters (NEW)
        canAct = data.value("canAct", canAct);
        stunDuration = data.value("stunDuration", stunDuration);
        
        // Set initial state based on JSON string
        std::string stateStr = data.value("initialState", "IDLE");
        if(stateStr == "CHASE") currentState = EnemyState::CHASE;
        else if(stateStr == "ATTACK") currentState = EnemyState::ATTACK;
        else if(stateStr == "STUNNED") currentState = EnemyState::STUNNED;
        else currentState = EnemyState::IDLE; // Handles "IDLE" or any unrecognized string
    }
}