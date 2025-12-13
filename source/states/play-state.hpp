#pragma once

#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/orbit-camera-controller.hpp>
#include <systems/player-controller.hpp>
#include <systems/collision.hpp>
#include <systems/gravity.hpp>
#include <systems/combat.hpp>
#include <systems/enemy-ai.hpp>
#include <systems/movement.hpp>
#include <asset-loader.hpp>
#include <components/click-attack.hpp>
#include <components/health.hpp>
#include <components/enemy-ai.hpp>

#include <imgui.h>

#include <vector>
#include <string>

// This state shows how to use the ECS framework and deserialization.
class Playstate: public our::State {

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::OrbitCameraControllerSystem orbitCameraController;
    our::PlayerControllerSystem playerController;
    our::CollisionSystem collisionSystem;
    our::GravitySystem gravitySystem;
    our::CombatSystem combatSystem;
    our::EnemyAISystem enemyAISystem;
    our::MovementSystem movementSystem;
    
    our::Entity* player = nullptr;  // Reference to the player entity
    int postprocessIndex = 0;
    std::vector<std::string> postprocessShaders;
    // Victory tracking: when true and timer expires we return to menu
    bool victory = false;
    float victoryTimer = 0.0f;

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
            // If the player has a ClickAttackComponent, use its parameters to initialize the combat system
            float attackRange = 3.0f;
            float attackDamage = 25.0f;
            float attackCooldown = 0.5f;
            if(auto click = player->getComponent<our::ClickAttackComponent>()){
                attackRange = click->range;
                attackDamage = click->damage;
                attackCooldown = click->attackCooldown;
            }
            combatSystem.enter(getApp(), player, attackRange, attackDamage, attackCooldown);
            
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
        
        // Set gravity ground level (ground plane is at Y=-1.5, characters stand at Y=0)
        gravitySystem.setGroundLevel(0.0f);
        
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
        // Setup postprocess shader cycling (T key)
        postprocessShaders = {"", "assets/shaders/postprocess/sepia.frag", "assets/shaders/postprocess/vignette.frag"};
        std::string initial = config["renderer"].value<std::string>("postprocess", "");
        postprocessIndex = 0;
        for(size_t i = 0; i < postprocessShaders.size(); ++i) if(postprocessShaders[i] == initial) postprocessIndex = (int)i;
        renderer.setPostprocessShader(postprocessShaders[postprocessIndex]);
        // Reset victory state when entering play so replay works correctly
        victory = false;
        victoryTimer = 0.0f;
    }

    void onImmediateGui() override {
        // Draw health UI using ImGui
        drawHealthUI();
        // Victory popup
        if(victory){
            ImGui::SetNextWindowPos(ImVec2(400, 100), ImGuiCond_Always);
            ImGui::Begin("Victory", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
            ImGui::Text(" You Wooooooooon!");
            ImGui::Text("Returning to menu in %.1f seconds", victoryTimer);
            ImGui::End();
        }
    }

    void onDraw(double deltaTime) override {
        // Check if player is dead
        if(player) {
            our::HealthComponent* playerHealth = player->getComponent<our::HealthComponent>();
            if(playerHealth && !playerHealth->isAlive) {
                std::cout << "💀 GAME OVER - Player is dead!" << std::endl;
                getApp()->changeState("menu");
                return;
            }
        }
        
        // Here, we just run a bunch of systems to control the world logic
        
        // Movement and control systems
        movementSystem.update(&world, (float)deltaTime);
        playerController.update(&world, (float)deltaTime);
        
        // Apply gravity to keep entities on ground
        gravitySystem.update(&world, (float)deltaTime);
        
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

        // Check win condition: if player is alive and all enemies are dead announce victory
        if(!victory){
            int aliveEnemies = 0;
            for(auto entity : world.getEntities()){
                auto enemyAI = entity->getComponent<our::EnemyAIComponent>();
                if(enemyAI){
                    auto h = entity->getComponent<our::HealthComponent>();
                    if(h && h->isAlive) aliveEnemies++;
                }
            }
            auto playerHealth = player ? player->getComponent<our::HealthComponent>() : nullptr;
            if(playerHealth && playerHealth->isAlive && aliveEnemies == 0){
                std::cout << "🎉 Player wins! Returning to menu..." << std::endl;
                victory = true;
                victoryTimer = 2.0f; // seconds before returning to menu
            }
        } else {
            // Countdown and return to menu when timer expires
            victoryTimer -= (float)deltaTime;
            if(victoryTimer <= 0.0f){
                getApp()->changeState("menu");
                return;
            }
        }

        // Get a reference to the keyboard object
        auto& keyboard = getApp()->getKeyboard();

        if(keyboard.justPressed(GLFW_KEY_ESCAPE)){
            // If the escape key is pressed in this frame, go to the menu state
            getApp()->changeState("menu");
        }

        // Cycle postprocessing effects with T
        if(keyboard.justPressed(GLFW_KEY_T)){
            postprocessIndex = (postprocessIndex + 1) % (int)postprocessShaders.size();
            renderer.setPostprocessShader(postprocessShaders[postprocessIndex]);
            std::cout << "[Renderer] Postprocess set to: " << (postprocessShaders[postprocessIndex].empty() ? "OFF" : postprocessShaders[postprocessIndex]) << std::endl;
        }
    }

    void drawHealthUI() {
        // Set up ImGui window for health display
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Always);
        ImGui::Begin("Health Status", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        // Display player health
        if (player) {
            auto playerHealth = player->getComponent<our::HealthComponent>();
            if (playerHealth) {
                ImGui::Text("PLAYER HEALTH");
                float healthPercent = playerHealth->currentHealth / playerHealth->maxHealth;
                
                // Color the bar based on health percentage
                ImVec4 barColor;
                if (healthPercent > 0.6f) {
                    barColor = ImVec4(0.0f, 0.8f, 0.0f, 1.0f); // Green
                } else if (healthPercent > 0.3f) {
                    barColor = ImVec4(0.9f, 0.7f, 0.0f, 1.0f); // Yellow
                } else {
                    barColor = ImVec4(0.9f, 0.0f, 0.0f, 1.0f); // Red
                }
                
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
                ImGui::ProgressBar(healthPercent, ImVec2(0.0f, 0.0f), "");
                ImGui::PopStyleColor();
                ImGui::SameLine(0.0f, 10.0f);
                ImGui::Text("%.0f / %.0f", playerHealth->currentHealth, playerHealth->maxHealth);
                
                if (!playerHealth->isAlive) {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "PLAYER DEFEATED!");
                }
            }
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Display enemy health
        ImGui::Text("ENEMIES");
        int enemyCount = 0;
        for (auto entity : world.getEntities()) {
            auto enemyAI = entity->getComponent<our::EnemyAIComponent>();
            if (enemyAI) {
                auto enemyHealth = entity->getComponent<our::HealthComponent>();
                if (enemyHealth && enemyHealth->isAlive) {
                    enemyCount++;
                    ImGui::PushID(entity);
                    
                    ImGui::Text("Enemy #%d", enemyCount);
                    float healthPercent = enemyHealth->currentHealth / enemyHealth->maxHealth;
                    
                    // Color the bar based on health percentage
                    ImVec4 barColor;
                    if (healthPercent > 0.6f) {
                        barColor = ImVec4(0.8f, 0.0f, 0.0f, 1.0f); // Red for enemies
                    } else if (healthPercent > 0.3f) {
                        barColor = ImVec4(0.9f, 0.3f, 0.0f, 1.0f); // Orange
                    } else {
                        barColor = ImVec4(0.5f, 0.0f, 0.0f, 1.0f); // Dark red
                    }
                    
                    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
                    ImGui::ProgressBar(healthPercent, ImVec2(0.0f, 0.0f), "");
                    ImGui::PopStyleColor();
                    ImGui::SameLine(0.0f, 10.0f);
                    ImGui::Text("%.0f / %.0f", enemyHealth->currentHealth, enemyHealth->maxHealth);
                    
                    // Show enemy state
                    const char* stateText = "IDLE";
                    switch(enemyAI->currentState) {
                        case our::EnemyState::IDLE: stateText = "IDLE"; break;
                        case our::EnemyState::CHASE: stateText = "CHASING"; break;
                        case our::EnemyState::ATTACK: stateText = "ATTACKING"; break;
                        case our::EnemyState::STUNNED: stateText = "STUNNED"; break;
                    }
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "[%s]", stateText);
                    
                    ImGui::PopID();
                    ImGui::Spacing();
                }
            }
        }

        if (enemyCount == 0) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No enemies alive");
        }

        ImGui::End();
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