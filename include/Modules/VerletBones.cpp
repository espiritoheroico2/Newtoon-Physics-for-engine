#pragma once
#include "../NewtoonTypes.h"
#include <vector>

namespace Newtoon {
    struct DynamicBone {
        Vec3 position;
        Vec3 previousPosition; 
        float mass;
        float windInfluence;   
    };

    struct BoneConstraint {
        Entity boneA;
        Entity boneB;
        float restLength; 
        float stiffness;  
    };

    class SkeletalPhysicsSolver {
    public:
        void Step(std::vector<DynamicBone>& bones, std::vector<BoneConstraint>& constraints, const Vec3& windForce, float dt) {
            // 1. Integrar (Verlet)
            for (auto& bone : bones) {
                if (bone.mass <= 0.0f) continue; 

                Vec3 velocity = {
                    bone.position.x - bone.previousPosition.x,
                    bone.position.y - bone.previousPosition.y,
                    bone.position.z - bone.previousPosition.z
                };

                bone.previousPosition = bone.position;

                Vec3 totalForce = {0.0f, -9.81f * bone.mass, 0.0f};
                totalForce.x += windForce.x * bone.windInfluence;
                totalForce.z += windForce.z * bone.windInfluence;

                bone.position.x += velocity.x + (totalForce.x / bone.mass) * dt * dt;
                bone.position.y += velocity.y + (totalForce.y / bone.mass) * dt * dt;
                bone.position.z += velocity.z + (totalForce.z / bone.mass) * dt * dt;
            }

            // 2. Resolver Restrições IK
            const int iterations = 3;
            for (int i = 0; i < iterations; ++i) {
                for (const auto& constraint : constraints) {
                    DynamicBone& b1 = bones[constraint.boneA];
                    DynamicBone& b2 = bones[constraint.boneB];

                    Vec3 delta = {b2.position.x - b1.position.x, b2.position.y - b1.position.y, b2.position.z - b1.position.z};
                    float currentLength = delta.Magnitude();
                    if(currentLength == 0.0f) continue;
                    
                    float difference = (currentLength - constraint.restLength) / currentLength;
                    Vec3 correction = {
                        delta.x * difference * 0.5f * constraint.stiffness,
                        delta.y * difference * 0.5f * constraint.stiffness,
                        delta.z * difference * 0.5f * constraint.stiffness
                    };

                    if (b1.mass > 0.0f) {
                        b1.position.x += correction.x; b1.position.y += correction.y; b1.position.z += correction.z;
                    }
                    if (b2.mass > 0.0f) {
                        b2.position.x -= correction.x; b2.position.y -= correction.y; b2.position.z -= correction.z;
                    }
                }
            }
        }
    };
}