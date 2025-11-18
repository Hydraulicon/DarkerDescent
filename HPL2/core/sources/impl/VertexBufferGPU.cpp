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

#include "impl/VertexBufferGPU.h"
#include "impl/LowLevelGraphicsGPU.h"
#include "system/LowLevelSystem.h"
#include "math/Math.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// ELEMENT ARRAY
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cVtxBufferGPUElementArray::cVtxBufferGPUElementArray(eVertexBufferElementFormat aFormat)
		: mFormat(aFormat), mlElementNum(0), mlProgramVarIndex(0)
	{
		mpByteArray = nullptr;
		mpIntArray = nullptr;
		mpFloatArray = nullptr;

		if (aFormat == eVertexBufferElementFormat_Byte)
			mpByteArray = hplNew(tByteVec, ());
		else if (aFormat == eVertexBufferElementFormat_Int)
			mpIntArray = hplNew(tIntVec, ());
		else if (aFormat == eVertexBufferElementFormat_Float)
			mpFloatArray = hplNew(tFloatVec, ());
	}

	//-----------------------------------------------------------------------

	cVtxBufferGPUElementArray::~cVtxBufferGPUElementArray()
	{
		if (mpByteArray) hplDelete(mpByteArray);
		if (mpIntArray) hplDelete(mpIntArray);
		if (mpFloatArray) hplDelete(mpFloatArray);
	}

	//-----------------------------------------------------------------------

	void cVtxBufferGPUElementArray::Reserve(size_t alSize)
	{
		if (mpByteArray) mpByteArray->reserve(alSize);
		else if (mpIntArray) mpIntArray->reserve(alSize);
		else if (mpFloatArray) mpFloatArray->reserve(alSize);
	}

	//-----------------------------------------------------------------------

	void cVtxBufferGPUElementArray::Resize(size_t alSize)
	{
		if (mpByteArray) mpByteArray->resize(alSize);
		else if (mpIntArray) mpIntArray->resize(alSize);
		else if (mpFloatArray) mpFloatArray->resize(alSize);
	}

	//-----------------------------------------------------------------------

	void cVtxBufferGPUElementArray::PushBack(const void *apData)
	{
		if (mpByteArray)
		{
			unsigned char *pData = (unsigned char*)apData;
			for (int i = 0; i < mlElementNum; i++) mpByteArray->push_back(pData[i]);
		}
		else if (mpIntArray)
		{
			int *pData = (int*)apData;
			for (int i = 0; i < mlElementNum; i++) mpIntArray->push_back(pData[i]);
		}
		else if (mpFloatArray)
		{
			float *pData = (float*)apData;
			for (int i = 0; i < mlElementNum; i++) mpFloatArray->push_back(pData[i]);
		}
	}

	//-----------------------------------------------------------------------

	size_t cVtxBufferGPUElementArray::Size()
	{
		if (mpByteArray) return mpByteArray->size() / mlElementNum;
		else if (mpIntArray) return mpIntArray->size() / mlElementNum;
		else if (mpFloatArray) return mpFloatArray->size() / mlElementNum;
		return 0;
	}

	//-----------------------------------------------------------------------

	void* cVtxBufferGPUElementArray::GetArrayPtr()
	{
		if (mpByteArray) return &(*mpByteArray)[0];
		else if (mpIntArray) return &(*mpIntArray)[0];
		else if (mpFloatArray) return &(*mpFloatArray)[0];
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cVertexBufferGPU::cVertexBufferGPU(	iLowLevelGraphics* apLowLevelGraphics,
										eVertexBufferType aType,
										eVertexBufferDrawType aDrawType, eVertexBufferUsageType aUsageType,
										size_t alReserveVtxSize, size_t alReserveIdxSize)
		: iVertexBuffer(apLowLevelGraphics, aType, aDrawType, aUsageType, alReserveVtxSize, alReserveIdxSize)
	{
		mbHasShadowDouble = false;
		mbCompiled = false;
		mpVertexBuffer = nullptr;
		mpIndexBuffer = nullptr;

		for (int i = 0; i < eVertexBufferElement_LastEnum; i++)
		{
			mvElementArrayIndex[i] = -1;
		}

		if (alReserveIdxSize > 0)
			mvIndexArray.reserve(alReserveIdxSize);
	}

	//-----------------------------------------------------------------------

	cVertexBufferGPU::~cVertexBufferGPU()
	{
		for (size_t i = 0; i < mvElementArrays.size(); i++)
		{
			hplDelete(mvElementArrays[i]);
		}

		// TODO: Release SDL_GPU buffers when implemented
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::CreateElementArray(	eVertexBufferElement aType, eVertexBufferElementFormat aFormat,
												int alElementNum, int alProgramVarIndex)
	{
		if (mvElementArrayIndex[aType] >= 0)
		{
			Error("Element array type %d already exists!\n", aType);
			return;
		}

		cVtxBufferGPUElementArray *pArray = hplNew(cVtxBufferGPUElementArray, (aFormat));

		pArray->mType = aType;
		pArray->mFlag = eVertexElementFlag_Position << aType;
		pArray->mlElementNum = alElementNum;
		pArray->mlProgramVarIndex = alProgramVarIndex;

		if (mlReservedVtxSize > 0)
			pArray->Reserve(mlReservedVtxSize * alElementNum);

		mvElementArrayIndex[aType] = (char)mvElementArrays.size();
		mvElementArrays.push_back(pArray);

		mVertexFlags |= pArray->mFlag;
	}

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::AddVertexVec3f(eVertexBufferElement aElement, const cVector3f& avVtx)
	{
		cVtxBufferGPUElementArray *pArray = GetElementArray(aElement);
		if (pArray == nullptr) return;

		float vVtx[3] = { avVtx.x, avVtx.y, avVtx.z };
		pArray->PushBack(vVtx);
	}

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::AddVertexVec4f(eVertexBufferElement aElement, const cVector3f& avVtx, float afW)
	{
		cVtxBufferGPUElementArray *pArray = GetElementArray(aElement);
		if (pArray == nullptr) return;

		float vVtx[4] = { avVtx.x, avVtx.y, avVtx.z, afW };
		pArray->PushBack(vVtx);
	}

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::AddVertexColor(eVertexBufferElement aElement, const cColor& aColor)
	{
		cVtxBufferGPUElementArray *pArray = GetElementArray(aElement);
		if (pArray == nullptr) return;

		float vColor[4] = { aColor.r, aColor.g, aColor.b, aColor.a };
		pArray->PushBack(vColor);
	}

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::AddIndex(unsigned int alIndex)
	{
		mvIndexArray.push_back(alIndex);
	}

	//-----------------------------------------------------------------------

	bool cVertexBufferGPU::Compile(tVertexCompileFlag aFlags)
	{
		if (mbCompiled)
		{
			Warning("Vertex buffer already compiled!\n");
			return true;
		}

		// TODO: Create actual SDL_GPU buffers and upload data
		// For now, just mark as compiled
		mbCompiled = true;
		return true;
	}

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::UpdateData(tVertexElementFlag aTypes, bool abIndices)
	{
		// TODO: Update GPU buffers with new data
	}

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::CreateShadowDouble(bool abUpdateData)
	{
		if (mbHasShadowDouble) return;

		cVtxBufferGPUElementArray *pPosElement = GetElementArray(eVertexBufferElement_Position);
		if (pPosElement == nullptr || pPosElement->mFormat != eVertexBufferElementFormat_Float)
			return;

		// Set to new size (double the current size)
		int lSize = (int)pPosElement->Size();
		pPosElement->Reserve(lSize * 2);

		float *pPosArray = (float*)pPosElement->GetArrayPtr();

		// Assuming 4 floats per vertex (vec4)
		int lCount = lSize / 4;
		int lZero = 0;
		for (int i = 0; i < lCount; i++)
		{
			pPosElement->PushBack(&pPosArray[i * 4 + 0]);
			pPosElement->PushBack(&pPosArray[i * 4 + 1]);
			pPosElement->PushBack(&pPosArray[i * 4 + 2]);
			pPosElement->PushBack(&lZero);
		}

		mbHasShadowDouble = true;

		if (abUpdateData)
		{
			UpdateData(eVertexElementFlag_Position, false);
		}
	}

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::Transform(const cMatrixf &mtxTransform)
	{
		cVtxBufferGPUElementArray *pPosArray = GetElementArray(eVertexBufferElement_Position);
		if (pPosArray == nullptr || pPosArray->mFormat != eVertexBufferElementFormat_Float)
			return;

		int lPosNum = pPosArray->mlElementNum;
		float *pPosData = pPosArray->mpFloatArray->data();
		size_t lVtxCount = pPosArray->Size();

		for (size_t i = 0; i < lVtxCount; i++)
		{
			cVector3f vPos;
			vPos.x = pPosData[i * lPosNum + 0];
			vPos.y = pPosData[i * lPosNum + 1];
			vPos.z = pPosData[i * lPosNum + 2];

			vPos = cMath::MatrixMul(mtxTransform, vPos);

			pPosData[i * lPosNum + 0] = vPos.x;
			pPosData[i * lPosNum + 1] = vPos.y;
			pPosData[i * lPosNum + 2] = vPos.z;
		}
	}

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::Draw(eVertexBufferDrawType aDrawType)
	{
		// TODO: Implement actual drawing with SDL_GPU
	}

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::DrawIndices(unsigned int *apIndices, int alCount,
										eVertexBufferDrawType aDrawType)
	{
		// TODO: Implement indexed drawing with SDL_GPU
	}

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::Bind()
	{
		// TODO: Bind vertex buffer for rendering
	}

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::UnBind()
	{
		// TODO: Unbind vertex buffer
	}

	//-----------------------------------------------------------------------

	iVertexBuffer* cVertexBufferGPU::CreateCopy(eVertexBufferType aType, eVertexBufferUsageType aUsageType,
												tVertexElementFlag alVtxToCopy)
	{
		if (alVtxToCopy == eFlagBit_All) alVtxToCopy = mVertexFlags;

		cVertexBufferGPU *pVtxBuff = hplNew(cVertexBufferGPU, (mpLowLevelGraphics, aType, mDrawType, aUsageType, GetVertexNum(), GetIndexNum()));

		// Copy the element arrays to the new buffer
		for (size_t i = 0; i < mvElementArrays.size(); ++i)
		{
			cVtxBufferGPUElementArray *pSrcElement = mvElementArrays[i];
			if ((pSrcElement->mFlag & alVtxToCopy) == 0) continue;

			pVtxBuff->CreateElementArray(pSrcElement->mType, pSrcElement->mFormat, pSrcElement->mlElementNum, pSrcElement->mlProgramVarIndex);
			cVtxBufferGPUElementArray *pDestElement = pVtxBuff->GetElementArray(pSrcElement->mType);

			pDestElement->Resize(pSrcElement->Size());

			memcpy(pDestElement->GetArrayPtr(), pSrcElement->GetArrayPtr(),
				pSrcElement->Size() * GetVertexFormatByteSize(pSrcElement->mFormat));
		}

		// Copy indices to the new buffer
		pVtxBuff->ResizeIndices(static_cast<int>(GetIndexNum()));
		memcpy(pVtxBuff->GetIndices(), GetIndices(), GetIndexNum() * sizeof(unsigned int));

		pVtxBuff->mbTangents = mbTangents;
		pVtxBuff->mbHasShadowDouble = mbHasShadowDouble;

		pVtxBuff->Compile(0);

		return pVtxBuff;
	}

	//-----------------------------------------------------------------------

	cBoundingVolume cVertexBufferGPU::CreateBoundingVolume()
	{
		cVtxBufferGPUElementArray *pPosArray = GetElementArray(eVertexBufferElement_Position);
		if (pPosArray == nullptr)
			return cBoundingVolume();

		cVector3f vMax(-100000.0f);
		cVector3f vMin(100000.0f);

		float *pPosData = (float*)pPosArray->GetArrayPtr();
		size_t lVtxCount = pPosArray->Size();
		int lPosNum = pPosArray->mlElementNum;

		for (size_t i = 0; i < lVtxCount; i++)
		{
			cVector3f vPos;
			vPos.x = pPosData[i * lPosNum + 0];
			vPos.y = pPosData[i * lPosNum + 1];
			vPos.z = pPosData[i * lPosNum + 2];

			if (vMax.x < vPos.x) vMax.x = vPos.x;
			if (vMin.x > vPos.x) vMin.x = vPos.x;

			if (vMax.y < vPos.y) vMax.y = vPos.y;
			if (vMin.y > vPos.y) vMin.y = vPos.y;

			if (vMax.z < vPos.z) vMax.z = vPos.z;
			if (vMin.z > vPos.z) vMin.z = vPos.z;
		}

		cBoundingVolume boundingVolume;
		boundingVolume.AddArrayPoints(pPosData, (int)lVtxCount);
		boundingVolume.CreateFromPoints(lPosNum);

		return boundingVolume;
	}

	//-----------------------------------------------------------------------

	size_t cVertexBufferGPU::GetVertexNum()
	{
		cVtxBufferGPUElementArray *pPosElement = GetElementArray(eVertexBufferElement_Position);
		if (pPosElement == nullptr) return 0;

		size_t lSize = pPosElement->Size() / pPosElement->mlElementNum;

		// If there is a shadow double, just return the length of the first half
		if (mbHasShadowDouble)	return lSize / 2;
		else					return lSize;
	}

	//-----------------------------------------------------------------------

	size_t cVertexBufferGPU::GetIndexNum()
	{
		return mvIndexArray.size();
	}

	//-----------------------------------------------------------------------

	int cVertexBufferGPU::GetElementNum(eVertexBufferElement aElement)
	{
		cVtxBufferGPUElementArray *pArray = GetElementArray(aElement);
		if (pArray == nullptr) return 0;
		return pArray->mlElementNum;
	}

	//-----------------------------------------------------------------------

	eVertexBufferElementFormat cVertexBufferGPU::GetElementFormat(eVertexBufferElement aElement)
	{
		cVtxBufferGPUElementArray *pArray = GetElementArray(aElement);
		if (pArray == nullptr) return eVertexBufferElementFormat_Float;
		return pArray->mFormat;
	}

	//-----------------------------------------------------------------------

	int cVertexBufferGPU::GetElementProgramVarIndex(eVertexBufferElement aElement)
	{
		cVtxBufferGPUElementArray *pArray = GetElementArray(aElement);
		if (pArray == nullptr) return -1;
		return pArray->mlProgramVarIndex;
	}

	//-----------------------------------------------------------------------

	float* cVertexBufferGPU::GetFloatArray(eVertexBufferElement aElement)
	{
		cVtxBufferGPUElementArray *pArray = GetElementArray(aElement);
		if (pArray == nullptr || pArray->mpFloatArray == nullptr) return nullptr;
		if (pArray->mpFloatArray->empty()) return nullptr;
		return &(*pArray->mpFloatArray)[0];
	}

	//-----------------------------------------------------------------------

	int* cVertexBufferGPU::GetIntArray(eVertexBufferElement aElement)
	{
		cVtxBufferGPUElementArray *pArray = GetElementArray(aElement);
		if (pArray == nullptr || pArray->mpIntArray == nullptr) return nullptr;
		if (pArray->mpIntArray->empty()) return nullptr;
		return &(*pArray->mpIntArray)[0];
	}

	//-----------------------------------------------------------------------

	unsigned char* cVertexBufferGPU::GetByteArray(eVertexBufferElement aElement)
	{
		cVtxBufferGPUElementArray *pArray = GetElementArray(aElement);
		if (pArray == nullptr || pArray->mpByteArray == nullptr) return nullptr;
		if (pArray->mpByteArray->empty()) return nullptr;
		return &(*pArray->mpByteArray)[0];
	}

	//-----------------------------------------------------------------------

	unsigned int* cVertexBufferGPU::GetIndices()
	{
		if (mvIndexArray.empty()) return nullptr;
		return &mvIndexArray[0];
	}

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::ResizeArray(eVertexBufferElement aElement, int alSize)
	{
		cVtxBufferGPUElementArray *pArray = GetElementArray(aElement);
		if (pArray == nullptr) return;

		pArray->Resize(alSize * pArray->mlElementNum);
	}

	//-----------------------------------------------------------------------

	void cVertexBufferGPU::ResizeIndices(int alSize)
	{
		mvIndexArray.resize(alSize);
	}

	//-----------------------------------------------------------------------

} // namespace hpl
