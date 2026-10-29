#pragma once
#include "../ECS/Components3D.h"

namespace Newtoon {

    struct ContactManifold {
        bool isColliding = false;
        Entity entityA, entityB;
        Vec3 normal;       // Direção para empurrar os objetos para fora
        float penetration; // O quão fundo eles se transpassaram
    };

    class CollisionDetector {
    public:
        static ContactManifold CheckCollision(Entity eA, const Transform& tA, const Collider& cA, 
                                              Entity eB, const Transform& tB, const Collider& cB) {
            
            if (cA.type == ShapeType::Sphere && cB.type == ShapeType::Sphere) {
                return CheckSphereSphere(eA, tA, cA, eB, tB, cB);
            }
            if (cA.type == ShapeType::Box && cB.type == ShapeType::Box) {
                return CheckAABBAABB(eA, tA, cA, eB, tB, cB);
            }
            // (Futuro: Sphere vs Box, e GJK para ConvexMeshes)
            
            return {false};
        }

    private:
        static ContactManifold CheckSphereSphere(Entity eA, const Transform& tA, const Collider& cA, 
                                                 Entity eB, const Transform& tB, const Collider& cB) {
            ContactManifold m;
            Vec3 delta = tB.position - tA.position;
            float distSq = delta.Dot(delta);
            float radiusSum = cA.radius + cB.radius;

            if (distSq < (radiusSum * radiusSum)) {
                m.isColliding = true;
                m.entityA = eA; m.entityB = eB;
                float dist = std::sqrt(distSq);
                
                if (dist == 0.0f) { // Estão exatamente no mesmo pixel
                    m.penetration = cA.radius;
                    m.normal = {0, 1, 0}; 
                } else {
                    m.penetration = radiusSum - dist;
                    m.normal = delta * (1.0f / dist); // Aponta de A para B
                }
            }
            return m;
        }

        static ContactManifold CheckAABBAABB(Entity eA, const Transform& tA, const Collider& cA, 
                                             Entity eB, const Transform& tB, const Collider& cB) {
            ContactManifold m;
            // Lógica para caixas que não giram (AABB). Para caixas que giram, usa-se o SAT (Separating Axis Theorem)
            Vec3 aMin = tA.position - cA.extents;
            Vec3 aMax = tA.position + cA.extents;
            Vec3 bMin = tB.position - cB.extents;
            Vec3 bMax = tB.position + cB.extents;

            if (aMin.x <= bMax.x && aMax.x >= bMin.x &&
                aMin.y <= bMax.y && aMax.y >= bMin.y &&
                aMin.z <= bMax.z && aMax.z >= bMin.z) {
                
                m.isColliding = true;
                m.entityA = eA; m.entityB = eB;
                // Cálculo simplificado de penetração: pegar o eixo com menor sobreposição
                m.normal = {0, 1, 0}; // Placeholder de normal simplificado para eixo Y (chão)
                m.penetration = (aMax.y - bMin.y); // Exemplo de penetração no Y
            }
            return m;
        }
    };
}