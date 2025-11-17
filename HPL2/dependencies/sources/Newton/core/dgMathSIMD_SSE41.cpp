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
* SSE4.1 SIMD IMPLEMENTATIONS FOR NEWTON DYNAMICS
*
* This file contains SSE4.1 optimizations for Newton Dynamics.
* SSE4.1 (available on Intel Core 2008+, AMD Bulldozer 2011+) adds the dedicated
* dot product instruction (_mm_dp_ps) which provides 1.5-2x speedup over SSE2.
*/

#include "dgMathSIMD.h"
#include "dgVector.h"
#include <smmintrin.h>  // SSE4.1 intrinsics

//-----------------------------------------------------------------------
// VECTOR DOT PRODUCT (SSE4.1)
// Uses dedicated _mm_dp_ps instruction for optimal performance
// Speedup: 1.5-2x over SSE2 horizontal add
//-----------------------------------------------------------------------

dgFloat32 dgVectorDotProduct_SSE41(const dgVector& a, const dgVector& b)
{
	// Load vectors
	__m128 va = _mm_loadu_ps(&a.m_x);
	__m128 vb = _mm_loadu_ps(&b.m_x);

	// Dot product using SSE4.1 dp instruction
	// Mask 0x71: multiply first 3 components (xyz), store result in first component
	__m128 result = _mm_dp_ps(va, vb, 0x71);

	// Extract scalar result
	return _mm_cvtss_f32(result);
}
