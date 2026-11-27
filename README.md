# Newtoon Physics Engine 🍎

Newtoon Physics é uma biblioteca de física 2D e 3D focada em alta performance, construída estritamente com princípios de **Data-Oriented Design (DOD)** e pronta para integração em arquiteturas **Entity Component System (ECS)**.

## Características
* **Memória Contígua:** Sem ponteiros soltos, virtualização ou `std::shared_ptr` no loop de simulação.
* **Dual API (2D e 3D):** Subsistemas separados. O 2D usa matrizes matemáticas puramente bidimensionais para máxima performance.
* **Multi-Threaded:** `PhysicsWorld` usa um pool de threads persistente (`JobPool`); o mundo 2D usa o `JobSystem` oficial (`std::async`).
* **Pronto para Renderização:** Expõe a `BVH` internamente para que a sua engine gráfica possa usar os mesmos dados espaciais para Raytracing e Culling de Luzes, poupando memória da GPU.
* **Módulos Avançados:** Suporte a Ragdoll, Veículos (Raycast Suspension), CCD (Continuous Collision Detection) e Verlet Integration (Ossos/Tecidos dinâmicos).

---

## 🛠️ Como Integrar na sua Game Engine

### 1. Atualizando o Loop Principal (C++ Backend)

A sua Engine deve ser a "dona" dos arrays de componentes. A Newtoon apenas processa os dados.

```cpp
#include "Newtoon.h"

Newtoon::PhysicsWorld physics3D;          // 3D: colisao SAT (caixa/esfera), solver com atrito, sono, CCD, gatilhos
Newtoon::PhysicsWorld2D physics2D;        // 2D: integracao simples

void EngineUpdate(float dt) {
    // 1. A engine e dona dos arrays; a Newtoon so processa (passo fixo; o dt variavel e acumulado)
    physics3D.Simulate(myECS.transforms, myECS.rigidbodies, myECS.colliders, dt);

    // 2. Eventos de colisao (Enter / Stay / Exit) para sons e dano
    for (auto& event : physics3D.GetCollisionEvents()) {
        if (event.type == Newtoon::CollisionType::Enter) AudioEngine::Play("hit.wav");
    }
    physics3D.ClearEvents();

    // 3. Gizmos de depuracao (opcional)
    for (auto& l : Newtoon::DebugVisualNewtoon::Get().GetLinesToRender()) MyRenderer::DrawLine(l.start, l.end, l.color);
    Newtoon::DebugVisualNewtoon::Get().Clear();
}
```

---

## Documentação Completa

Consulte o **[Guia Completo de Implementação e Uso (`Newtoon::Core`)](docs/GUIA_NEWTOON_CORE.md)** para regras de integração, exemplos 3D/2D/XPBD, interpolação, configuração para escala massiva e telemetria.

## Testes

### Via CMake + CTest (Recomendado)

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Teste de fumaça legado sem framework (so `g++`):

```
g++ -std=c++20 -O1 -pthread -Iinclude tests/smoke_newtoon.cpp -o smoke && ./smoke
```
