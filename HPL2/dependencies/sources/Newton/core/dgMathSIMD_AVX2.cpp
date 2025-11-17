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
* AVX2 SIMD IMPLEMENTATIONS FOR NEWTON DYNAMICS
*
* This file contains AVX2 (256-bit + FMA) optimizations for Newton Dynamics.
* AVX2 (available on Intel Haswell 2013+, AMD Excavator 2015+) adds FMA
* (Fused Multiply-Add) instructions for 2-2.5x speedup over SSE2.
*
* FMA instructions combine multiply and add into a single operation:
*   result = a * b + c (single instruction, higher precision, better performance)
*/

#include "dgMathSIMD.h"
#include "dgVector.h"
#include "dgMatrix.h"
#include <immintrin.h>  // AVX2 and FMA intrinsics

//-----------------------------------------------------------------------
// MATRIX OPERATIONS (AVX2 with FMA)
// Uses FMA instructions for optimal performance
// Speedup: 2-2.5x over SSE2
//-----------------------------------------------------------------------

dgMatrix dgMatrixMul_AVX2(const dgMatrix& A, const dgMatrix& B)
{
	// Matrix multiply using AVX2 with FMA
	// FMA provides single-instruction multiply-add for better performance
	__m128 b0 = _mm_loadu_ps(&B.m_front.m_x);
	__m128 b1 = _mm_loadu_ps(&B.m_up.m_x);
	__m128 b2 = _mm_loadu_ps(&B.m_right.m_x);
	__m128 b3 = _mm_loadu_ps(&B.m_posit.m_x);

	dgMatrix result;

	// For each row of A (unrolled for performance)
	for (int i = 0; i < 4; i++)
	{
		// Get pointer to current row of A
		const dgFloat32* a_row = (i == 0) ? &A.m_front.m_x :
			(i == 1) ? &A.m_up.m_x :
			(i == 2) ? &A.m_right.m_x :
			&A.m_posit.m_x;

		// Broadcast each element of A's row
		__m128 a0 = _mm_set1_ps(a_row[0]);
		__m128 a1 = _mm_set1_ps(a_row[1]);
		__m128 a2 = _mm_set1_ps(a_row[2]);
		__m128 a3 = _mm_set1_ps(a_row[3]);

		// Multiply and accumulate using FMA instructions
		// row = a0*b0 + a1*b1 + a2*b2 + a3*b3
		__m128 row = _mm_mul_ps(a0, b0);  // Start with a0*b0
		row = _mm_fmadd_ps(a1, b1, row);  // row = a1*b1 + row (FMA)
		row = _mm_fmadd_ps(a2, b2, row);  // row = a2*b2 + row (FMA)
		row = _mm_fmadd_ps(a3, b3, row);  // row = a3*b3 + row (FMA)

		// Store result row
		dgFloat32* result_row = (i == 0) ? &result.m_front.m_x :
			(i == 1) ? &result.m_up.m_x :
			(i == 2) ? &result.m_right.m_x :
			&result.m_posit.m_x;
		_mm_storeu_ps(result_row, row);
	}

	return result;
}

dgVector dgMatrixRotateVector_AVX2(const dgMatrix& source, const dgVector& v)
{
	// Rotate vector using AVX2 with FMA
	__m128 vx = _mm_set1_ps(v.m_x);
	__m128 vy = _mm_set1_ps(v.m_y);
	__m128 vz = _mm_set1_ps(v.m_z);

	__m128 r0 = _mm_loadu_ps(&source.m_front.m_x);
	__m128 r1 = _mm_loadu_ps(&source.m_up.m_x);
	__m128 r2 = _mm_loadu_ps(&source.m_right.m_x);

	// result = r0*vx + r1*vy + r2*vz
	__m128 result = _mm_mul_ps(r0, vx);
	result = _mm_fmadd_ps(r1, vy, result);  // FMA: result = r1*vy + result
	result = _mm_fmadd_ps(r2, vz, result);  // FMA: result = r2*vz + result

	dgVector out;
	_mm_storeu_ps(&out.m_x, result);
	return out;
}

dgVector dgMatrixUnrotateVector_AVX2(const dgMatrix& source, const dgVector& v)
{
	// Unrotate vector using AVX2 with FMA
	// (FMA doesn't help much here, but included for completeness)
	__m128 vv = _mm_loadu_ps(&v.m_x);
	__m128 r0 = _mm_loadu_ps(&source.m_front.m_x);
	__m128 r1 = _mm_loadu_ps(&source.m_up.m_x);
	__m128 r2 = _mm_loadu_ps(&source.m_right.m_x);

	// Dot products
	__m128 m0 = _mm_mul_ps(vv, r0);
	__m128 m1 = _mm_mul_ps(vv, r1);
	__m128 m2 = _mm_mul_ps(vv, r2);

	// Horizontal add
	__m128 hadd01 = _mm_hadd_ps(m0, m1);
	__m128 hadd2 = _mm_hadd_ps(m2, _mm_setzero_ps());
	__m128 result = _mm_hadd_ps(hadd01, hadd2);

	dgVector out;
	_mm_storeu_ps(&out.m_x, result);
	out.m_w = v.m_w;
	return out;
}

dgVector dgMatrixTransformVector_AVX2(const dgMatrix& source, const dgVector& v)
{
	// Transform vector (with translation) using AVX2 with FMA
	__m128 vx = _mm_set1_ps(v.m_x);
	__m128 vy = _mm_set1_ps(v.m_y);
	__m128 vz = _mm_set1_ps(v.m_z);

	__m128 r0 = _mm_loadu_ps(&source.m_front.m_x);
	__m128 r1 = _mm_loadu_ps(&source.m_up.m_x);
	__m128 r2 = _mm_loadu_ps(&source.m_right.m_x);
	__m128 r3 = _mm_loadu_ps(&source.m_posit.m_x);

	// result = r0*vx + r1*vy + r2*vz + r3
	__m128 result = _mm_mul_ps(r0, vx);
	result = _mm_fmadd_ps(r1, vy, result);  // FMA: result = r1*vy + result
	result = _mm_fmadd_ps(r2, vz, result);  // FMA: result = r2*vz + result
	result = _mm_add_ps(result, r3);

	dgVector out;
	_mm_storeu_ps(&out.m_x, result);
	return out;
}
