#pragma once
#include "NewtoonMath.h"
#include <vector>

namespace Newtoon {
    struct DebugLine {
        Vec3 start, end, color;
    };

    class DebugVisualNewtoon {
    private:
        std::vector<DebugLine> m_lines;
        DebugVisualNewtoon() = default;

    public:
        // Singleton para acesso global fácil
        static DebugVisualNewtoon& Get() { 
            static DebugVisualNewtoon instance; 
            return instance; 
        }

        void DrawLine(const Vec3& start, const Vec3& end, const Vec3& color = {1,0,0}) {
            m_lines.push_back({start, end, color});
        }

        void DrawAABB(const Vec3& min, const Vec3& max, const Vec3& color = {0,1,0}) {
            Vec3 p1 = {min.x, min.y, min.z}; Vec3 p2 = {max.x, min.y, min.z};
            Vec3 p3 = {max.x, min.y, max.z}; Vec3 p4 = {min.x, min.y, max.z};
            Vec3 p5 = {min.x, max.y, min.z}; Vec3 p6 = {max.x, max.y, min.z};
            Vec3 p7 = {max.x, max.y, max.z}; Vec3 p8 = {min.x, max.y, max.z};

            DrawLine(p1, p2, color); DrawLine(p2, p3, color); DrawLine(p3, p4, color); DrawLine(p4, p1, color);
            DrawLine(p5, p6, color); DrawLine(p6, p7, color); DrawLine(p7, p8, color); DrawLine(p8, p5, color);
            DrawLine(p1, p5, color); DrawLine(p2, p6, color); DrawLine(p3, p7, color); DrawLine(p4, p8, color);
        }

        const std::vector<DebugLine>& GetLinesToRender() const { return m_lines; }
        void Clear() { m_lines.clear(); }
    };
}