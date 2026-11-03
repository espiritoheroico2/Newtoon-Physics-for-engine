// Teste de fumaca da Newtoon: esfera caindo numa caixa, raycast, 2D. Compile: g++ -std=c++20 -O1 -pthread -Iinclude tests/smoke_newtoon.cpp -o smoke
#include "Newtoon.h"
#include <cstdio>
using namespace Newtoon;
int main() {
  // mundo 3D: chao (caixa estatica) e uma esfera caindo
  PhysicsWorld world;
  std::vector<Transform> tf(2); std::vector<Rigidbody> rb(2); std::vector<Collider> col(2);
  tf[0].position = {0, -0.5f, 0}; tf[0].rotation = Quat{};
  col[0].type = ShapeType::Box; col[0].extents = {20, 0.5f, 20}; rb[0].inverseMass = 0.f;
  tf[1].position = {0, 5, 0}; tf[1].rotation = Quat{};
  col[1].type = ShapeType::Sphere; col[1].radius = 0.5f; rb[1].SetMass(1.f);
  int enters = 0;
  for (int i = 0; i < 300; ++i) {
    world.Simulate(tf, rb, col, 1.f / 60.f);
    for (auto& e : world.GetCollisionEvents()) if (e.type == CollisionType::Enter) ++enters;
    world.ClearEvents();
  }
  std::printf("esfera y=%.3f (esperado ~0.5), velocidade y=%.3f, contatos Enter=%d, dormindo=%d\n",
              tf[1].position.y, rb[1].velocity.y, enters, rb[1].isSleeping ? 1 : 0);
  // raycast para baixo a partir de y=3 deve acertar o chao em y~0
  RaycastHit h = world.Raycast({0, 3, 0}, {0, -1, 0}, 10.f);
  std::printf("raycast hit=%d distancia=%.3f (esperado ~3.0)\n", h.hit ? 1 : 0, h.distance);
  // 2D oficial: queda livre
  PhysicsWorld2D w2; std::vector<Transform2D> t2(1); std::vector<Rigidbody2D> r2(1);
  t2[0].position = {0, 10}; r2[0].SetMass(1.f);
  for (int i = 0; i < 60; ++i) w2.Simulate(t2, r2, 1.f / 60.f);
  std::printf("2D: y=%.3f (queda em 1s, esperado ~5.1)\n", t2[0].position.y);
  DebugVisualNewtoon::Get().DrawAABB({0,0,0},{1,1,1});
  std::printf("debug linhas=%zu\n", DebugVisualNewtoon::Get().GetLinesToRender().size());
  return 0;
}
