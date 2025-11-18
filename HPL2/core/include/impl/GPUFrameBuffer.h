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

#ifndef HPL_GPU_FRAME_BUFFER_H
#define HPL_GPU_FRAME_BUFFER_H

#include "graphics/FrameBuffer.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

namespace hpl {

	class cDepthStencilBufferGPU : public iDepthStencilBuffer
	{
	public:
		cDepthStencilBufferGPU(const cVector2l& avSize, int alDepthBits, int alStencilBits);
		~cDepthStencilBufferGPU();

		// SDL_GPU Specific
		SDL_GPUTexture* GetTextureHandle() { return mpTexture; }

	private:
		SDL_GPUTexture* mpTexture;
	};

	//-----------------------------------------------

	class cFrameBufferGPU : public iFrameBuffer
	{
	public:
		cFrameBufferGPU(const tString& asName, iLowLevelGraphics* apLowLevelGraphics);
		~cFrameBufferGPU();

		void SetTexture2D(int alColorIdx, iTexture *apTexture, int alMipmapLevel=0);
		void SetTexture3D(int alColorIdx, iTexture *apTexture, int alZ, int alMipmapLevel=0);
		void SetTextureCubeMap(int alColorIdx, iTexture *apTexture, int alFace, int alMipmapLevel=0);

		void SetDepthTexture2D(iTexture *apTexture, int alMipmapLevel=0);
		void SetDepthTextureCubeMap(iTexture *apTexture, int alFace, int alMipmapLevel=0);

		void SetDepthStencilBuffer(iDepthStencilBuffer* apBuffer);

		bool CompileAndValidate();

		void PostBindUpdate();

		// SDL_GPU Specific
		bool IsUpdated() { return mbIsUpdated; }
		void SetIsUpdated(bool abX) { mbIsUpdated = abX; }

	private:
		bool mbIsUpdated;
	};

}; // namespace hpl

#endif // HPL_GPU_FRAME_BUFFER_H
