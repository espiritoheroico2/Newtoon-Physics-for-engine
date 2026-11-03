#pragma once
#include <vector>
#include "../ECS/Components2D.h"
#include "JobSystem.h"

namespace Newtoon {
    class PhysicsWorld2D {
    private:
        Vec2 m_gravity = {0.0f, -9.81f};

    public:
        void Simulate(std::vector<Transform2D>& transforms, std::vector<Rigidbody2D>& rigidbodies, float dt) {
            
            // 1. Integrar Velocidades 2D (Multi-threaded)
            JobSystem::ParallelFor<Rigidbody2D>(rigidbodies, [this, dt](Rigidbody2D& rb) {
                if (rb.IsStatic() || rb.isSleeping) return;
                
                rb.velocity.x += (rb.forceAccumulator.x * rb.inverseMass + m_gravity.x) * dt;
                rb.velocity.y += (rb.forceAccumulator.y * rb.inverseMass + m_gravity.y) * dt;

                rb.angularVelocity += rb.torqueAccumulator * rb.inverseInertia * dt;

                rb.forceAccumulator = {0, 0};
                rb.torqueAccumulator = 0.0f;
            });

            // 2. Integrar Posições 2D
            for (size_t i = 0; i < rigidbodies.size(); ++i) {
                if(rigidbodies[i].IsStatic() || rigidbodies[i].isSleeping) continue;
                transforms[i].position.x += rigidbodies[i].velocity.x * dt;
                transforms[i].position.y += rigidbodies[i].velocity.y * dt;
                transforms[i].rotation += rigidbodies[i].angularVelocity * dt;
            }
        }
    };
}