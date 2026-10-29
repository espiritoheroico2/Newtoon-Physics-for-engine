#pragma once
#include "../NewtoonTypes.h"
#include <utility>
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

        // Substitua a função Raycast de dentro do seu BVH.h por esta:

        static RaycastHit Raycast(const BVH& bvh, const Vec3& origin, const Vec3& direction, float maxDistance) {
        RaycastHit hit;
        hit.hit = false;
        hit.distance = maxDistance;

        // Função interna lambda para testar Ray vs AABB (Slab Method)
        auto RayVsAABB = [](const Vec3& orig, const Vec3& dir, const Vec3& min, const Vec3& max, float& tOut) -> bool {
            float tmin = (min.x - orig.x) / (dir.x != 0.0f ? dir.x : 0.00001f);
            float tmax = (max.x - orig.x) / (dir.x != 0.0f ? dir.x : 0.00001f);
            if (tmin > tmax) std::swap(tmin, tmax);

            float tymin = (min.y - orig.y) / (dir.y != 0.0f ? dir.y : 0.00001f);
            float tymax = (max.y - orig.y) / (dir.y != 0.0f ? dir.y : 0.00001f);
            if (tymin > tymax) std::swap(tymin, tymax);

            if ((tmin > tymax) || (tymin > tmax)) return false;
            if (tymin > tmin) tmin = tymin;
            if (tymax < tmax) tmax = tymax;

            float tzmin = (min.z - orig.z) / (dir.z != 0.0f ? dir.z : 0.00001f);
            float tzmax = (max.z - orig.z) / (dir.z != 0.0f ? dir.z : 0.00001f);
            if (tzmin > tzmax) std::swap(tzmin, tzmax);

            if ((tmin > tzmax) || (tzmin > tmax)) return false;
            if (tzmin > tmin) tmin = tzmin;

            tOut = tmin;
            return tOut >= 0;
        };

        // Varredura Linear de Fallback (Enquanto a Árvore real não é construída)
        // Otimização: A engine deve criar os BVHNodes reais no futuro.
        for (const auto& node : bvh.nodes) {
            float t;
            if (RayVsAABB(origin, direction, node.bounds.min, node.bounds.max, t)) {
                if (t < hit.distance) {
                    hit.hit = true;
                    hit.distance = t;
                    hit.entity = node.entityId;
                    hit.point = {
                        origin.x + direction.x * t,
                        origin.y + direction.y * t,
                        origin.z + direction.z * t
                    };
                }
            }
        }
        return hit;
        }
    };
}