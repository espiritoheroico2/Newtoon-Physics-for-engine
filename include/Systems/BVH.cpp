#pragma once
#include "../NewtoonTypes.h"
#include <vector>

namespace Newtoon {
    struct AABB {
        Vec3 min;
        Vec3 max;
    };

    struct BVHNode {
        AABB bounds;
        int leftChildIndex = -1;  // -1 significa que é uma folha
        int rightChildIndex = -1;
        Entity entityId;          // Só tem valor se for folha
    };

    class BVH {
    public:
        std::vector<BVHNode> nodes;

        // O Raycaster usa a BVH para não testar objetos que estão longe
        static RaycastHit Raycast(const BVH& bvh, const Vec3& origin, const Vec3& direction, float maxDistance) {
            RaycastHit hit;
            hit.hit = false;
            
            // Se a árvore estiver vazia, retorna falso
            if (bvh.nodes.empty()) return hit;

            // Lógica de Traversal (Navegação) na árvore ficaria aqui.
            // Para estrutura da API, retornamos o objeto base.
            return hit;
        }
    };
}