#pragma once

#include "../ecs/world.hpp"
#include "../components/collision.hpp"

#include <glm/glm.hpp>
#include <vector>

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
            glm::vec3 diff = posA - posB;
            float distance = glm::length(diff);
            float minDistance = radiusA + radiusB;

            if(distance < minDistance) {
                // Collision detected
                penetrationDepth = minDistance - distance;
                if(distance > 0.0001f) {
                    penetrationVector = glm::normalize(diff) * penetrationDepth;
                } else {
                    // Objects are at same position, push apart in arbitrary direction
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
            
            // Check all pairs of entities with collision components
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
                        // Resolve collision
                        resolveCollision(entityA, entityB, collisionA, collisionB, penetrationVector);
                    }
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
                            const glm::vec3& penetrationVector) {
            // If both are static, do nothing
            if(collisionA->isStatic && collisionB->isStatic) return;

            // If A is static, only move B
            if(collisionA->isStatic) {
                entityB->localTransform.position -= penetrationVector;
            }
            // If B is static, only move A
            else if(collisionB->isStatic) {
                entityA->localTransform.position += penetrationVector;
            }
            // If both are dynamic, push both apart
            else {
                entityA->localTransform.position += penetrationVector * 0.5f;
                entityB->localTransform.position -= penetrationVector * 0.5f;
            }
        }
    };

}
