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
    };
    
    struct alignas(16) Quat { 
        float x, y, z, w; 
    };
}