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

#include "dgStdafx.h"
#include "dgMathSIMD.h"
#include "dgVector.h"
#include "dgMatrix.h"
#include "dgDebug.h"

#include <SDL3/SDL_cpuinfo.h>
#include <string.h>

//-----------------------------------------------------------------------
// FUNCTION POINTER DECLARATIONS
//-----------------------------------------------------------------------

// Matrix operations
dgMatrixMulFunc g_dgMatrixMul = nullptr;
dgMatrixInverseFunc g_dgMatrixInverse = nullptr;
dgMatrixRotateVectorFunc g_dgMatrixRotateVector = nullptr;
dgMatrixUnrotateVectorFunc g_dgMatrixUnrotateVector = nullptr;
dgMatrixTransformVectorFunc g_dgMatrixTransformVector = nullptr;

// Vector operations
dgVectorDotProductFunc g_dgVectorDotProduct = nullptr;
dgVectorCrossProductFunc g_dgVectorCrossProduct = nullptr;
dgVectorNormalizeFunc g_dgVectorNormalize = nullptr;

//-----------------------------------------------------------------------
// SIMD OVERRIDE SUPPORT (for debugging/testing)
//-----------------------------------------------------------------------

static const char* g_dgSimdOverride = nullptr;

void dgSetSIMDOverride(const char* level)
{
	g_dgSimdOverride = level;
}

//-----------------------------------------------------------------------
// EXTERNAL IMPLEMENTATION DECLARATIONS
// These are defined in dgMathSIMD_SSE2.cpp, dgMathSIMD_SSE41.cpp, etc.
//-----------------------------------------------------------------------

// SSE2 implementations (always available on x64)
extern dgMatrix dgMatrixMul_SSE2(const dgMatrix& a_mtxA, const dgMatrix& a_mtxB);
extern dgMatrix dgMatrixInverse_SSE2(const dgMatrix& a_mtxA);
extern dgVector dgMatrixRotateVector_SSE2(const dgMatrix& a_mtxA, const dgVector& avB);
extern dgVector dgMatrixUnrotateVector_SSE2(const dgMatrix& a_mtxA, const dgVector& avB);
extern dgVector dgMatrixTransformVector_SSE2(const dgMatrix& a_mtxA, const dgVector& avB);
extern dgFloat32 dgVectorDotProduct_SSE2(const dgVector& avVecA, const dgVector& avVecB);
extern dgVector dgVectorCrossProduct_SSE2(const dgVector& avVecA, const dgVector& avVecB);
extern dgVector dgVectorNormalize_SSE2(const dgVector& avVec);

// SSE4.1 implementations (optional, CPU detected)
extern dgFloat32 dgVectorDotProduct_SSE41(const dgVector& avVecA, const dgVector& avVecB);

// AVX implementations (optional, CPU detected)
extern dgMatrix dgMatrixMul_AVX(const dgMatrix& a_mtxA, const dgMatrix& a_mtxB);
extern dgVector dgMatrixRotateVector_AVX(const dgMatrix& a_mtxA, const dgVector& avB);
extern dgVector dgMatrixUnrotateVector_AVX(const dgMatrix& a_mtxA, const dgVector& avB);
extern dgVector dgMatrixTransformVector_AVX(const dgMatrix& a_mtxA, const dgVector& avB);

// AVX2 implementations (optional, CPU detected - uses FMA)
extern dgMatrix dgMatrixMul_AVX2(const dgMatrix& a_mtxA, const dgMatrix& a_mtxB);
extern dgVector dgMatrixRotateVector_AVX2(const dgMatrix& a_mtxA, const dgVector& avB);
extern dgVector dgMatrixUnrotateVector_AVX2(const dgMatrix& a_mtxA, const dgVector& avB);
extern dgVector dgMatrixTransformVector_AVX2(const dgMatrix& a_mtxA, const dgVector& avB);

// AVX-512 implementations (optional, CPU detected - 512-bit operations)
extern dgMatrix dgMatrixMul_AVX512(const dgMatrix& a_mtxA, const dgMatrix& a_mtxB);
extern dgVector dgMatrixRotateVector_AVX512(const dgMatrix& a_mtxA, const dgVector& avB);
extern dgVector dgMatrixUnrotateVector_AVX512(const dgMatrix& a_mtxA, const dgVector& avB);
extern dgVector dgMatrixTransformVector_AVX512(const dgMatrix& a_mtxA, const dgVector& avB);

//-----------------------------------------------------------------------
// SIMD INITIALIZATION
//-----------------------------------------------------------------------

void dgInitializeSIMD()
{
	dgTrace(("Newton Physics: Initializing SIMD optimizations...\n"));

	// Check for override (debugging/testing)
	if (g_dgSimdOverride)
	{
		if (strcmp(g_dgSimdOverride, "SSE2") == 0)
		{
			dgTrace(("  SIMD Override: Forcing SSE2 baseline\n"));
			g_dgMatrixMul = dgMatrixMul_SSE2;
			g_dgMatrixInverse = dgMatrixInverse_SSE2;
			g_dgMatrixRotateVector = dgMatrixRotateVector_SSE2;
			g_dgMatrixUnrotateVector = dgMatrixUnrotateVector_SSE2;
			g_dgMatrixTransformVector = dgMatrixTransformVector_SSE2;
			g_dgVectorDotProduct = dgVectorDotProduct_SSE2;
			g_dgVectorCrossProduct = dgVectorCrossProduct_SSE2;
			g_dgVectorNormalize = dgVectorNormalize_SSE2;
			dgTrace(("Newton Physics: SIMD initialization complete (forced SSE2).\n"));
			return;
		}
		// Additional overrides can be added here (AVX, AVX2, AVX512)
	}

	// Detect CPU features
	bool bHasSSE2 = SDL_HasSSE2();       // Guaranteed on x64, but check anyway
	bool bHasSSE41 = SDL_HasSSE41();     // Core (2008) and newer
	bool bHasAVX = SDL_HasAVX();         // Sandy Bridge (2011) and newer
	bool bHasAVX2 = SDL_HasAVX2();       // Haswell (2013) and newer
	bool bHasAVX512F = SDL_HasAVX512F(); // Skylake-X (2017), Zen 5 (2024) and newer

	dgTrace(("  CPU Features: SSE2=%d, SSE4.1=%d, AVX=%d, AVX2=%d, AVX-512F=%d\n",
		bHasSSE2, bHasSSE41, bHasAVX, bHasAVX2, bHasAVX512F));

	// Sanity check - SSE2 should always be available on x64
	if (!bHasSSE2)
	{
		dgTrace(("  WARNING: SSE2 not detected! This should not happen on x64 systems.\n"));
		dgTrace(("  WARNING: SIMD optimizations will NOT be available.\n"));
		// Leave function pointers as nullptr - will cause crash if called
		// This is intentional to catch configuration errors early
		return;
	}

	// Select best available implementations
	// Priority: AVX-512F > AVX2 (FMA) > AVX > SSE2

	// Matrix operations: AVX-512F provides best throughput (2.5-3x over SSE2)
	// Note: AVX-512 may cause frequency throttling on some CPUs
	if (bHasAVX512F)
	{
		g_dgMatrixMul = dgMatrixMul_AVX512;
		g_dgMatrixRotateVector = dgMatrixRotateVector_AVX512;
		g_dgMatrixUnrotateVector = dgMatrixUnrotateVector_AVX512;
		g_dgMatrixTransformVector = dgMatrixTransformVector_AVX512;
		dgTrace(("  Matrix operations: Using AVX-512F (512-bit)\n"));
	}
	else if (bHasAVX2)
	{
		g_dgMatrixMul = dgMatrixMul_AVX2;
		g_dgMatrixRotateVector = dgMatrixRotateVector_AVX2;
		g_dgMatrixUnrotateVector = dgMatrixUnrotateVector_AVX2;
		g_dgMatrixTransformVector = dgMatrixTransformVector_AVX2;
		dgTrace(("  Matrix operations: Using AVX2 with FMA (256-bit)\n"));
	}
	else if (bHasAVX)
	{
		g_dgMatrixMul = dgMatrixMul_AVX;
		g_dgMatrixRotateVector = dgMatrixRotateVector_AVX;
		g_dgMatrixUnrotateVector = dgMatrixUnrotateVector_AVX;
		g_dgMatrixTransformVector = dgMatrixTransformVector_AVX;
		dgTrace(("  Matrix operations: Using AVX (256-bit)\n"));
	}
	else
	{
		g_dgMatrixMul = dgMatrixMul_SSE2;
		g_dgMatrixRotateVector = dgMatrixRotateVector_SSE2;
		g_dgMatrixUnrotateVector = dgMatrixUnrotateVector_SSE2;
		g_dgMatrixTransformVector = dgMatrixTransformVector_SSE2;
		dgTrace(("  Matrix operations: Using SSE2 (128-bit)\n"));
	}

	// Vector dot product: SSE4.1 has dedicated instruction
	if (bHasSSE41)
	{
		g_dgVectorDotProduct = dgVectorDotProduct_SSE41;
		dgTrace(("  Vector dot product: Using SSE4.1 (dp instruction)\n"));
	}
	else
	{
		g_dgVectorDotProduct = dgVectorDotProduct_SSE2;
		dgTrace(("  Vector dot product: Using SSE2\n"));
	}

	// Vector cross product and normalize: SSE2 is optimal (no benefit from AVX for 4-component vectors)
	g_dgVectorCrossProduct = dgVectorCrossProduct_SSE2;
	g_dgVectorNormalize = dgVectorNormalize_SSE2;
	dgTrace(("  Vector cross/normalize: Using SSE2\n"));

	// Matrix inverse: Always use SSE2 (no AVX/AVX2/AVX-512 versions implemented yet)
	g_dgMatrixInverse = dgMatrixInverse_SSE2;
	dgTrace(("  Matrix inverse: Using SSE2\n"));

	dgTrace(("Newton Physics: SIMD initialization complete.\n"));
}
