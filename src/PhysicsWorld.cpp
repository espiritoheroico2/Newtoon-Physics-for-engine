#pragma once
#include <vector>
#include "../ECS/Components3D.h"
#include "JobSystem.h"
#include "CollisionDetector.h"
#include "PhysicsSolver.h"

namespace Newtoon {
    class PhysicsWorld {
    private:
        std::vector<CollisionEvent> m_collisionEvents;
        Vec3 m_gravity = {0.0f, -9.81f, 0.0f};

    public:
        const std::vector<CollisionEvent>& GetCollisionEvents() const { return m_collisionEvents; }
        void ClearEvents() { m_collisionEvents.clear(); }

        void Simulate(std::vector<Transform>& transforms, std::vector<Rigidbody>& rigidbodies, const std::vector<Collider>& colliders, float dt) {
            
            // 1. Integrar Velocidades (JobSystem mantido do original)
            JobSystem::ParallelFor<Rigidbody>(rigidbodies, [this, dt](Rigidbody& rb) {
                if (rb.IsStatic() || rb.isSleeping) return;
                
                rb.velocity = rb.velocity + (rb.forceAccumulator * rb.inverseMass + m_gravity) * dt;
                rb.forceAccumulator = {0, 0, 0};
                // Angular omitido por espaço, mas mesmo princípio
            });

            // 2. Integrar Posições
            for (size_t i = 0; i < rigidbodies.size(); ++i) {
                if(rigidbodies[i].IsStatic() || rigidbodies[i].isSleeping) continue;
                transforms[i].position = transforms[i].position + (rigidbodies[i].velocity * dt);
            }
            
            // 3. Detectar Colisões e Gerar Eventos
            std::vector<ContactManifold> activeContacts;
            
            // Um loop "bruto" simples O(N^2). O uso da BVH na Broad-Phase serve justamente 
            // para evitar esse loop duplo em jogos com milhares de objetos.
            for (Entity i = 0; i < colliders.size(); ++i) {
                for (Entity j = i + 1; j < colliders.size(); ++j) {
                    ContactManifold m = CollisionDetector::CheckCollision(
                        i, transforms[i], colliders[i], 
                        j, transforms[j], colliders[j]
                    );

                    if (m.isColliding) {
                        activeContacts.push_back(m);
                        
                        // Despacha Evento para a Game Engine tocar Som/Dano
                        m_collisionEvents.push_back({i, j, transforms[i].position, m.normal, rigidbodies[i].velocity.Magnitude(), CollisionType::Enter});
                    }
                }
            }

            // 4. Resolver as Forças (Empurrar e Quicar)
            PhysicsSolver::ResolveContacts(activeContacts, transforms, rigidbodies);
        }
    };
}