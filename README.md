# Newtoon Physics Engine 🍎

Newtoon Physics é uma biblioteca de física 2D e 3D focada em alta performance, construída estritamente com princípios de **Data-Oriented Design (DOD)** e pronta para integração em arquiteturas **Entity Component System (ECS)**.

## Características
* **Memória Contígua:** Sem ponteiros soltos, virtualização ou `std::shared_ptr` no loop de simulação.
* **Dual API (2D e 3D):** Subsistemas separados. O 2D usa matrizes matemáticas puramente bidimensionais para máxima performance.
* **Multi-Threaded (Job System):** Integração paralela de velocidades e Broad-Phase dividida por núcleos da CPU.
* **Pronto para Renderização:** Expõe a `BVH` internamente para que a sua engine gráfica possa usar os mesmos dados espaciais para Raytracing e Culling de Luzes, poupando memória da GPU.
* **Módulos Avançados:** Suporte a Ragdoll, Veículos (Raycast Suspension), CCD (Continuous Collision Detection) e Verlet Integration (Ossos/Tecidos dinâmicos).

---

## 🛠️ Como Integrar na sua Game Engine

### 1. Atualizando o Loop Principal (C++ Backend)

A sua Engine deve ser a "dona" dos arrays de componentes. A Newtoon apenas processa os dados.

```cpp
#include "Newtoon/PhysicsWorld.h"
#include "Newtoon/DebugVisualNewtoon.h"

Newtoon::PhysicsWorld physics3D;
Newtoon::PhysicsWorld2D physics2D;

void EngineUpdate(float dt) {
    // 1. Processa a física 3D
    physics3D.Simulate(myECS.transforms, myECS.rigidbodies, dt);
    
    // 2. Lê os eventos de colisão para tocar sons ou dar dano
    for(auto& event : physics3D.GetCollisionEvents()) {
        if(event.type == Newtoon::CollisionType::Enter) {
            AudioEngine::Play("hit.wav");
        }
    }
    
    // 3. Limpa os eventos para o próximo frame
    physics3D.ClearEvents();
    
    // 4. Renderiza Gizmos (opcional para Editor mode)
    physics3D.RenderGizmos();
    MyRenderer::DrawLines(Newtoon::DebugVisualNewtoon::Get().GetLinesToRender());
    Newtoon::DebugVisualNewtoon::Get().Clear();
}