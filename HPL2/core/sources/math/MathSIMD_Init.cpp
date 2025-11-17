/*
 * Copyright © 2009-2020 Frictional Games
 *
 * This file is part of Amnesia: The Dark Descent.
 *
 * Amnesia: The Dark Descent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Amnesia: The Dark Descent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: The Dark Descent.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "math/MathSIMD.h"
#include "system/LowLevelSystem.h"

#include <SDL3/SDL_cpuinfo.h>

namespace hpl {

	//-----------------------------------------------------------------------
	// FUNCTION POINTER DECLARATIONS
	//-----------------------------------------------------------------------

	// Matrix operations
	MatrixMulFunc g_MatrixMul = nullptr;
	MatrixMulVectorFunc g_MatrixMulVector = nullptr;

	// Vector3 operations
	Vector3DotFunc g_Vector3Dot = nullptr;
	Vector3CrossFunc g_Vector3Cross = nullptr;
	Vector3NormalizeFunc g_Vector3Normalize = nullptr;

	//-----------------------------------------------------------------------
	// EXTERNAL IMPLEMENTATION DECLARATIONS
	// These are defined in MathSIMD_SSE2.cpp, MathSIMD_SSE41.cpp, MathSIMD_AVX.cpp
	//-----------------------------------------------------------------------

	// SSE2 implementations (always available on x64)
	extern cMatrixf MatrixMul_SSE2(const cMatrixf& a_mtxA, const cMatrixf& a_mtxB);
	extern cVector3f MatrixMulVector_SSE2(const cMatrixf& a_mtxA, const cVector3f& avB);
	extern float Vector3Dot_SSE2(const cVector3f& avVecA, const cVector3f& avVecB);
	extern cVector3f Vector3Cross_SSE2(const cVector3f& avVecA, const cVector3f& avVecB);
	extern cVector3f Vector3Normalize_SSE2(const cVector3f& avVec);

	// SSE4.1 implementations (optional, CPU detected)
	extern float Vector3Dot_SSE41(const cVector3f& avVecA, const cVector3f& avVecB);

	// AVX implementations (optional, CPU detected)
	extern cMatrixf MatrixMul_AVX(const cMatrixf& a_mtxA, const cMatrixf& a_mtxB);
	extern cVector3f MatrixMulVector_AVX(const cMatrixf& a_mtxA, const cVector3f& avB);

	// AVX2 implementations (optional, CPU detected - uses FMA)
	extern cMatrixf MatrixMul_AVX2(const cMatrixf& a_mtxA, const cMatrixf& a_mtxB);
	extern cVector3f MatrixMulVector_AVX2(const cMatrixf& a_mtxA, const cVector3f& avB);

	// AVX-512 implementations (optional, CPU detected - 512-bit operations)
	extern cMatrixf MatrixMul_AVX512(const cMatrixf& a_mtxA, const cMatrixf& a_mtxB);
	extern cVector3f MatrixMulVector_AVX512(const cMatrixf& a_mtxA, const cVector3f& avB);

	//-----------------------------------------------------------------------
	// SIMD INITIALIZATION
	//-----------------------------------------------------------------------

	void InitializeSIMD()
	{
		Log("Initializing SIMD optimizations...\n");

		// Detect CPU features
		bool bHasSSE2 = SDL_HasSSE2();       // Guaranteed on x64, but check anyway
		bool bHasSSE41 = SDL_HasSSE41();     // Core (2008) and newer
		bool bHasAVX = SDL_HasAVX();         // Sandy Bridge (2011) and newer
		bool bHasAVX2 = SDL_HasAVX2();       // Haswell (2013) and newer
		bool bHasAVX512F = SDL_HasAVX512F(); // Skylake-X (2017), Zen 5 (2024) and newer

		Log("  CPU Features: SSE2=%d, SSE4.1=%d, AVX=%d, AVX2=%d, AVX-512F=%d\n",
			bHasSSE2, bHasSSE41, bHasAVX, bHasAVX2, bHasAVX512F);

		// Sanity check - SSE2 should always be available on x64
		if (!bHasSSE2)
		{
			Warning("SSE2 not detected! This should not happen on x64 systems.\n");
			Warning("SIMD optimizations will NOT be available.\n");
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
			g_MatrixMul = MatrixMul_AVX512;
			g_MatrixMulVector = MatrixMulVector_AVX512;
			Log("  Matrix operations: Using AVX-512F (512-bit)\n");
		}
		else if (bHasAVX2)
		{
			g_MatrixMul = MatrixMul_AVX2;
			g_MatrixMulVector = MatrixMulVector_AVX2;
			Log("  Matrix operations: Using AVX2 with FMA (256-bit)\n");
		}
		else if (bHasAVX)
		{
			g_MatrixMul = MatrixMul_AVX;
			g_MatrixMulVector = MatrixMulVector_AVX;
			Log("  Matrix operations: Using AVX (256-bit)\n");
		}
		else
		{
			g_MatrixMul = MatrixMul_SSE2;
			g_MatrixMulVector = MatrixMulVector_SSE2;
			Log("  Matrix operations: Using SSE2 (128-bit)\n");
		}

		// Vector3 dot product: SSE4.1 has dedicated instruction
		if (bHasSSE41)
		{
			g_Vector3Dot = Vector3Dot_SSE41;
			Log("  Vector3 dot product: Using SSE4.1 (dp instruction)\n");
		}
		else
		{
			g_Vector3Dot = Vector3Dot_SSE2;
			Log("  Vector3 dot product: Using SSE2\n");
		}

		// Vector3 cross product and normalize: SSE2 is optimal (no benefit from AVX for 3-component vectors)
		g_Vector3Cross = Vector3Cross_SSE2;
		g_Vector3Normalize = Vector3Normalize_SSE2;
		Log("  Vector3 cross/normalize: Using SSE2\n");

		Log("SIMD initialization complete.\n");
	}

}; // namespace hpl
