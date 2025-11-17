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

namespace hpl {

	//-----------------------------------------------------------------------
	// SSE2 IMPLEMENTATIONS (128-bit SIMD, guaranteed on x64)
	// These wrapper functions call the inline SSE2 implementations in MathSIMD.h
	// and are used for runtime dispatch via function pointers
	//-----------------------------------------------------------------------

	cMatrixf MatrixMul_SSE2(const cMatrixf& a_mtxA, const cMatrixf& a_mtxB)
	{
		return MatrixMulSSE2(a_mtxA, a_mtxB);
	}

	cVector3f MatrixMulVector_SSE2(const cMatrixf& a_mtxA, const cVector3f& avB)
	{
		return MatrixMulVectorSSE2(a_mtxA, avB);
	}

	float Vector3Dot_SSE2(const cVector3f& avVecA, const cVector3f& avVecB)
	{
		return Vector3DotSSE2(avVecA, avVecB);
	}

	cVector3f Vector3Cross_SSE2(const cVector3f& avVecA, const cVector3f& avVecB)
	{
		return Vector3CrossSSE2(avVecA, avVecB);
	}

	cVector3f Vector3Normalize_SSE2(const cVector3f& avVec)
	{
		return Vector3NormalizeSSE2(avVec);
	}

}; // namespace hpl
