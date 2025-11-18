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

#ifndef HPL_VERTEXBUFFER_GPU_H
#define HPL_VERTEXBUFFER_GPU_H

#include "graphics/VertexBuffer.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <vector>

namespace hpl {

	//---------------------------------------------------------

	class cVtxBufferGPUElementArray
	{
	public:
		cVtxBufferGPUElementArray(eVertexBufferElementFormat aFormat);
		~cVtxBufferGPUElementArray();

		void Reserve(size_t alSize);
		void Resize(size_t alSize);
		void PushBack(const void *apData);
		size_t Size();

		void* GetArrayPtr();

		eVertexBufferElement mType;
		tVertexElementFlag mFlag;

		eVertexBufferElementFormat mFormat;

		int mlElementNum;
		int mlProgramVarIndex;

		tByteVec* mpByteArray;
		tIntVec* mpIntArray;
		tFloatVec* mpFloatArray;
	};

	//---------------------------------------------------------

	class cVertexBufferGPU : public iVertexBuffer
	{
	public:
		cVertexBufferGPU(	iLowLevelGraphics* apLowLevelGraphics,
							eVertexBufferType aType,
							eVertexBufferDrawType aDrawType, eVertexBufferUsageType aUsageType,
							size_t alReserveVtxSize, size_t alReserveIdxSize);
		~cVertexBufferGPU();

		void CreateElementArray(	eVertexBufferElement aType, eVertexBufferElementFormat aFormat,
									int alElementNum, int alProgramVarIndex=0);

		void AddVertexVec3f(eVertexBufferElement aElement, const cVector3f& avVtx);
		void AddVertexVec4f(eVertexBufferElement aElement, const cVector3f& avVtx, float afW);
		void AddVertexColor(eVertexBufferElement aElement, const cColor& aColor);
		void AddIndex(unsigned int alIndex);

		bool Compile(tVertexCompileFlag aFlags);
		void UpdateData(tVertexElementFlag aTypes, bool abIndices);

		void CreateShadowDouble(bool abUpdateData);
		void Transform(const cMatrixf &mtxTransform);

		void Draw(eVertexBufferDrawType aDrawType = eVertexBufferDrawType_LastEnum);
		void DrawIndices(	unsigned int *apIndices, int alCount,
							eVertexBufferDrawType aDrawType = eVertexBufferDrawType_LastEnum);

		void Bind();
		void UnBind();

		iVertexBuffer* CreateCopy(	eVertexBufferType aType, eVertexBufferUsageType aUsageType,
									tVertexElementFlag alVtxToCopy);

		cBoundingVolume CreateBoundingVolume();

		size_t GetVertexNum();
		size_t GetIndexNum();

		int GetElementNum(eVertexBufferElement aElement);
		eVertexBufferElementFormat GetElementFormat(eVertexBufferElement aElement);
		int GetElementProgramVarIndex(eVertexBufferElement aElement);

		float* GetFloatArray(eVertexBufferElement aElement);
		int* GetIntArray(eVertexBufferElement aElement);
		unsigned char* GetByteArray(eVertexBufferElement aElement);

		unsigned int* GetIndices();

		void ResizeArray(eVertexBufferElement aElement, int alSize);
		void ResizeIndices(int alSize);

	protected:
		inline cVtxBufferGPUElementArray* GetElementArray(eVertexBufferElement aElement)
		{
			int lIdx = mvElementArrayIndex[aElement];
			if(lIdx <0) return NULL;
			return mvElementArrays[ lIdx ];
		}

		char mvElementArrayIndex[eVertexBufferElement_LastEnum];
		std::vector<cVtxBufferGPUElementArray*> mvElementArrays;

		tUIntVec mvIndexArray;

		bool mbHasShadowDouble;
		bool mbCompiled;

		// SDL_GPU specific - buffers will be created on demand
		SDL_GPUBuffer* mpVertexBuffer;
		SDL_GPUBuffer* mpIndexBuffer;
	};

};
#endif // HPL_VERTEXBUFFER_GPU_H
