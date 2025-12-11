#pragma once

#include "../ecs/world.hpp"
#include "../components/enemy-ai.hpp"
#include "../components/health.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace our {

    // The enemy AI system manages enemy behavior using a state machine
    class EnemyAISystem {
        Entity* player = nullptr;  // Reference to the player entity

    public:
        // Set the player entity for enemies to track
        void setPlayer(Entity* playerEntity) {
            this->player = playerEntity;
        }

        // Update all enemy AI
        void update(World* world, float deltaTime) {
            if(!player || !world) return; // Safety check
            
            try {
                // Get player position
                glm::vec3 playerPos = player->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1);
            
            // Check if player is alive
            HealthComponent* playerHealth = player->getComponent<HealthComponent>();
            bool playerAlive = (!playerHealth || playerHealth->isAlive);

            // Update each enemy
            for(auto entity : world->getEntities()) {
                EnemyAIComponent* ai = entity->getComponent<EnemyAIComponent>();
                if(!ai) continue;

                // Check if enemy is alive
                HealthComponent* health = entity->getComponent<HealthComponent>();
                if(health && !health->isAlive) continue;

                // Get enemy position and calculate 2D distance (XY plane only)
                glm::vec3 enemyPos = entity->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1);
                glm::vec2 playerPos2D = glm::vec2(playerPos.x, playerPos.y);
                glm::vec2 enemyPos2D = glm::vec2(enemyPos.x, enemyPos.y);
                float distanceToPlayer = glm::length(playerPos2D - enemyPos2D);

                // State machine
                switch(ai->currentState) {
                    case EnemyState::IDLE:
                        updateIdleState(entity, ai, enemyPos, playerPos, distanceToPlayer, playerAlive, deltaTime);
                        break;
                    
                    case EnemyState::CHASE:
                        updateChaseState(entity, ai, enemyPos, playerPos, distanceToPlayer, playerAlive, deltaTime);
                        break;
                    
                    case EnemyState::ATTACK:
                        updateAttackState(entity, ai, enemyPos, playerPos, distanceToPlayer, playerAlive, deltaTime);
                        break;
                }
            }
            } catch(...) {
                std::cout << "⚠️  Error in enemy AI system!" << std::endl;
            }
        }

    private:
        // IDLE state: Wait or patrol
        void updateIdleState(Entity* entity, EnemyAIComponent* ai, 
                            const glm::vec3& enemyPos, const glm::vec3& playerPos, 
                            float distanceToPlayer, bool playerAlive, float deltaTime) {
            // Check if player is in detection range
            if(playerAlive && distanceToPlayer < ai->detectionRange) {
                ai->currentState = EnemyState::CHASE;
                return;
            }

            // Update idle timer
            ai->idleTimer += deltaTime;
            
            // Simple idle behavior - could add patrol logic here
            if(ai->hasPatrolPoint && ai->idleTimer > ai->idleTime) {
                // TODO: Add patrol movement if needed
                ai->idleTimer = 0.0f;
            }
        }

        // CHASE state: Follow the player
        void updateChaseState(Entity* entity, EnemyAIComponent* ai, 
                             const glm::vec3& enemyPos, const glm::vec3& playerPos, 
                             float distanceToPlayer, bool playerAlive, float deltaTime) {
            // If player is dead or too far, return to idle
            if(!playerAlive || distanceToPlayer > ai->loseTargetRange) {
                ai->currentState = EnemyState::IDLE;
                ai->idleTimer = 0.0f;
                return;
            }

            // If close enough, switch to attack
            if(distanceToPlayer < ai->attackRange) {
                ai->currentState = EnemyState::ATTACK;
                ai->attackTimer = 0.0f;
                return;
            }

            // Move towards player
            glm::vec3 direction = glm::normalize(playerPos - enemyPos);
            direction.z = 0; // Keep on same Z level
            
            if(glm::length(direction) > 0.001f) {
                direction = glm::normalize(direction);
                
                // Store current Z position to maintain ground depth
                float groundDepth = entity->localTransform.position.z;
                
                // Move towards player
                entity->localTransform.position += direction * ai->moveSpeed * deltaTime;
                
                // Lock Z position to ground plane
                entity->localTransform.position.z = groundDepth;
                
                // Rotate to face player
                float targetYaw = atan2(direction.x, direction.y);
                entity->localTransform.rotation.z = targetYaw;
            }
        }

        // ATTACK state: Attack the player
        void updateAttackState(Entity* entity, EnemyAIComponent* ai, 
                              const glm::vec3& enemyPos, const glm::vec3& playerPos, 
                              float distanceToPlayer, bool playerAlive, float deltaTime) {
            // If player is dead, return to idle
            if(!playerAlive) {
                ai->currentState = EnemyState::IDLE;
                ai->idleTimer = 0.0f;
                return;
            }

            // If player moved out of attack range, chase again
            if(distanceToPlayer > ai->attackRange * 1.2f) { // 1.2x for hysteresis
                ai->currentState = EnemyState::CHASE;
                return;
            }

            // If player is too far, return to idle
            if(distanceToPlayer > ai->loseTargetRange) {
                ai->currentState = EnemyState::IDLE;
                ai->idleTimer = 0.0f;
                return;
            }

            // Face the player
            glm::vec3 direction = glm::normalize(playerPos - enemyPos);
            direction.z = 0;
            if(glm::length(direction) > 0.001f) {
                float targetYaw = atan2(direction.x, direction.y);
                entity->localTransform.rotation.z = targetYaw;
            }

            // Update attack timer
            ai->attackTimer -= deltaTime;
            
            // Perform attack if cooldown is ready
            if(ai->attackTimer <= 0.0f) {
                performAttack(entity, ai);
                ai->attackTimer = ai->attackCooldown;
            }
        }

        // Perform an attack on the player
        void performAttack(Entity* enemy, EnemyAIComponent* ai) {
            if(!player) return;

            HealthComponent* playerHealth = player->getComponent<HealthComponent>();
            if(playerHealth && playerHealth->isAlive) {
                playerHealth->takeDamage(ai->attackDamage);
                // Optional: Add attack animation or sound here
            }
        }
    };

}
