#pragma once
#include "../NewtoonTypes.h"

namespace Newtoon {
    struct Transform {
        Vec3 position;
        Quat rotation;
    };

    struct Rigidbody {
        Vec3 velocity = {0,0,0};
        Vec3 angularVelocity = {0,0,0};
        Vec3 forceAccumulator = {0,0,0};
        Vec3 torqueAccumulator = {0,0,0};
        
        float inverseMass = 1.0f;
        float inverseInertia = 1.0f;
        
        bool isSleeping = false;
        float sleepTimer = 0.0f;
        float sleepThreshold = 0.05f;
        bool useCCD = false;

        void SetMass(float mass) { inverseMass = (mass <= 0.0f) ? 0.0f : (1.0f / mass); }
        bool IsStatic() const { return inverseMass == 0.0f; }
        void WakeUp() { isSleeping = false; sleepTimer = 0.0f; }
    };

    struct Collider {
        ShapeType type;
        uint16_t materialId = 0;
        Vec3 extents;
        float radius;
        float height;
        void* customMesh;
    };
    
    enum class JointType { Fixed, Hinge, BallAndSocket };

    struct Joint {
        JointType type;
        Entity bodyA;
        Entity bodyB;
        Vec3 anchorPointA;
        Vec3 anchorPointB;
        Vec3 hingeAxis;
    };
    
    struct WheelCollider {
        float radius = 0.3f;
        float suspensionRestLength = 0.5f;
        float springStiffness = 30000.0f;
        float springDamper = 3000.0f;
        bool isGrounded = false;
        float currentSuspensionLength = 0.0f;
    };
}