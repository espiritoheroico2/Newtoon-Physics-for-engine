// Controle de ragdoll (liga/desliga a massa das partes). Ver docs/PHYSICS.md.
#pragma once
#include "../ECS/Components3D.h"

#include <vector>

namespace Newtoon {

class RagdollController {
 public:
  std::vector<Entity> parts; bool active = false; float partMass = 15.f;
  void Enable(std::vector<Rigidbody>& bodies) { active = true; for (Entity e : parts) if (e < bodies.size()) { bodies[e].SetMass(partMass); bodies[e].WakeUp(); } }
  void Disable(std::vector<Rigidbody>& bodies) { active = false; for (Entity e : parts) if (e < bodies.size()) { bodies[e].inverseMass = 0.f; bodies[e].isSleeping = true; } }
};

} // namespace Newtoon
