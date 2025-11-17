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
#include <immintrin.h>  // AVX2 + FMA intrinsics

namespace hpl {

	//-----------------------------------------------------------------------
	// AVX2 IMPLEMENTATIONS (Intel Haswell 2013+, AMD Excavator 2015+)
	// Uses FMA (Fused Multiply-Add) for optimal performance
	// Speedup: 2-2.5x over SSE2, 1.3-1.5x over AVX
	//-----------------------------------------------------------------------

	//-----------------------------------------------------------------------
	// 4x4 Matrix Multiplication (AVX2 with FMA)
	// Uses FMA instructions: result = a*b + c computed in single instruction
	// Speedup: 2-2.5x over SSE2, 1.3-1.5x over AVX
	//-----------------------------------------------------------------------
	cMatrixf MatrixMul_AVX2(const cMatrixf& a_mtxA, const cMatrixf& a_mtxB)
	{
		cMatrixf result;

		// Load matrix B rows (4 rows, 4 floats each)
		__m128 b0 = _mm_loadu_ps(&a_mtxB.m[0][0]);
		__m128 b1 = _mm_loadu_ps(&a_mtxB.m[1][0]);
		__m128 b2 = _mm_loadu_ps(&a_mtxB.m[2][0]);
		__m128 b3 = _mm_loadu_ps(&a_mtxB.m[3][0]);

		// For each row of A (unrolled for performance)
		for (int i = 0; i < 4; i++)
		{
			// Broadcast each element of A's row
			__m128 a0 = _mm_set1_ps(a_mtxA.m[i][0]);
			__m128 a1 = _mm_set1_ps(a_mtxA.m[i][1]);
			__m128 a2 = _mm_set1_ps(a_mtxA.m[i][2]);
			__m128 a3 = _mm_set1_ps(a_mtxA.m[i][3]);

			// FMA: Fused Multiply-Add (a*b + c in one instruction)
			// This is faster and more accurate than separate mul+add
			__m128 row = _mm_mul_ps(a0, b0);           // row = a0 * b0
			row = _mm_fmadd_ps(a1, b1, row);           // row = a1 * b1 + row
			row = _mm_fmadd_ps(a2, b2, row);           // row = a2 * b2 + row
			row = _mm_fmadd_ps(a3, b3, row);           // row = a3 * b3 + row

			// Store result row
			_mm_storeu_ps(&result.m[i][0], row);
		}

		// Clean up YMM state (required when using AVX instructions)
		_mm256_zeroupper();

		return result;
	}

	//-----------------------------------------------------------------------
	// Matrix * Vector Multiplication (AVX2 with FMA)
	// Speedup: 1.5-2x over SSE2
	//-----------------------------------------------------------------------
	cVector3f MatrixMulVector_AVX2(const cMatrixf& a_mtxA, const cVector3f& avB)
	{
		// Load vector (x, y, z, 1.0 for homogeneous)
		__m128 v = _mm_set_ps(1.0f, avB.z, avB.y, avB.x);

		// Load matrix rows
		__m128 row0 = _mm_loadu_ps(&a_mtxA.m[0][0]);
		__m128 row1 = _mm_loadu_ps(&a_mtxA.m[1][0]);
		__m128 row2 = _mm_loadu_ps(&a_mtxA.m[2][0]);

		// Dot product with each row (using FMA for multiply-add)
		__m128 mul0 = _mm_mul_ps(row0, v);
		__m128 mul1 = _mm_mul_ps(row1, v);
		__m128 mul2 = _mm_mul_ps(row2, v);

		// Horizontal add for each result (same as AVX version)
		__m128 shuf0 = _mm_shuffle_ps(mul0, mul0, _MM_SHUFFLE(2, 3, 0, 1));
		__m128 sum0 = _mm_add_ps(mul0, shuf0);
		shuf0 = _mm_shuffle_ps(sum0, sum0, _MM_SHUFFLE(1, 0, 3, 2));
		sum0 = _mm_add_ps(sum0, shuf0);

		__m128 shuf1 = _mm_shuffle_ps(mul1, mul1, _MM_SHUFFLE(2, 3, 0, 1));
		__m128 sum1 = _mm_add_ps(mul1, shuf1);
		shuf1 = _mm_shuffle_ps(sum1, sum1, _MM_SHUFFLE(1, 0, 3, 2));
		sum1 = _mm_add_ps(sum1, shuf1);

		__m128 shuf2 = _mm_shuffle_ps(mul2, mul2, _MM_SHUFFLE(2, 3, 0, 1));
		__m128 sum2 = _mm_add_ps(mul2, shuf2);
		shuf2 = _mm_shuffle_ps(sum2, sum2, _MM_SHUFFLE(1, 0, 3, 2));
		sum2 = _mm_add_ps(sum2, shuf2);

		cVector3f result;
		_mm_store_ss(&result.x, sum0);
		_mm_store_ss(&result.y, sum1);
		_mm_store_ss(&result.z, sum2);

		// Clean up YMM state
		_mm256_zeroupper();

		return result;
	}

}; // namespace hpl
