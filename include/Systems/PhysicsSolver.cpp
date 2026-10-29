#pragma once
#include <vector>
#include <algorithm>
#include "../ECS/Components3D.h"
#include "CollisionDetector.h"

namespace Newtoon {
    class PhysicsSolver {
    public:
        static void ResolveContacts(std::vector<ContactManifold>& manifolds, 
                                    std::vector<Transform>& transforms, 
                                    std::vector<Rigidbody>& bodies) {
            
            for (auto& m : manifolds) {
                Rigidbody& rbA = bodies[m.entityA];
                Rigidbody& rbB = bodies[m.entityB];
                
                if (rbA.IsStatic() && rbB.IsStatic()) continue;

                // --- 1. Resolução de Velocidade (Impulso) ---
                Vec3 rv = rbB.velocity - rbA.velocity;
                float velAlongNormal = Vec3::Dot(rv, m.normal);

                // Se estão se afastando, não faz nada
                if (velAlongNormal > 0) continue;

                // Calcular Restituição (Bounciness) - usando média (0.2 como placeholder)
                float e = 0.2f; 
                
                // Magnitude do Impulso (j)
                float j = -(1.0f + e) * velAlongNormal;
                j /= (rbA.inverseMass + rbB.inverseMass);

                // Aplica o vetor de impulso
                Vec3 impulse = m.normal * j;
                
                if (!rbA.IsStatic()) rbA.velocity = rbA.velocity - (impulse * rbA.inverseMass);
                if (!rbB.IsStatic()) rbB.velocity = rbB.velocity + (impulse * rbB.inverseMass);

                // --- 2. Correção Posicional (Evitar Sinking) ---
                // Para evitar que objetos afundem (jittering), empurramos eles baseados na penetração.
                const float percent = 0.2f; // Resolve 20% da penetração por frame
                const float slop = 0.01f;   // Permite penetração mínima sem corrigir (estabilidade)
                
                float correctionMagnitude = std::max(m.penetration - slop, 0.0f) / (rbA.inverseMass + rbB.inverseMass) * percent;
                Vec3 correction = m.normal * correctionMagnitude;

                if (!rbA.IsStatic()) transforms[m.entityA].position = transforms[m.entityA].position - (correction * rbA.inverseMass);
                if (!rbB.IsStatic()) transforms[m.entityB].position = transforms[m.entityB].position + (correction * rbB.inverseMass);
                
                // Acorda os objetos na colisão
                rbA.WakeUp(); rbB.WakeUp();
            }
        }
    };
}