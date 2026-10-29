#pragma once
#include "NewtoonMath.h"
#include <cstdint>

namespace Newtoon {
    typedef uint32_t Entity;

    // Formas Físicas
    enum class ShapeType { Box, Sphere, Cylinder, CustomConvex };
    enum class ShapeType2D { Box, Circle, Polygon };

    // Materiais e Fricção
    enum class CombineMode { Average, Minimum, Maximum, Multiply };

    struct PhysicsMaterial {
        float friction;
        float bounciness;
        CombineMode frictionCombine = CombineMode::Average;
        CombineMode bounceCombine = CombineMode::Maximum;
    };

    // Eventos de Colisão
    enum class CollisionType { Enter, Stay, Exit };

    struct CollisionEvent {
        Entity entityA;
        Entity entityB;
        Vec3 contactPoint;
        Vec3 contactNormal;
        float impactForce;
        CollisionType type;
    };

    struct RaycastHit {
        bool hit = false;
        Vec3 point;
        Vec3 normal;
        float distance = 0.0f;
        Entity entity;
    };
}