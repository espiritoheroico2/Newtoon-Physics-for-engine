#pragma once
#include "../ECS/Components3D.h"
#include <vector>

namespace Newtoon {
    struct RagdollPart {
        Entity boneEntity;    
        Entity physicsEntity; 
    };

    class RagdollController {
    public:
        std::vector<RagdollPart> parts;
        bool isActive = false;

        void EnableRagdoll(std::vector<Rigidbody>& bodies, std::vector<Transform>& transforms) {
            isActive = true;
            for (auto& part : parts) {
                Rigidbody& rb = bodies[part.physicsEntity];
                rb.inverseMass = 1.0f / 15.0f; // Exemplo de massa
                rb.WakeUp();
            }
        }

        void DisableRagdoll(std::vector<Rigidbody>& bodies) {
            isActive = false;
            for (auto& part : parts) {
                Rigidbody& rb = bodies[part.physicsEntity];
                rb.isSleeping = true;
                rb.inverseMass = 0.0f; 
            }
        }
        
        void SyncBonesToPhysics(std::vector<Transform>& transforms) {
            if (!isActive) return;
            for (auto& part : parts) {
                transforms[part.boneEntity].position = transforms[part.physicsEntity].position;
                transforms[part.boneEntity].rotation = transforms[part.physicsEntity].rotation;
            }
        }
    };
}