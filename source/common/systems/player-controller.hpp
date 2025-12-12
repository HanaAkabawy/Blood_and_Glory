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

            // Get camera's forward and right vectors
            glm::mat4 cameraTransform = cameraEntity->getLocalToWorldMatrix();
            glm::vec3 cameraForward = glm::normalize(glm::vec3(cameraTransform * glm::vec4(0, 0, -1, 0)));
            glm::vec3 cameraRight = glm::normalize(glm::vec3(cameraTransform * glm::vec4(1, 0, 0, 0)));
            
            // Project forward and right onto XY plane (ignore Z component)
            cameraForward.z = 0;
            cameraRight.z = 0;
            
            // Normalize again after projection, or use default axes if projection resulted in zero
            if(glm::length(cameraForward) > 0.001f) {
                cameraForward = glm::normalize(cameraForward);
            } else {
                // If camera is looking straight at XY plane, use Y as forward
                cameraForward = glm::vec3(0, 1, 0);
            }
            
            if(glm::length(cameraRight) > 0.001f) {
                cameraRight = glm::normalize(cameraRight);
            } else {
                // If camera right is perpendicular to XY plane, use X as right
                cameraRight = glm::vec3(1, 0, 0);
            }
            
            // Debug output to see what's happening
            static int debugCounter = 0;
            if(debugCounter++ % 60 == 0) { // Print once per second (assuming 60fps)
                std::cout << "Camera forward: " << cameraForward.x << ", " << cameraForward.y << ", " << cameraForward.z << std::endl;
                std::cout << "Camera right: " << cameraRight.x << ", " << cameraRight.y << ", " << cameraRight.z << std::endl;
            }

            // Update all entities with PlayerControllerComponent
            try {
                for(auto entity : world->getEntities()){
                    if(!entity) continue; // Skip null entities
                    
                    PlayerControllerComponent* controller = entity->getComponent<PlayerControllerComponent>();
                    if(!controller) continue;

                // Handle mouse left-click to set facing direction toward mouse cursor
                // (Simplified approach without ray-casting for stability)
                // Player will just face the direction they're moving when not using mouse

                // Calculate movement direction based on WASD + EQ input
                glm::vec3 movementDirection(0.0f);
                bool keyPressed = false;

                // W/S = Forward/Backward (Z axis in world - toward/away from camera)
                if(keyboard.isPressed(GLFW_KEY_W)) { movementDirection.z -= 1.0f; keyPressed = true; } // Forward (negative Z)
                if(keyboard.isPressed(GLFW_KEY_S)) { movementDirection.z += 1.0f; keyPressed = true; } // Backward (positive Z)
                
                // A/D = Left/Right (X axis in world)
                if(keyboard.isPressed(GLFW_KEY_A)) { movementDirection.x -= 1.0f; keyPressed = true; } // Left (negative X)
                if(keyboard.isPressed(GLFW_KEY_D)) { movementDirection.x += 1.0f; keyPressed = true; } // Right (positive X)
                
                // E/Q = Up/Down (Y axis in world)
                if(keyboard.isPressed(GLFW_KEY_E)) { movementDirection.y += 1.0f; keyPressed = true; } // Up (positive Y)
                if(keyboard.isPressed(GLFW_KEY_Q)) { movementDirection.y -= 1.0f; keyPressed = true; } // Down (negative Y)

                if(keyPressed) {
                    std::cout << "🎮 Key pressed! Movement direction: " 
                              << movementDirection.x << ", " << movementDirection.y << ", " << movementDirection.z << std::endl;
                }

                // Normalize movement direction if any key is pressed
                if(glm::length(movementDirection) > 0.001f) {
                    movementDirection = glm::normalize(movementDirection);

                    // Calculate movement speed (with sprint)
                    float speed = controller->movementSpeed;
                    if(keyboard.isPressed(GLFW_KEY_LEFT_SHIFT)) {
                        speed *= controller->sprintMultiplier;
                    }

                    // Apply movement in world space (no camera-relative movement)
                    entity->localTransform.position += movementDirection * speed * deltaTime;
                    
                    // Debug: Show position
                    static int posCounter = 0;
                    if(posCounter++ % 30 == 0) { // Every half second
                        std::cout << "📍 Player position: (" << entity->localTransform.position.x 
                                  << ", " << entity->localTransform.position.y 
                                  << ", " << entity->localTransform.position.z << ")" << std::endl;
                    }

                    // Rotate player to face movement direction (only if not using mouse to rotate)
                    if(!mouse.isPressed(GLFW_MOUSE_BUTTON_LEFT)) {
                        if(controller->smoothRotation) {
                            // Calculate target rotation (yaw only) - using Y axis since we're in XY plane
                            float targetYaw = atan2(movementDirection.x, movementDirection.y);
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
                            // Instant rotation - using Y axis since we're in XY plane
                            entity->localTransform.rotation.y = atan2(movementDirection.x, movementDirection.y);
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
