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
            
            // Get keyboard input
            auto& keyboard = app->getKeyboard();

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

                // Calculate movement direction based on WASD input (relative to camera)
                glm::vec3 movementDirection(0.0f);
                bool keyPressed = false;

                if(keyboard.isPressed(GLFW_KEY_W)) { movementDirection += cameraForward; keyPressed = true; }
                if(keyboard.isPressed(GLFW_KEY_S)) { movementDirection -= cameraForward; keyPressed = true; }
                if(keyboard.isPressed(GLFW_KEY_D)) { movementDirection += cameraRight; keyPressed = true; }
                if(keyboard.isPressed(GLFW_KEY_A)) { movementDirection -= cameraRight; keyPressed = true; }

                if(keyPressed) {
                    std::cout << "🎮 Key pressed! Movement direction before normalize: " 
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

                    // Apply movement (keeping Z constant for ground-based movement)
                    float groundDepth = entity->localTransform.position.z;
                    glm::vec3 oldPos = entity->localTransform.position;
                    entity->localTransform.position += movementDirection * speed * deltaTime;
                    entity->localTransform.position.z = groundDepth; // Lock to Z plane
                    
                    // Debug: Show position
                    static int posCounter = 0;
                    if(posCounter++ % 30 == 0) { // Every half second
                        std::cout << "📍 Player position: (" << entity->localTransform.position.x 
                                  << ", " << entity->localTransform.position.y 
                                  << ", " << entity->localTransform.position.z << ")" << std::endl;
                    }

                    // Rotate player to face movement direction
                    if(controller->smoothRotation) {
                        // Calculate target rotation (yaw only)
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
                        // Instant rotation
                        entity->localTransform.rotation.y = atan2(movementDirection.x, movementDirection.z);
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
