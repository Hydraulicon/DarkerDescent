/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source distribution.
*/

/*
* SIMD RUNTIME DISPATCH SYSTEM FOR NEWTON DYNAMICS
*
* This header provides runtime CPU detection and multi-tier SIMD optimization
* for Newton Dynamics physics engine. Mirrors the HPL2 SIMD architecture but
* remains completely independent from HPL2 internals.
*
* Supported instruction sets (runtime-dispatched):
*   - AVX-512F: 512-bit operations (Intel Skylake-X 2017+, AMD Zen 5 2024+)
*   - AVX2:     256-bit + FMA (Intel Haswell 2013+, AMD Excavator 2015+)
*   - AVX:      256-bit operations (Intel Sandy Bridge 2011+, AMD Bulldozer 2011+)
*   - SSE4.1:   Dedicated dot product instruction (Intel Core 2008+)
*   - SSE2:     128-bit baseline (guaranteed on x64)
*
* Usage:
*   1. Call dgInitializeSIMD() once at Newton world creation
*   2. Use inline wrapper functions (e.g., dgMatrixMul_SIMD()) for zero-overhead dispatch
*   3. Optionally call dgSetSIMDOverride() to force specific instruction set
*/

#ifndef DG_MATH_SIMD_H
#define DG_MATH_SIMD_H

#include "dgStdafx.h"
#include "dgTypes.h"

// Forward declarations
class dgVector;
class dgMatrix;

//-----------------------------------------------------------------------
// SIMD DISPATCH SYSTEM
// Runtime CPU detection selects best available instruction set:
// AVX-512F (512-bit, 2017+) > AVX2 (FMA, 2013+) > AVX (256-bit, 2011+) > SSE4.1 (dp, 2008+) > SSE2 (128-bit)
//-----------------------------------------------------------------------

// Function pointer typedefs for runtime dispatch
typedef dgMatrix (*dgMatrixMulFunc)(const dgMatrix&, const dgMatrix&);
typedef dgMatrix (*dgMatrixInverseFunc)(const dgMatrix&);
typedef dgVector (*dgMatrixRotateVectorFunc)(const dgMatrix&, const dgVector&);
typedef dgVector (*dgMatrixUnrotateVectorFunc)(const dgMatrix&, const dgVector&);
typedef dgVector (*dgMatrixTransformVectorFunc)(const dgMatrix&, const dgVector&);
typedef dgFloat32 (*dgVectorDotProductFunc)(const dgVector&, const dgVector&);
typedef dgVector (*dgVectorCrossProductFunc)(const dgVector&, const dgVector&);
typedef dgVector (*dgVectorNormalizeFunc)(const dgVector&);

// Global function pointers (initialized by dgInitializeSIMD)
extern dgMatrixMulFunc g_dgMatrixMul;
extern dgMatrixInverseFunc g_dgMatrixInverse;
extern dgMatrixRotateVectorFunc g_dgMatrixRotateVector;
extern dgMatrixUnrotateVectorFunc g_dgMatrixUnrotateVector;
extern dgMatrixTransformVectorFunc g_dgMatrixTransformVector;
extern dgVectorDotProductFunc g_dgVectorDotProduct;
extern dgVectorCrossProductFunc g_dgVectorCrossProduct;
extern dgVectorNormalizeFunc g_dgVectorNormalize;

// Initialize SIMD system (call once at Newton world creation, after SDL_Init)
void dgInitializeSIMD();

// Set SIMD instruction set override (for debugging/testing)
// Valid values: "SSE2", "SSE41", "AVX", "AVX2", "AVX512", nullptr (auto-detect)
// Must be called BEFORE dgInitializeSIMD()
void dgSetSIMDOverride(const char* level);

#endif // DG_MATH_SIMD_H
