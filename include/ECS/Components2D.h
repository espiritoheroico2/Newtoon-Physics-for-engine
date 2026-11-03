#pragma once
#include "../NewtoonTypes.h"

namespace Newtoon {
    struct Transform2D {
        Vec2 position;
        float rotation; // Radianos
    };

    struct Rigidbody2D {
        Vec2 velocity = {0,0};
        Vec2 forceAccumulator = {0,0};
        float angularVelocity = 0.0f;
        float torqueAccumulator = 0.0f;
        
        float inverseMass = 1.0f;
        float inverseInertia = 1.0f;
        
        bool isSleeping = false;
        bool useCCD = false;
        
        void SetMass(float mass) { inverseMass = (mass <= 0.0f) ? 0.0f : (1.0f / mass); }
        bool IsStatic() const { return inverseMass == 0.0f; }
        void WakeUp() { isSleeping = false; }
    };

    struct Collider2D {
        ShapeType2D type;
        uint16_t materialId = 0;
        Vec2 extents;
        float radius;
    };
}