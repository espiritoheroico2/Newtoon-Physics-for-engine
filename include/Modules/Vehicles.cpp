#pragma once
#include "../ECS/Components3D.h"
#include "../Systems/BVH.h"

namespace Newtoon {
    class VehicleSolver {
    public:
        // Resolve a física de 1 roda específica
        void UpdateWheel(WheelCollider& wheel, Transform& wheelTx, Rigidbody& carBody, const BVH& bvh, float dt) {
            
            // Atira um raio do chassi para baixo
            Vec3 down = {0.0f, -1.0f, 0.0f};
            RaycastHit hit = BVH::Raycast(bvh, wheelTx.position, down, wheel.suspensionRestLength + wheel.radius);

            wheel.isGrounded = hit.hit;

            if (hit.hit) {
                // 1. Calcula o quão comprimida está a mola da suspensão
                wheel.currentSuspensionLength = hit.distance - wheel.radius;
                float compression = wheel.suspensionRestLength - wheel.currentSuspensionLength;

                // 2. Calcula a velocidade da compressão (Amortecedor)
                float compressionVelocity = (carBody.velocity.y * -1.0f);

                // 3. Lei de Hooke (Força da Mola = rigidez * compressão + amortecedor * velocidade)
                float springForce = (wheel.springStiffness * compression) + (wheel.springDamper * compressionVelocity);

                // Empurra o chassi do carro para cima
                carBody.forceAccumulator.y += springForce;
            }
        }
    };
}