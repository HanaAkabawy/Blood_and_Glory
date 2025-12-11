#pragma once

#include "../ecs/world.hpp"
#include "../components/camera.hpp"
#include "../components/orbit-camera-controller.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <iostream>

namespace our {

    // The orbit camera controller system is responsible for making the camera orbit around a target entity
    // This system handles zooming and rotation around the target
    class OrbitCameraControllerSystem {
        Application* app; // Pointer to the application for input handling
        bool mouse_locked = false; // Is the mouse locked for camera control?

    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application* app){
            this->app = app;
        }

        // This should be called every frame to update all entities containing an OrbitCameraControllerComponent
        void update(World* world, float deltaTime) {
            // First, we search for an entity containing both a CameraComponent and an OrbitCameraControllerComponent
            CameraComponent* camera = nullptr;
            OrbitCameraControllerComponent* controller = nullptr;
            for(auto entity : world->getEntities()){
                camera = entity->getComponent<CameraComponent>();
                controller = entity->getComponent<OrbitCameraControllerComponent>();
                if(camera && controller) break;
            }
            // If no entity with both components exists, we can do nothing
            if(!(camera && controller)) return;

            // Get the entity that owns the camera
            Entity* entity = camera->getOwner();
            if(!entity) return;

            // If there's no target to orbit around, do nothing
            if(!controller->target) {
                // Reset mouse lock if we lost the target
                if(mouse_locked) {
                    mouse_locked = false;
                    our::Mouse::unlockMouse(app->getWindow());
                }
                return;
            }

            // Get the target's world position
            glm::vec3 targetPos = controller->target->getLocalToWorldMatrix() * glm::vec4(controller->offset, 1.0f);

            // Handle mouse input for orbiting
            auto& mouse = app->getMouse();
            
            // Safety check for window and application
            GLFWwindow* window = app->getWindow();
            if(!window) {
                std::cout << "⚠️  Error: Window is null in orbit camera!" << std::endl;
                return;
            }
            
            // Check if right mouse button is held down
            bool rightMouseHeld = false;
            try {
                rightMouseHeld = mouse.isPressed(GLFW_MOUSE_BUTTON_RIGHT);
            } catch(...) {
                std::cout << "⚠️  Error: Mouse input failed!" << std::endl;
                return;
            }
            
            // Update mouse lock state based on right mouse button
            if(rightMouseHeld && !mouse_locked) {
                mouse_locked = true;
                try {
                    our::Mouse::lockMouse(window);
                    std::cout << "🖱️  Mouse locked for camera control" << std::endl;
                } catch(...) {
                    std::cout << "⚠️  Error: Could not lock mouse!" << std::endl;
                    mouse_locked = false;
                }
            } else if(!rightMouseHeld && mouse_locked) {
                mouse_locked = false;
                try {
                    our::Mouse::unlockMouse(window);
                    std::cout << "🖱️  Mouse unlocked" << std::endl;
                } catch(...) {
                    std::cout << "⚠️  Error: Could not unlock mouse!" << std::endl;
                }
            }

            // If mouse is locked and orbit is enabled, update rotation
            if(mouse_locked && controller->enableMouseOrbit) {
                glm::vec2 delta = mouse.getMouseDelta();
                controller->yaw -= delta.x * controller->orbitSensitivity;
                controller->pitch -= delta.y * controller->orbitSensitivity;

                // Clamp pitch to avoid gimbal lock
                controller->pitch = glm::clamp(controller->pitch, controller->minPitch, controller->maxPitch);
            }

            // Handle zoom with mouse scroll
            float scrollOffset = mouse.getScrollOffset().y;
            if(scrollOffset != 0.0f) {
                controller->distance -= scrollOffset * controller->zoomSensitivity;
                controller->distance = glm::clamp(controller->distance, controller->minDistance, controller->maxDistance);
            }

            // Calculate camera position based on spherical coordinates
            float pitchRad = glm::radians(controller->pitch);
            float yawRad = glm::radians(controller->yaw);

            // Calculate offset from target
            glm::vec3 offset;
            offset.x = controller->distance * cos(pitchRad) * sin(yawRad);
            offset.y = controller->distance * sin(pitchRad);
            offset.z = controller->distance * cos(pitchRad) * cos(yawRad);

            // Set camera position
            glm::vec3 cameraPos = targetPos + offset;
            entity->localTransform.position = cameraPos;

            // Make camera look at target
            glm::vec3 direction = glm::normalize(targetPos - cameraPos);
            glm::vec3 right = glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0)));
            glm::vec3 up = glm::cross(right, direction);

            // Build rotation matrix
            glm::mat4 rotationMatrix = glm::mat4(1.0f);
            rotationMatrix[0] = glm::vec4(right, 0.0f);
            rotationMatrix[1] = glm::vec4(up, 0.0f);
            rotationMatrix[2] = glm::vec4(-direction, 0.0f);

            // Extract euler angles from rotation matrix (approximate)
            entity->localTransform.rotation = glm::vec3(
                glm::atan(-rotationMatrix[2][1], rotationMatrix[2][2]),
                glm::atan(rotationMatrix[2][0], glm::sqrt(rotationMatrix[2][1] * rotationMatrix[2][1] + rotationMatrix[2][2] * rotationMatrix[2][2])),
                glm::atan(-rotationMatrix[1][0], rotationMatrix[0][0])
            );
        }

        // When the state exits, it should call this function to ensure the mouse is unlocked
        void exit(){
            if(mouse_locked) {
                mouse_locked = false;
                our::Mouse::unlockMouse(app->getWindow());
            }
        }

    };

}
