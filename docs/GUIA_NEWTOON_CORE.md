# Guia Completo de Implementação e Uso — `Newtoon::Core`

O módulo **`Newtoon::Core`** (`include/Newtoon/`) é uma biblioteca de física **header-only (C++17)** orientada a dados (DOD/ECS), sem dependências externas, com suporte separado para **3D** e **2D**, paralelismo via pool de threads persistente, **BVH-4 com Morton Codes (SIMD)**, **Wide-SIMD 4× no solver**, **Soft Step (TGS Soft)**, **Warm Starting**, **Simulation Islands**, **Render Interpolation** e **XPBD para cabelos e tecidos**.

---

## 1. Regras Fundamentais de Uso

1. **A sua Engine é dona dos dados**: a Newtoon **não** aloca nem gerencia objetos de cena. Você mantém vetores contíguos (`std::vector<Transform>`, `std::vector<Rigidbody>`, `std::vector<Collider>`) e os passa para `Simulate(...)`.
2. **`Entity` é o índice no vetor**: o índice `0, 1, 2, ...` nos vetores identifica a entidade. Todos os vetores (`tf`, `rb`, `col`) devem estar sincronizados no mesmo índice.
3. **Corpo estático vs. dinâmico vs. cinemático**:
   - **Estático**: `rb.SetMass(0.f)` (ou `SetMassFromShape(rb, col, 0.f)`). Nunca move e não entra na lista de ativos.
   - **Dinâmico**: `SetMassFromShape(rb, col, massaKg)` (calcula massa e inércia local automaticamente).
   - **Cinemático**: `rb.isKinematic = true` e `rb.SetMass(0.f)`. Move por velocidade sem sofrer gravidade/impulsos.
4. **Não chame `Simulate` com `dt` negativo**: `Simulate` acumula `dt` (limitado a `0.25s`) e executa passos fixos de tamanho `fixedDt` (padrão `1/60s`).
5. **Consuma e limpe eventos a cada quadro**: leia `world.GetCollisionEvents()` e chame `world.ClearEvents()` após processar (`Enter`, `Stay`, `Exit`).
6. **Alinhamento em C++17**: `Vec3`, `Quat`, `Rigidbody` e `Rigidbody2D` usam `alignas(16)` / `alignas(64)`. Compile sempre em **C++17 ou superior** (`std::vector` respeita `alignas` automaticamente em C++17).
7. **2D vs. 3D separados**: para jogos 2D, use `PhysicsWorld2D` + `Transform2D` + `Rigidbody2D` + `Collider2D` (`newtoon_2d.hpp`), evitando custo de eixo Z, quaternions e tensor 3×3.

---

## 2. Como Integrar no Projeto

### Via CMake (Recomendado)
```cmake
add_subdirectory(caminho/para/Newtoon)
target_link_libraries(sua_engine PRIVATE newtoon_core)
```

### Inclusão Direta (Header-only)
Basta adicionar `include/` ao *include path* e incluir:
```cpp
#include "Newtoon/newtoon.hpp"         // 3D + 2D + Matemática + Pool
#include "Newtoon/newtoon_modules.hpp" // Opcional: XPBD (cabelo/tecido), Veículos, Ragdoll
```

---

## 3. Exemplos Práticos

### Exemplo A: Simulação 3D, Eventos e Raycast
```cpp
#include "Newtoon/newtoon.hpp"
using namespace Newtoon::Core;

int main() {
  std::vector<Transform> tf(2);
  std::vector<Rigidbody> rb(2);
  std::vector<Collider> col(2);

  // 0: Chão estático
  tf[0].position = {0.f, -0.5f, 0.f};
  col[0].type = ShapeType::Box;
  col[0].extents = {50.f, 0.5f, 50.f};
  rb[0].SetMass(0.f);

  // 1: Caixa dinâmica de 5 kg
  tf[1].position = {0.f, 5.f, 0.f};
  col[1].type = ShapeType::Box;
  col[1].extents = {0.5f, 0.5f, 0.5f};
  SetMassFromShape(rb[1], col[1], 5.f);

  PhysicsWorld world;
  world.Rebuild(tf, rb, col); // opcional: pré-constrói a BVH4 antes do primeiro frame

  for (int frame = 0; frame < 120; ++frame) {
    world.Simulate(tf, rb, col, 1.f / 60.f);

    for (const auto& ev : world.GetCollisionEvents()) {
      if (ev.type == CollisionType::Enter) {
        // Som de impacto: ev.entityA, ev.entityB, ev.impactForce
      }
    }
    world.ClearEvents();
  }

  RaycastHit hit = world.Raycast({0.f, 10.f, 0.f}, {0.f, -1.f, 0.f}, 20.f);
  if (hit.hit) {
    // hit.entity, hit.point, hit.normal, hit.distance
  }
}
```

### Exemplo B: Simulação 2D Dedicada (`PhysicsWorld2D`)
```cpp
#include "Newtoon/newtoon.hpp"
using namespace Newtoon::Core;

void Exemplo2D() {
  std::vector<Transform2D> tf(2);
  std::vector<Rigidbody2D> rb(2);
  std::vector<Collider2D> col(2);

  // Chão 2D estático
  tf[0].position = {0.f, -0.5f};
  col[0].type = ShapeType2D::Box;
  col[0].halfExtents = {20.f, 0.5f};
  rb[0].SetMass(0.f);

  // Círculo 2D dinâmico
  tf[1].position = {0.f, 4.f};
  col[1].type = ShapeType2D::Circle;
  col[1].radius = 0.5f;
  SetMassFromShape2D(rb[1], col[1], 2.f);

  PhysicsWorld2D world2d;
  world2d.Simulate(tf, rb, col, 1.f / 60.f);
}
```

### Exemplo C: Interpolação de Renderização (144Hz / 240Hz)
Quando a física roda em passo fixo (`60 Hz`) e o monitor renderiza a `144 Hz` ou `240 Hz`, use `GetInterpolatedTransforms` para evitar *stuttering* visual sem alterar o estado físico real:
```cpp
std::vector<Transform> renderTransforms;
world.Simulate(tf, rb, col, frameDt);
world.GetInterpolatedTransforms(tf, renderTransforms);
// Envie `renderTransforms` para o seu renderizador!
```
*(No 2D, use `world2d.GetInterpolatedTransforms(tf2d, renderTransforms2d)`.)*

### Exemplo D: Cabelos e Tecidos com `XPBDClothAndHairSolver`
```cpp
#include "Newtoon/newtoon_modules.hpp"
using namespace Newtoon::Core;

void ExemploCabelo() {
  std::vector<XPBDParticle> pts(6);
  std::vector<XPBDDistanceConstraint> dists;
  std::vector<XPBDBendingConstraint> bends;

  for (size_t i = 0; i < pts.size(); ++i) {
    pts[i].position = {static_cast<float>(i) * 0.2f, 2.f, 0.f};
    pts[i].inverseMass = (i == 0) ? 0.f : 1.f; // i == 0 é a raiz fixa
    if (i > 0) dists.push_back({static_cast<Entity>(i - 1), static_cast<Entity>(i), 0.2f, 0.f, 0.f});
    if (i > 1) bends.push_back({static_cast<Entity>(i - 2), static_cast<Entity>(i - 1), static_cast<Entity>(i), 0.4f, 0.01f, 0.f});
  }

  std::vector<XPBDSphereCollider> headCollider = {{{0.f, 1.7f, 0.f}, 0.35f}};
  XPBDClothAndHairSolver solver;
  solver.substeps = 4; // "Small Steps": mais substeps com 1 iteração é mais estável
  solver.groundY = 0.f;

  solver.Step(pts, dists, bends, headCollider, {1.5f, 0.f, 0.f}, 1.f / 60.f);
}
```

---

## 4. Guia de Otimização para Escala Massiva (100k – 1M+ Corpos)

| Parâmetro | Padrão | Recomendação para Escala Extrema | Efeito |
| :--- | :--- | :--- | :--- |
| `enableInterpolation` | `true` | `false` (se não interpolar) | Evita copiar `prevTransforms_` (48 B/corpo) a cada passo |
| `enableIslands` | `true` | `true` | Agrupa corpos em ilhas para dormir/acordar em bloco |
| `enableWarmStarting` | `true` | `true` | Reduz iterações necessárias em pilhas de corpos |
| `enableSoftConstraints` | `true` | `true` (`30 Hz`) | Estabiliza grandes diferenças de massa (Soft Step) |
| `Rebuild(tf, rb, col)` | manual | Chamar após carregar cena | Constrói a `BVH4` (Morton LBVH) antes do loop |

### Telemetria em Tempo Real (`world.stats`)
- `bodies`, `awake`, `islands`, `pairs`, `contacts`, `warmStartedContacts`
- `colorsUsed`, `largestColorBucket` (qualidade do paralelismo no *Graph Coloring*)
- `integrateMs`, `prepareMs`, `broadphaseMs`, `narrowphaseMs`, `solverMs`, `ccdMs`, `finishMs`
- `hitStepLimit` (`true` se o quadro excedeu `maxStepsPerFrame`)

---

## 5. Executando os Testes Automatizados

O projeto utiliza **CMake + CTest** para validar sintaxe, compilação estrita (`/W4` no MSVC, `-Wall -Wextra -Wpedantic` no GCC/Clang) e comportamento físico:

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure -V
```

### Suítes incluídas
1. **`newtoon_module_smoke`** ([tests/newtoon_module_smoke.cpp](file:///c:/Users/espir/Downloads/NTNPHYSICS/Newtoon%20Physics%20for%20engine/tests/newtoon_module_smoke.cpp)): queda e repouso 3D.
2. **`newtoon_advanced_test`** ([tests/newtoon_advanced_test.cpp](file:///c:/Users/espir/Downloads/NTNPHYSICS/Newtoon%20Physics%20for%20engine/tests/newtoon_advanced_test.cpp)): matemática SIMD, pool persistente, *Warm Starting*, *Simulation Islands*, *Render Interpolation*, `XPBDClothAndHairSolver`, `BVH4` (Query/RayQuery/Refit), *Wide-SIMD* e *Soft Step* (50:1).
3. **`newtoon_2d_test`** ([tests/newtoon_2d_test.cpp](file:///c:/Users/espir/Downloads/NTNPHYSICS/Newtoon%20Physics%20for%20engine/tests/newtoon_2d_test.cpp)): `Vec2`, `Rot2`, `PhysicsWorld2D`, *Warm Starting* 2D, `Raycast` 2D e interpolação 2D.
4. **`newtoon_scale_test`** ([tests/newtoon_scale_test.cpp](file:///c:/Users/espir/Downloads/NTNPHYSICS/Newtoon%20Physics%20for%20engine/tests/newtoon_scale_test.cpp)): benchmark de escala com 100.000 corpos 3D e 100.000 corpos 2D.
