#pragma once

#include "../ecs/component.hpp"
#include <glm/glm.hpp>

namespace our {

    // Enemy AI states
    enum class EnemyState {
        IDLE,    // Standing still or patrolling
        CHASE,   // Following the player
        ATTACK   // Attacking the player
    };

    // This component adds AI behavior to an enemy entity
    class EnemyAIComponent : public Component {
    public:
        EnemyState currentState = EnemyState::IDLE;
        
        // Detection parameters
        float detectionRange = 10.0f;      // How far enemy can see player
        float loseTargetRange = 15.0f;     // Distance at which enemy stops chasing
        
        // Movement parameters
        float moveSpeed = 3.0f;            // Speed when chasing
        float rotationSpeed = 5.0f;        // How fast enemy rotates
        
        // Attack parameters
        float attackRange = 2.0f;          // Distance to start attacking
        float attackDamage = 10.0f;        // Damage per attack
        float attackCooldown = 1.0f;       // Time between attacks
        float attackTimer = 0.0f;          // Current attack cooldown
        
        // Idle/Patrol parameters
        float idleTime = 2.0f;             // How long to stay idle
        float idleTimer = 0.0f;            // Current idle time
        glm::vec3 patrolPoint = {0, 0, 0}; // Point to patrol to (if any)
        bool hasPatrolPoint = false;

        // The ID of this component type is "Enemy AI"
        static std::string getID() { return "Enemy AI"; }

        // Reads parameters from the given json object
        void deserialize(const nlohmann::json& data) override;
    };

}
