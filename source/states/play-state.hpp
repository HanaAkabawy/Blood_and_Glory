#pragma once

#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/orbit-camera-controller.hpp>
#include <systems/player-controller.hpp>
#include <systems/collision.hpp>
#include <systems/combat.hpp>
#include <systems/enemy-ai.hpp>
#include <systems/movement.hpp>
#include <asset-loader.hpp>

// This state shows how to use the ECS framework and deserialization.
class Playstate: public our::State {

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::OrbitCameraControllerSystem orbitCameraController;
    our::PlayerControllerSystem playerController;
    our::CollisionSystem collisionSystem;
    our::CombatSystem combatSystem;
    our::EnemyAISystem enemyAISystem;
    our::MovementSystem movementSystem;
    
    our::Entity* player = nullptr;  // Reference to the player entity

    void onInitialize() override {
        // First of all, we get the scene configuration from the app config
        auto& config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if(config.contains("assets")){
            our::deserializeAllAssets(config["assets"]);
        }
        // If we have a world in the scene config, we use it to populate our world
        if(config.contains("world")){
            world.deserialize(config["world"]);
        }
        
        // Find the player entity (entity with a PlayerControllerComponent)
        for(auto entity : world.getEntities()){
            if(entity->getComponent<our::PlayerControllerComponent>()){
                player = entity;
                break;
            }
        }
        
        // We initialize the camera controller systems since they need a pointer to the app
        cameraController.enter(getApp());
        orbitCameraController.enter(getApp());
        playerController.enter(getApp());
        
        // Initialize combat system with player reference
        if(player) {
            combatSystem.enter(getApp(), player, 3.0f, 25.0f, 0.5f);
            
            // Initialize enemy AI system with player reference
            enemyAISystem.setPlayer(player);
            
            // Set the orbit camera target to the player if orbit camera exists
            for(auto entity : world.getEntities()){
                auto orbitCam = entity->getComponent<our::OrbitCameraControllerComponent>();
                if(orbitCam){
                    orbitCam->target = player;
                    std::cout << "✅ Orbit camera target set to player" << std::endl;
                    break;
                }
            }
        } else {
            std::cout << "⚠️  Warning: No player entity found!" << std::endl;
        }
        
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
    }

    void onDraw(double deltaTime) override {
        // Here, we just run a bunch of systems to control the world logic
        
        // Movement and control systems
        movementSystem.update(&world, (float)deltaTime);
        playerController.update(&world, (float)deltaTime);
        
        // Apply collision resolution after player movement
        collisionSystem.update(&world, (float)deltaTime);
        
        // Enemy AI (they also move)
        enemyAISystem.update(&world, (float)deltaTime);
        
        // Apply collision resolution after enemy movement
        collisionSystem.update(&world, (float)deltaTime);
        
        // Camera systems (use only one active camera type)
        cameraController.update(&world, (float)deltaTime);
        orbitCameraController.update(&world, (float)deltaTime);
        
        // Combat system
        combatSystem.update(&world, (float)deltaTime);
        
        // And finally we use the renderer system to draw the scene
        renderer.render(&world);

        // Get a reference to the keyboard object
        auto& keyboard = getApp()->getKeyboard();

        if(keyboard.justPressed(GLFW_KEY_ESCAPE)){
            // If the escape key is pressed in this frame, go to the menu state
            getApp()->changeState("menu");
        }
    }

    void onDestroy() override {
        // Don't forget to destroy the renderer
        renderer.destroy();
        // On exit, we call exit for the camera controller system to make sure that the mouse is unlocked
        cameraController.exit();
        orbitCameraController.exit();
        playerController.exit();
        // Clear the world
        world.clear();
        // and we delete all the loaded assets to free memory on the RAM and the VRAM
        our::clearAllAssets();
    }
};