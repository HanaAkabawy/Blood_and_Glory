#pragma once

#include "../ecs/world.hpp"
#include "../components/collision.hpp"
#include "../components/player-controller.hpp"
#include "../components/enemy-ai.hpp"
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <cfloat>

namespace our {

    // Simple gravity system: uses PlayerControllerComponent verticalVelocity for players
    // and a per-entity vertical velocity for other dynamic entities (enemies) so they
    // fall the same way without adding new components.
    class GravitySystem {
    private:
        float gravity = 30.0f; // Gravity acceleration
        std::unordered_map<Entity*, float> velocities; // per-entity vertical velocities
        float groundLevelOverride = -FLT_MAX; // optional override for ground top
    public:
        void setGravity(float g){ gravity = g; }
        void setGroundLevel(float level){ groundLevelOverride = level; }

        void update(World* world, float deltaTime) {
            if(!world) return;

            // Collect static supports (platforms/walls) with their top Y and horizontal extents
            struct Support { Entity* e; float topY; float hx; float hz; };
            std::vector<Support> supports;
            float groundSupportTop = -FLT_MAX;
            for(auto e : world->getEntities()){
                if(!e) continue;
                CollisionComponent* c = e->getComponent<CollisionComponent>();
                if(!c || !c->isStatic) continue;
                // Use collider offset and radius so thin platforms and offsets are handled consistently
                float top = e->localTransform.position.y + c->offset.y + c->radius;
                float hx = 0.5f * e->localTransform.scale.x + c->radius;
                float hz = 0.5f * e->localTransform.scale.z + c->radius;
                supports.push_back({e, top, hx, hz});
                // if this support is the named ground, remember it as a fallback
                if(e->name.find("ground") != std::string::npos || e->name.find("Ground") != std::string::npos){
                    groundSupportTop = top;
                }
            }
            // If an override was set via setGroundLevel, use it as the fallback ground
            if(groundLevelOverride > -FLT_MAX) groundSupportTop = groundLevelOverride;

            for(auto entity : world->getEntities()){
                if(!entity) continue;
                CollisionComponent* col = entity->getComponent<CollisionComponent>();
                if(!col || col->isStatic) continue;

                PlayerControllerComponent* pc = entity->getComponent<PlayerControllerComponent>();
                glm::vec3& pos = entity->localTransform.position;

                // Find the highest support top that is below or near the entity AND horizontally overlaps its footprint
                float bestTop = -FLT_MAX;
                for(auto &s : supports){
                    // horizontal overlap test in XZ between entity circle (radius) and support AABB
                    glm::vec2 entXZ(pos.x, pos.z);
                    glm::vec2 supportXZ(s.e->localTransform.position.x, s.e->localTransform.position.z);
                    float dx = std::abs(entXZ.x - supportXZ.x);
                    float dz = std::abs(entXZ.y - supportXZ.y);
                    bool overlap = (dx <= (s.hx + col->radius)) && (dz <= (s.hz + col->radius));
                    float supportTop = s.topY;
                    if(overlap && supportTop <= pos.y + 0.01f && supportTop > bestTop){
                        bestTop = supportTop;
                    }
                }

                // Determine effective floor: prefer a found support, otherwise fallback to groundSupportTop
                float effectiveTop = bestTop;
                if(entity->getComponent<EnemyAIComponent>()){
                    // For enemies, force them to use the global ground level so they line up with the player
                    effectiveTop = groundSupportTop;
                } else {
                    if(effectiveTop <= -FLT_MAX) effectiveTop = groundSupportTop;
                }

                if(pc){
                    if(std::abs(pc->verticalVelocity) > 0.0001f){
                        pc->verticalVelocity -= gravity * deltaTime;
                        pos.y += pc->verticalVelocity * deltaTime;
                        // never fall below the fallback ground
                        if(groundSupportTop > -FLT_MAX){
                            float minY = groundSupportTop + col->offset.y + col->radius;
                            if(pos.y < minY) {
                                pos.y = minY;
                                pc->verticalVelocity = 0.0f;
                                pc->canJump = true;
                            }
                        }
                    } else {
                        if(effectiveTop > -FLT_MAX) {
                            pos.y = effectiveTop + col->offset.y + col->radius;
                            pc->canJump = true;
                        }
                    }
                } else if(entity->getComponent<EnemyAIComponent>()){
                    // Give enemies per-entity gravity similar to the player
                    float &vel = velocities[entity];
                    if(std::abs(vel) > 0.0001f){
                        vel -= gravity * deltaTime;
                        pos.y += vel * deltaTime;
                        if(groundSupportTop > -FLT_MAX){
                            float minY = groundSupportTop + col->offset.y + col->radius;
                            if(pos.y < minY){
                                pos.y = minY;
                                vel = 0.0f;
                            }
                        }
                    } else {
                        if(effectiveTop > -FLT_MAX) pos.y = effectiveTop + col->offset.y + col->radius;
                    }
                } else {
                    if(effectiveTop > -FLT_MAX) pos.y = effectiveTop + col->offset.y + col->radius;
                    velocities[entity] = 0.0f;
                }
            }
        }
    };

}
