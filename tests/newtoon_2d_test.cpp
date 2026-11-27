// Testes automatizados do modulo 2D dedicado (Newtoon::Core 2D):
// - Matematica 2D (Vec2, Cross 2D, Perp, Rot2, LerpAngle)
// - PhysicsWorld2D (Circle/Box, Sweep-and-Prune, SAT 2D, Warm Starting, Raycast 2D, Interpolacao)
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
  // 1) Matematica 2D e Rot2
  {
    const Vec2 a{3.f, 4.f}, b{1.f, 2.f};
    Check(std::fabs(a.Magnitude() - 5.f) < 1e-5f, "Vec2 Magnitude");
    Check(std::fabs(Dot(a, b) - 11.f) < 1e-5f, "Vec2 Dot");
    Check(std::fabs(Cross(Vec2{1.f, 0.f}, Vec2{0.f, 1.f}) - 1.f) < 1e-5f, "Vec2 Cross escalar");
    const Rot2 r = Rot2::FromAngle(1.5707963f);
    const Vec2 rv = r.Rotate({1.f, 0.f});
    Check(std::fabs(rv.x) < 1e-4f && std::fabs(rv.y - 1.f) < 1e-4f, "Rot2 90 deg");
    const Vec2 irv = r.InvRotate(rv);
    Check(std::fabs(irv.x - 1.f) < 1e-4f && std::fabs(irv.y) < 1e-4f, "Rot2 InvRotate");
  }

  // 2) Simulacao 2D: chao + pilha de caixas + circulo
  {
    std::vector<Transform2D> tf(4);
    std::vector<Rigidbody2D> rb(4);
    std::vector<Collider2D> col(4);

    // Chao estatico 2D
    tf[0].position = {0.f, -0.5f};
    col[0].type = ShapeType2D::Box;
    col[0].halfExtents = {50.f, 0.5f};
    rb[0].SetMass(0.f);

    // Duas caixas empilhadas
    for (int i = 1; i <= 2; ++i) {
      tf[i].position = {0.f, 0.5f + (i - 1) * 1.02f};
      col[i].type = ShapeType2D::Box;
      col[i].halfExtents = {0.5f, 0.5f};
      SetMassFromShape2D(rb[i], col[i], 1.f);
    }

    // Circulo caindo ao lado
    tf[3].position = {3.f, 4.f};
    col[3].type = ShapeType2D::Circle;
    col[3].radius = 0.5f;
    SetMassFromShape2D(rb[3], col[3], 1.f);

    PhysicsWorld2D world2d;
    int warmSeen = 0;
    for (int f = 0; f < 300; ++f) {
      world2d.Simulate(tf, rb, col, 1.f / 60.f);
      warmSeen += world2d.stats.warmStartedContacts;
    }

    Check(world2d.stats.bodies == 4, "PhysicsWorld2D ve os 4 corpos");
    Check(warmSeen > 0, "PhysicsWorld2D Warm Starting reaproveitou contatos");
    Check(tf[2].position.y > 1.0f && tf[2].position.y < 2.5f, "Caixa 2D superior manteve pilha estavel");
    Check(tf[3].position.y > 0.f && tf[3].position.y < 1.f, "Circulo 2D repousou sobre o chao");

    const RaycastHit2D hit = world2d.Raycast({3.f, 5.f}, {0.f, -1.f}, 10.f);
    Check(hit.hit && hit.entity == 3, "Raycast 2D acertou o circulo");

    world2d.Simulate(tf, rb, col, 0.008f);
    std::vector<Transform2D> interp;
    world2d.GetInterpolatedTransforms(tf, interp);
    Check(interp.size() == tf.size() && std::isfinite(interp[3].position.y), "Render interpolation 2D valida");
  }

  std::printf("Newtoon.Core 2D: %d falha(s)\n", gFailures);
  return gFailures == 0 ? 0 : 1;
}
