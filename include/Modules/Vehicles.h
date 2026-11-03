// Suspensao de veiculo por raycast (mola de Hooke + amortecedor). Ver docs/PHYSICS.md.
#pragma once
#include "../Systems/PhysicsWorld.h"

namespace Newtoon {

class VehicleSolver {
 public:
  // Uma roda: raio para baixo a partir da roda; mola de Hooke + amortecedor empurram o chassi.
  static void UpdateWheel(WheelCollider& w, const Vec3& wheelPos, Rigidbody& body, const PhysicsWorld& world, Entity chassis, float /*dt*/) {
    const RaycastHit h = world.Raycast(wheelPos, {0, -1, 0}, w.suspensionRestLength + w.radius, chassis);
    w.isGrounded = h.hit;
    if (!h.hit) return;
    w.currentSuspensionLength = h.distance - w.radius;
    const float compression = w.suspensionRestLength - w.currentSuspensionLength;
    body.AddForce({0.f, w.springStiffness * compression - w.springDamper * body.velocity.y, 0.f});
  }
};

} // namespace Newtoon
