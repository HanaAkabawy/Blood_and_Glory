#pragma once

#include "../ecs/world.hpp"
#include "../components/health.hpp"
#include "../components/camera.hpp"
#include "../components/collision.hpp"

#include <glm/glm.hpp>
#include <iostream>
#include <limits>

namespace our {

    // The combat system handles click-to-attack mechanics
    class CombatSystem {
        Application* app;
        Entity* player = nullptr;  // Reference to the player entity
        float attackRange = 3.0f;   // How close you need to be to attack
        float attackDamage = 25.0f; // Damage per attack
        float attackCooldown = 0.5f; // Time between attacks
        float cooldownTimer = 0.0f;  // Current cooldown

    public:
        // Initialize the combat system
        void enter(Application* app, Entity* playerEntity = nullptr, 
                   float range = 3.0f, float damage = 25.0f, float cooldown = 0.5f) {
            this->app = app;
            this->player = playerEntity;
            this->attackRange = range;
            this->attackDamage = damage;
            this->attackCooldown = cooldown;
            this->cooldownTimer = 0.0f;
        }

        // Set the player entity (can be called after enter if player isn't known yet)
        void setPlayer(Entity* playerEntity) {
            this->player = playerEntity;
        }

        // Update combat system
        void update(World* world, float deltaTime) {
            // Safety check
            if(!player || !world || !app) {
                return; // Skip if player doesn't exist
            }
            
            // Update cooldown timer
            if(cooldownTimer > 0.0f) {
                cooldownTimer -= deltaTime;
            }

            // Check for attack input (left mouse button)
            auto& mouse = app->getMouse();
            auto& keyboard = app->getKeyboard();
            // Forced attack: press 'K' to perform an immediate attack (useful for testing)
            if(keyboard.justPressed(GLFW_KEY_K)){
                std::cout << "[Combat] Key K pressed - forced attack." << std::endl;
                performAttack(world);
            }
            if(mouse.justPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                std::cout << "[Combat] Mouse click detected." << std::endl;
                // If attackCooldown <= 0, allow immediate attacks with no cooldown enforcement
                if(attackCooldown <= 0.0f){
                    performAttack(world);
                } else {
                    // Normal cooldown behavior
                    std::cout << "[Combat] Cooldown remaining: " << cooldownTimer << "s" << std::endl;
                    if(cooldownTimer <= 0.0f) {
                        performAttack(world);
                        cooldownTimer = attackCooldown;
                    } else {
                        std::cout << "[Combat] Attack on cooldown. Wait " << cooldownTimer << "s" << std::endl;
                    }
                }
            }

            // Clean up dead entities
            cleanupDeadEntities(world);
        }

        // Perform an attack from the player
        void performAttack(World* world) {
            if(!player) return;

            // Get player position
            glm::vec3 playerPos = player->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1);

            // Find the closest enemy within attack range (allow tiny epsilon for floating error)
            Entity* closestEnemy = nullptr;
            float closestDistance = std::numeric_limits<float>::infinity();

            for(auto entity : world->getEntities()) {
                // Skip if it's the player
                if(entity == player) continue;

                // Check if entity has health (is attackable)
                HealthComponent* health = entity->getComponent<HealthComponent>();
                if(!health || !health->isAlive) continue;

                // Calculate distance (only in XY plane, ignore Z since everyone is at Z=0)
                glm::vec3 entityPos = entity->getLocalToWorldMatrix() * glm::vec4(0, 0, 0, 1);
                glm::vec2 playerPos2D = glm::vec2(playerPos.x, playerPos.y);
                glm::vec2 entityPos2D = glm::vec2(entityPos.x, entityPos.y);
                float distance = glm::length(entityPos2D - playerPos2D);

                // Debug output
                std::cout << "Distance to enemy: " << distance << " units" << std::endl;

                // Check if in range (allow tiny epsilon) and closer than previous closest
                const float eps = 1e-3f;
                if(distance <= attackRange + eps && distance < closestDistance) {
                    closestDistance = distance;
                    closestEnemy = entity;
                }
            }

            // If we found an enemy in range, attack it
            if(!closestEnemy){
                // No in-range target: fallback to the nearest alive entity (helpful for testing)
                Entity* nearest = nullptr;
                float nearestDist = std::numeric_limits<float>::infinity();
                for(auto entity : world->getEntities()){
                    if(entity == player) continue;
                    HealthComponent* h = entity->getComponent<HealthComponent>();
                    if(!h || !h->isAlive) continue;
                    glm::vec3 entityPos = entity->getLocalToWorldMatrix() * glm::vec4(0,0,0,1);
                    float d = glm::length(glm::vec2(entityPos.x, entityPos.y) - glm::vec2(playerPos.x, playerPos.y));
                    if(d < nearestDist){ nearestDist = d; nearest = entity; }
                }
                if(nearest){
                    std::cout << "[Combat] No in-range target; falling back to nearest alive entity '" << nearest->name << "' at distance " << nearestDist << std::endl;
                    closestEnemy = nearest;
                    closestDistance = nearestDist;
                }
            }

            if(closestEnemy) {
                HealthComponent* health = closestEnemy->getComponent<HealthComponent>();
                if(health) {
                    float prev = health->currentHealth;
                    std::cout << "[Combat] Attacking entity '" << closestEnemy->name << "' at distance " << closestDistance << " (HP " << prev << ") with " << attackDamage << " dmg" << std::endl;
                    health->takeDamage(attackDamage);
                    std::cout << "💥 ATTACK! Enemy health: " << health->currentHealth << "/" << health->maxHealth << std::endl;
                    if(!health->isAlive) {
                        std::cout << "☠️  Enemy DEFEATED! Marking for removal." << std::endl;
                    }
                }
            } else {
                std::cout << "❌ No enemy in range (need to be within " << attackRange << " units)" << std::endl;
            }

            // After attempting an attack, print health of all entities with a Health component
            std::cout << "[Combat] Current health of entities:" << std::endl;
            for(auto e : world->getEntities()){
                if(auto h = e->getComponent<HealthComponent>()){
                    std::cout << "  - '" << e->name << "' : " << h->currentHealth << " / " << h->maxHealth << (h->isAlive ? " (alive)" : " (dead)") << std::endl;
                }
            }
        }

        // Clean up entities that have died
        void cleanupDeadEntities(World* world) {
            auto entities = world->getEntities();
            for(auto it = entities.begin(); it != entities.end(); ) {
                Entity* entity = *it;
                HealthComponent* health = entity->getComponent<HealthComponent>();
                
                if(health && !health->isAlive && health->destroyOnDeath) {
                    // Mark entity for deletion
                    world->markForRemoval(entity);
                    ++it;
                } else {
                    ++it;
                }
            }
            
            // Actually remove marked entities
            world->deleteMarkedEntities();
        }

        // Get current cooldown progress (0 = ready, 1 = just attacked)
        float getCooldownProgress() const {
            if(attackCooldown <= 0.0f) return 0.0f;
            return glm::clamp(cooldownTimer / attackCooldown, 0.0f, 1.0f);
        }

        // Check if can attack
        bool canAttack() const {
            return cooldownTimer <= 0.0f;
        }
    };

}
