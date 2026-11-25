// Teste de fumaca do modulo Newtoon::Core (include/Newtoon). Compila sozinho:
//   g++ -std=c++17 -O2 -pthread -Iinclude tests/newtoon_module_smoke.cpp -o smoke && ./smoke
#include "Newtoon/newtoon.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

using namespace Newtoon::Core;

static int gFailures = 0;
static void Check(bool ok, const char* what) {
  std::printf("%s: %s\n", ok ? "ok" : "FALHOU", what);
  if (!ok) ++gFailures;
}

int main() {
  // Chao estatico + caixa dinamica caindo: depois de alguns segundos a caixa deve estar em repouso sobre o chao.
  std::vector<Transform> tf(2);
  std::vector<Rigidbody> rb(2);
  std::vector<Collider> col(2);
  tf[0].position = {0.f, 0.f, 0.f};
  col[0].type = ShapeType::Box;
  col[0].extents = {50.f, 0.5f, 50.f};
  rb[0].SetMass(0.f); // massa 0 = corpo estatico
  tf[1].position = {0.f, 5.f, 0.f};
  col[1].type = ShapeType::Box;
  col[1].extents = {0.5f, 0.5f, 0.5f};
  SetMassFromShape(rb[1], col[1], 1.f);

  PhysicsWorld world;
  for (int i = 0; i < 600; ++i) world.Simulate(tf, rb, col, 1.f / 60.f);

  Check(world.stats.bodies == 2, "o mundo ve os dois corpos");
  Check(world.stats.steps > 0, "a simulacao avancou passos");
  Check(std::isfinite(tf[1].position.y), "posicao final e finita");
  Check(tf[1].position.y > 0.f && tf[1].position.y < 2.f, "a caixa parou perto do chao (y entre 0 e 2)");

  std::printf("Newtoon.Core smoke: %d falha(s)\n", gFailures);
  return gFailures == 0 ? 0 : 1;
}
