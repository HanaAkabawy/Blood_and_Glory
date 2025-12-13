#pragma once

#include "../ecs/world.hpp"
#include "../components/player-controller.hpp"
#include "../components/camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <iostream>

namespace our {

    // The player controller system is responsible for moving entities with PlayerControllerComponent
    // Movement is relative to the camera's view direction (forward is where camera looks)
    class PlayerControllerSystem {
        Application* app; // Pointer to the application for input handling

    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application* app){
            this->app = app;
        }

        // This should be called every frame to update all entities containing a PlayerControllerComponent
        void update(World* world, float deltaTime) {
            // Safety checks
            if(!app || !world) {
                std::cout << "⚠️  Error: app or world is null in player controller!" << std::endl;
                return;
            }
            
            // Get keyboard and mouse input
            auto& keyboard = app->getKeyboard();
            auto& mouse = app->getMouse();

            // Find the main camera to get view direction
            CameraComponent* camera = nullptr;
            Entity* cameraEntity = nullptr;
            for(auto entity : world->getEntities()){
                camera = entity->getComponent<CameraComponent>();
                if(camera) {
                    cameraEntity = entity;
                    break;
                }
            }

            // If no camera exists, we can't determine view direction
            if(!camera || !cameraEntity) return;

            // Movement is in world space - no camera-relative movement needed
            // W/S = Forward/Backward on Z axis, A/D = Left/Right on X axis, E/Q = Up/Down on Y axis

            // Update all entities with PlayerControllerComponent
            try {
                for(auto entity : world->getEntities()){
                    if(!entity) continue; // Skip null entities
                    
                    PlayerControllerComponent* controller = entity->getComponent<PlayerControllerComponent>();
                    if(!controller) continue;

                // Debug: log SPACE key states and canJump to diagnose jump issues
                std::cout << "[DEBUG] SPACE isPressed=" << keyboard.isPressed(GLFW_KEY_SPACE)
                          << " justPressed=" << keyboard.justPressed(GLFW_KEY_SPACE)
                          << " canJump=" << controller->canJump << std::endl;

                // Handle jump input (SPACE key only). Accept initial press or held key as fallback.
                if((keyboard.justPressed(GLFW_KEY_SPACE) || keyboard.isPressed(GLFW_KEY_SPACE)) && controller->canJump) {
                    controller->verticalVelocity = controller->jumpForce;
                    controller->canJump = false;  // Prevent double jump
                    std::cout << "🦘 Player jumped! Velocity: " << controller->jumpForce << std::endl;
                }

                // Calculate movement direction based on WASD input (removed E/Q for vertical movement)
                glm::vec3 movementDirection(0.0f);
                bool keyPressed = false;

                // W/S = Forward/Backward (Z axis in world - toward/away from camera)
                if(keyboard.isPressed(GLFW_KEY_W)) { movementDirection.z -= 1.0f; keyPressed = true; } // Forward (negative Z)
                if(keyboard.isPressed(GLFW_KEY_S)) { movementDirection.z += 1.0f; keyPressed = true; } // Backward (positive Z)
                
                // A/D = Left/Right (X axis in world)
                if(keyboard.isPressed(GLFW_KEY_A)) { movementDirection.x -= 1.0f; keyPressed = true; } // Left (negative X)
                if(keyboard.isPressed(GLFW_KEY_D)) { movementDirection.x += 1.0f; keyPressed = true; } // Right (positive X)

                // Normalize movement direction if any key is pressed (only horizontal movement)
                if(glm::length(movementDirection) > 0.001f) {
                    movementDirection = glm::normalize(movementDirection);

                    // Calculate movement speed (with sprint)
                    float speed = controller->movementSpeed;
                    if(keyboard.isPressed(GLFW_KEY_LEFT_SHIFT)) {
                        speed *= controller->sprintMultiplier;
                    }

                    // Apply horizontal movement only (Y is handled by gravity/jump system)
                    glm::vec3 horizontalMovement = movementDirection * speed * deltaTime;
                    entity->localTransform.position.x += horizontalMovement.x;
                    entity->localTransform.position.z += horizontalMovement.z;

                    // Rotate player to face movement direction (only if not using mouse to rotate)
                    if(!mouse.isPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                        if(controller->smoothRotation) {
                            // Calculate target rotation (yaw only) - using XZ plane for ground movement
                            float targetYaw = atan2(movementDirection.x, movementDirection.z);
                            float currentYaw = entity->localTransform.rotation.y;

                            // Smoothly interpolate rotation
                            float yawDiff = targetYaw - currentYaw;
                            
                            // Normalize angle difference to [-PI, PI]
                            while(yawDiff > glm::pi<float>()) yawDiff -= 2.0f * glm::pi<float>();
                            while(yawDiff < -glm::pi<float>()) yawDiff += 2.0f * glm::pi<float>();

                            // Apply smooth rotation
                            float rotationAmount = controller->rotationSpeed * deltaTime;
                            if(abs(yawDiff) < rotationAmount) {
                                entity->localTransform.rotation.y = targetYaw;
                            } else {
                                entity->localTransform.rotation.y += glm::sign(yawDiff) * rotationAmount;
                            }
                        } else {
                            // Instant rotation - using XZ plane for ground movement
                            entity->localTransform.rotation.y = atan2(movementDirection.x, movementDirection.z);
                        }
                    }
                }
                }
            } catch(const std::exception& e) {
                std::cout << "⚠️  Error in player controller: " << e.what() << std::endl;
            } catch(...) {
                std::cout << "⚠️  Unknown error in player controller!" << std::endl;
            }
        }

        // When the state exits, cleanup if needed
        void exit(){
            // Nothing to cleanup for now
        }

    };

}
