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
* SSE2 SIMD IMPLEMENTATIONS FOR NEWTON DYNAMICS
*
* This file contains SSE2 (128-bit) baseline implementations of physics math operations.
* These serve as the fallback implementation, guaranteed to be available on all x64 systems.
*
* Implementations are refactored from Newton's existing inline SIMD methods to allow
* runtime dispatch via function pointers.
*/

#include "dgMathSIMD.h"
#include "dgVector.h"
#include "dgMatrix.h"
#include "dgSimd_Instrutions.h"

//-----------------------------------------------------------------------
// MATRIX OPERATIONS (SSE2)
//-----------------------------------------------------------------------

dgMatrix dgMatrixMul_SSE2(const dgMatrix& A, const dgMatrix& B)
{
	// Matrix multiply using SSE2 intrinsics
	// Refactored from dgMatrix::MultiplySimd (dgMatrix.h:376-412)
	return dgMatrix(
		dgVector(simd_mul_add_v(
			simd_mul_add_v(
				simd_mul_add_v(simd_mul_v((simd_type&)B[0], simd_permut_v((simd_type&)A[0], (simd_type&)A[0], PURMUT_MASK(0, 0, 0, 0))),
					(simd_type&)B[1], simd_permut_v((simd_type&)A[0], (simd_type&)A[0], PURMUT_MASK(1, 1, 1, 1))),
				(simd_type&)B[2], simd_permut_v((simd_type&)A[0], (simd_type&)A[0], PURMUT_MASK(2, 2, 2, 2))),
			(simd_type&)B[3], simd_permut_v((simd_type&)A[0], (simd_type&)A[0], PURMUT_MASK(3, 3, 3, 3)))),

		dgVector(simd_mul_add_v(
			simd_mul_add_v(
				simd_mul_add_v(simd_mul_v((simd_type&)B[0], simd_permut_v((simd_type&)A[1], (simd_type&)A[1], PURMUT_MASK(0, 0, 0, 0))),
					(simd_type&)B[1], simd_permut_v((simd_type&)A[1], (simd_type&)A[1], PURMUT_MASK(1, 1, 1, 1))),
				(simd_type&)B[2], simd_permut_v((simd_type&)A[1], (simd_type&)A[1], PURMUT_MASK(2, 2, 2, 2))),
			(simd_type&)B[3], simd_permut_v((simd_type&)A[1], (simd_type&)A[1], PURMUT_MASK(3, 3, 3, 3)))),

		dgVector(simd_mul_add_v(
			simd_mul_add_v(
				simd_mul_add_v(simd_mul_v((simd_type&)B[0], simd_permut_v((simd_type&)A[2], (simd_type&)A[2], PURMUT_MASK(0, 0, 0, 0))),
					(simd_type&)B[1], simd_permut_v((simd_type&)A[2], (simd_type&)A[2], PURMUT_MASK(1, 1, 1, 1))),
				(simd_type&)B[2], simd_permut_v((simd_type&)A[2], (simd_type&)A[2], PURMUT_MASK(2, 2, 2, 2))),
			(simd_type&)B[3], simd_permut_v((simd_type&)A[2], (simd_type&)A[2], PURMUT_MASK(3, 3, 3, 3)))),

		dgVector(simd_mul_add_v(
			simd_mul_add_v(
				simd_mul_add_v(simd_mul_v((simd_type&)B[0], simd_permut_v((simd_type&)A[3], (simd_type&)A[3], PURMUT_MASK(0, 0, 0, 0))),
					(simd_type&)B[1], simd_permut_v((simd_type&)A[3], (simd_type&)A[3], PURMUT_MASK(1, 1, 1, 1))),
				(simd_type&)B[2], simd_permut_v((simd_type&)A[3], (simd_type&)A[3], PURMUT_MASK(2, 2, 2, 2))),
			(simd_type&)B[3], simd_permut_v((simd_type&)A[3], (simd_type&)A[3], PURMUT_MASK(3, 3, 3, 3))))
	);
}

dgMatrix dgMatrixInverse_SSE2(const dgMatrix& source)
{
	// Matrix inverse using SSE2 intrinsics
	// Refactored from dgMatrix::InverseSimd (dgMatrix.h:342-374)
	simd_type r0;
	simd_type r1;
	simd_type r2;
	dgMatrix matrix;

	r2 = simd_set1(dgFloat32(0.0f));
	r0 = simd_pack_lo_v((simd_type&)source[0], (simd_type&)source[1]);
	r1 = simd_pack_lo_v((simd_type&)source[2], r2);
	(simd_type&)matrix[0] = simd_move_lh_v(r0, r1);
	(simd_type&)matrix[1] = simd_move_hl_v(r1, r0);
	r0 = simd_pack_hi_v((simd_type&)source[0], (simd_type&)source[1]);
	r1 = simd_pack_hi_v((simd_type&)source[2], r2);
	(simd_type&)matrix[2] = simd_move_lh_v(r0, r1);

	(simd_type&)matrix[3] = simd_sub_v(r2,
		simd_mul_add_v(
			simd_mul_add_v(simd_mul_v((simd_type&)matrix[0], simd_permut_v((simd_type&)source[3], (simd_type&)source[3], PURMUT_MASK(3, 0, 0, 0))),
				(simd_type&)matrix[1], simd_permut_v((simd_type&)source[3], (simd_type&)source[3], PURMUT_MASK(3, 1, 1, 1))),
			(simd_type&)matrix[2], simd_permut_v((simd_type&)source[3], (simd_type&)source[3], PURMUT_MASK(3, 2, 2, 2))));
	matrix[3][3] = dgFloat32(1.0f);

	return matrix;
}

dgVector dgMatrixRotateVector_SSE2(const dgMatrix& source, const dgVector& v)
{
	// Rotate vector using SSE2 intrinsics
	// Refactored from dgMatrix::RotateVectorSimd (dgMatrix.h:318-331)
	return dgVector(simd_mul_add_v(
		simd_mul_add_v(
			simd_mul_v((simd_type&)source[0], simd_permut_v((simd_type&)v, (simd_type&)v, PURMUT_MASK(0, 0, 0, 0))),
			(simd_type&)source[1], simd_permut_v((simd_type&)v, (simd_type&)v, PURMUT_MASK(1, 1, 1, 1))),
		(simd_type&)source[2], simd_permut_v((simd_type&)v, (simd_type&)v, PURMUT_MASK(2, 2, 2, 2))));
}

dgVector dgMatrixUnrotateVector_SSE2(const dgMatrix& source, const dgVector& v)
{
	// Unrotate vector using SSE2 intrinsics
	// Refactored from dgMatrix::UnrotateVectorSimd (dgMatrix.h:333-340)
	return dgVector(v.DotProductSimd(source.m_front), v.DotProductSimd(source.m_up), v.DotProductSimd(source.m_right), v.m_w);
}

dgVector dgMatrixTransformVector_SSE2(const dgMatrix& source, const dgVector& v)
{
	// Transform vector (with translation) using SSE2 intrinsics
	// Refactored from dgMatrix::TransformVectorSimd (dgMatrix.h:288-300)
	return dgVector(simd_mul_add_v(
		simd_mul_add_v(
			simd_mul_add_v((simd_type&)source[3], (simd_type&)source[0], simd_permut_v((simd_type&)v, (simd_type&)v, PURMUT_MASK(0, 0, 0, 0))),
			(simd_type&)source[1], simd_permut_v((simd_type&)v, (simd_type&)v, PURMUT_MASK(1, 1, 1, 1))),
		(simd_type&)source[2], simd_permut_v((simd_type&)v, (simd_type&)v, PURMUT_MASK(2, 2, 2, 2))));
}

//-----------------------------------------------------------------------
// VECTOR OPERATIONS (SSE2)
//-----------------------------------------------------------------------

dgFloat32 dgVectorDotProduct_SSE2(const dgVector& a, const dgVector& b)
{
	// Dot product using SSE2 intrinsics
	// Refactored from dgVector::DotProductSimd (dgVector.h:324-338)
	dgVector tmp;
	(simd_type&)tmp = simd_mul_v((simd_type&)a, (simd_type&)b);
	return tmp.m_x + tmp.m_y + tmp.m_z;
}

dgVector dgVectorCrossProduct_SSE2(const dgVector& e21, const dgVector& e10)
{
	// Cross product using SSE2 intrinsics
	// Refactored from dgVector::CrossProductSimd (dgVector.h:340-349)
	return dgVector(simd_mul_sub_v(simd_mul_v(simd_permut_v((simd_type&)e21, (simd_type&)e21, PURMUT_MASK(3, 0, 2, 1)), simd_permut_v((simd_type&)e10, (simd_type&)e10, PURMUT_MASK(3, 1, 0, 2))),
		simd_permut_v((simd_type&)e21, (simd_type&)e21, PURMUT_MASK(3, 1, 0, 2)), simd_permut_v((simd_type&)e10, (simd_type&)e10, PURMUT_MASK(3, 0, 2, 1))));
}

dgVector dgVectorNormalize_SSE2(const dgVector& v)
{
	// Normalize vector using SSE2 intrinsics (fast reciprocal square root)
	// Calculate magnitude squared
	dgVector tmp;
	(simd_type&)tmp = simd_mul_v((simd_type&)v, (simd_type&)v);
	dgFloat32 mag2 = tmp.m_x + tmp.m_y + tmp.m_z;

	// Fast reciprocal square root
	simd_type mag2_simd = simd_set1(mag2);
	simd_type rsqrt = simd_rsqrt_v(mag2_simd);

	// Normalize
	return dgVector(simd_mul_v((simd_type&)v, rsqrt));
}
