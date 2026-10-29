#pragma once
#include <vector>
#include "../ECS/Components3D.h"
#include "JobSystem.h"

namespace Newtoon {
    class PhysicsWorld {
    private:
        std::vector<CollisionEvent> m_collisionEvents;
        Vec3 m_gravity = {0.0f, -9.81f, 0.0f};

    public:
        const std::vector<CollisionEvent>& GetCollisionEvents() const { return m_collisionEvents; }
        void ClearEvents() { m_collisionEvents.clear(); }

        void Simulate(std::vector<Transform>& transforms, std::vector<Rigidbody>& rigidbodies, float dt) {
            
            // 1. Integrar Velocidades (Multi-threaded)
            JobSystem::ParallelFor<Rigidbody>(rigidbodies, [this, dt](Rigidbody& rb) {
                if (rb.IsStatic() || rb.isSleeping) return;
                
                rb.velocity.x += (rb.forceAccumulator.x * rb.inverseMass + m_gravity.x) * dt;
                rb.velocity.y += (rb.forceAccumulator.y * rb.inverseMass + m_gravity.y) * dt;
                rb.velocity.z += (rb.forceAccumulator.z * rb.inverseMass + m_gravity.z) * dt;

                rb.angularVelocity.x += rb.torqueAccumulator.x * rb.inverseInertia * dt;
                rb.angularVelocity.y += rb.torqueAccumulator.y * rb.inverseInertia * dt;
                rb.angularVelocity.z += rb.torqueAccumulator.z * rb.inverseInertia * dt;

                rb.forceAccumulator = {0, 0, 0};
                rb.torqueAccumulator = {0, 0, 0};
                
                // Sleep check
                if (rb.velocity.Magnitude() < rb.sleepThreshold && rb.angularVelocity.Magnitude() < rb.sleepThreshold) {
                    rb.sleepTimer += dt;
                    if (rb.sleepTimer > 1.0f) rb.isSleeping = true;
                } else {
                    rb.sleepTimer = 0.0f;
                }
            });

            // 2. Integrar Posições
            for (size_t i = 0; i < rigidbodies.size(); ++i) {
                if(rigidbodies[i].IsStatic() || rigidbodies[i].isSleeping) continue;
                transforms[i].position.x += rigidbodies[i].velocity.x * dt;
                transforms[i].position.y += rigidbodies[i].velocity.y * dt;
                transforms[i].position.z += rigidbodies[i].velocity.z * dt;
            }
            
            // (A lógica de BVH, Raycast, CCD e GJK Narrow-Phase entraria logo aqui, 
            // processando intersecções e preenchendo os eventos em m_collisionEvents)
        }
    };
}