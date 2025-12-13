#pragma once

#include "../ecs/world.hpp"
#include "../components/collision.hpp"
#include "../components/player-controller.hpp"
#include <glm/glm.hpp>

namespace our {

    // Simple gravity system to keep entities on the ground and handle jump physics
    class GravitySystem {
    private:
        float groundLevel = 0.0f;  // Y position of the ground surface
        float gravity = 30.0f;      // Gravity acceleration
        
    public:
        void setGroundLevel(float level) {
            groundLevel = level;
        }

        void update(World* world, float deltaTime) {
            if(!world) return;

            for(auto entity : world->getEntities()) {
                // Only apply gravity to entities with collision that are not static
                CollisionComponent* collision = entity->getComponent<CollisionComponent>();
                if(!collision || collision->isStatic) continue;

                // Check if this entity has a player controller (for jump physics)
                PlayerControllerComponent* playerController = entity->getComponent<PlayerControllerComponent>();
                
                // Get current position
                glm::vec3& pos = entity->localTransform.position;

                if(playerController) {
                    // Handle player jump physics with vertical velocity
                    
                    // Apply gravity to vertical velocity
                    playerController->verticalVelocity -= gravity * deltaTime;
                    
                    // Apply vertical velocity to position
                    pos.y += playerController->verticalVelocity * deltaTime;
                    
                    // Check if on ground
                    if(pos.y <= groundLevel) {
                        pos.y = groundLevel;
                        playerController->verticalVelocity = 0.0f;
                        playerController->canJump = true;  // Allow jumping again when on ground
                    } else {
                        playerController->canJump = false;  // Can't jump while in air
                    }
                } else {
                    // Non-player entities use simple gravity
                    if(pos.y < groundLevel) {
                        pos.y = groundLevel;
                    } else if(pos.y > groundLevel + 0.1f) {
                        // If above ground (with small threshold), apply gravity
                        pos.y -= gravity * deltaTime;
                        
                        // Clamp to ground level
                        if(pos.y < groundLevel) {
                            pos.y = groundLevel;
                        }
                    }
                }
            }
        }
    };

}
