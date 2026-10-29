#pragma once
#include <cmath>

namespace Newtoon {
    // Alinhamento 16-bytes para futura vetorização SIMD (SSE/AVX)
    struct alignas(16) Vec2 { 
        float x, y; 
        float Magnitude() const { return std::sqrt(x*x + y*y); }
    };
    
    struct alignas(16) Vec3 { 
        float x, y, z; 
        float Magnitude() const { return std::sqrt(x*x + y*y + z*z); }

        float Dot(const Vec3& v) const { return x*v.x + y*v.y + z*v.z; }
        static float Dot(const Vec3& a, const Vec3& b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
            Vec3 Normalize() const {
                float mag = Magnitude();
                if (mag == 0.0f) return {0,0,0};
                return {x / mag, y / mag, z / mag};
            }
            Vec3 operator-(const Vec3& v) const { return {x - v.x, y - v.y, z - v.z}; }
            Vec3 operator+(const Vec3& v) const { return {x + v.x, y + v.y, z + v.z}; }
            Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
    };
    
    struct alignas(16) Quat { 
        float x, y, z, w; 
    };
}