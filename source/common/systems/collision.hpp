#pragma once

#include "../ecs/world.hpp"
#include "../components/collision.hpp"

#include <glm/glm.hpp>
#include <vector>
#include <cmath>
#include "../components/player-controller.hpp"
#include "../components/enemy-ai.hpp"

namespace our {

    // The collision system handles sphere-sphere collision detection and response
    class CollisionSystem {
    public:
        // Structure to hold collision information
        struct CollisionInfo {
            Entity* entityA;
            Entity* entityB;
            glm::vec3 penetrationVector; // Vector to separate the entities
            float penetrationDepth;
        };

        // Check if two spheres are colliding
        static bool checkSphereCollision(
            const glm::vec3& posA, float radiusA,
            const glm::vec3& posB, float radiusB,
            glm::vec3& penetrationVector, float& penetrationDepth)
        {
            // Use vector from A to B as penetration direction
            glm::vec3 diff = posB - posA;
            float distance = glm::length(diff);
            float minDistance = radiusA + radiusB;

            if(distance < minDistance) {
                penetrationDepth = minDistance - distance;
                if(distance > 0.0001f) {
                    penetrationVector = glm::normalize(diff) * penetrationDepth; // from A to B
                } else {
                    penetrationVector = glm::vec3(1, 0, 0) * penetrationDepth;
                }
                return true;
            }
            return false;
        }

        // Update collision detection and response for all entities
        void update(World* world, float deltaTime) {
            auto& entities = world->getEntities();

            // Convert to vector for indexed access
            std::vector<Entity*> entityList(entities.begin(), entities.end());

            // Precompute static supports for snapping
            struct Support { Entity* e; float topY; float hx; float hz; };
            std::vector<Support> supports;
            for(auto e : entityList){
                if(!e) continue;
                CollisionComponent* c = e->getComponent<CollisionComponent>();
                if(!c || !c->isStatic) continue;
                float top = e->localTransform.position.y + 0.5f * e->localTransform.scale.y;
                float hx = 0.5f * e->localTransform.scale.x;
                float hz = 0.5f * e->localTransform.scale.z;
                supports.push_back({e, top, hx, hz});
            }

            // Run multiple resolution passes to avoid tunneling & stacked penetrations
            const int passes = 6;
            for(int pass = 0; pass < passes; ++pass){
                for(size_t i = 0; i < entityList.size(); ++i) {
                    Entity* entityA = entityList[i];
                    CollisionComponent* collisionA = entityA->getComponent<CollisionComponent>();
                    if(!collisionA) continue;

                    // Get world position and add offset (without scaling the offset)
                    glm::vec3 posA = entityA->localTransform.position + collisionA->offset;

                    for(size_t j = i + 1; j < entityList.size(); ++j) {
                        Entity* entityB = entityList[j];
                        CollisionComponent* collisionB = entityB->getComponent<CollisionComponent>();
                        if(!collisionB) continue;

                        // Check if these entities should collide based on their settings
                        if(!shouldCollide(collisionA, collisionB, entityA, entityB)) continue;

                        // Get world position and add offset (without scaling the offset)
                        glm::vec3 posB = entityB->localTransform.position + collisionB->offset;

                        // Check collision
                        glm::vec3 penetrationVector;
                        float penetrationDepth;
                        if(checkSphereCollision(posA, collisionA->radius, posB, collisionB->radius, 
                                               penetrationVector, penetrationDepth)) {
                            resolveCollision(entityA, entityB, collisionA, collisionB, penetrationVector, penetrationDepth);
                        }
                    }
                }
            }

            // After resolution passes, align dynamic entities vertically to the highest overlapping support
                for(auto entity : entityList){
                if(!entity) continue;
                CollisionComponent* col = entity->getComponent<CollisionComponent>();
                if(!col || col->isStatic) continue;

                glm::vec3& pos = entity->localTransform.position;

                float bestTop = -FLT_MAX;
                for(auto &s : supports){
                    glm::vec2 entXZ(pos.x, pos.z);
                    glm::vec2 supportXZ(s.e->localTransform.position.x, s.e->localTransform.position.z);
                    float dx = std::abs(entXZ.x - supportXZ.x);
                    float dz = std::abs(entXZ.y - supportXZ.y);
                    bool overlap = (dx <= (s.hx + col->radius)) && (dz <= (s.hz + col->radius));
                    if(overlap && s.topY > bestTop){
                        bestTop = s.topY;
                    }
                }

                if(bestTop > -FLT_MAX){
                    // Match gravity: entity position (base) = support top + offset.y + radius
                    pos.y = bestTop + col->offset.y + col->radius;
                }
            }
        }

    private:
        // Determine if two entities should collide based on their tags/types
        bool shouldCollide(CollisionComponent* a, CollisionComponent* b, Entity* entityA, Entity* entityB) {
            // If either is static and the other doesn't collide with walls, skip
            if(a->isStatic && !b->canCollideWithWalls) return false;
            if(b->isStatic && !a->canCollideWithWalls) return false;

            // Both static objects don't collide with each other
            if(a->isStatic && b->isStatic) return false;

            return true;
        }

        // Resolve collision by pushing entities apart
        void resolveCollision(Entity* entityA, Entity* entityB, 
                            CollisionComponent* collisionA, CollisionComponent* collisionB,
                            const glm::vec3& penetrationVector, float penetrationDepth) {
            // If both are static, do nothing
            if(collisionA->isStatic && collisionB->isStatic) return;

            // Decide whether to resolve vertically (Y) or horizontally (XZ)
            glm::vec3 pen = penetrationVector;
            glm::vec3 penH = glm::vec3(pen.x, 0.0f, pen.z);
            float hLen = glm::length(penH);
            float vLen = std::abs(pen.y);

            glm::vec3 correction;
            if(hLen > vLen && hLen > 0.0001f) {
                // Use full penetration depth along horizontal direction
                correction = glm::normalize(penH) * penetrationDepth;
            } else {
                // Use full penetration depth along vertical direction (preserve sign)
                correction = glm::vec3(0.0f, (pen.y >= 0.0f ? 1.0f : -1.0f) * penetrationDepth, 0.0f);
            }

            // If A is static, move B by the full correction
            if(collisionA->isStatic) {
                entityB->localTransform.position += correction;
                return;
            }

            // If B is static, move A by the negative of the correction
            if(collisionB->isStatic) {
                entityA->localTransform.position -= correction;
                return;
            }

            // Both dynamic: split correction
            entityA->localTransform.position -= correction * 0.5f;
            entityB->localTransform.position += correction * 0.5f;
        }
    };

}
