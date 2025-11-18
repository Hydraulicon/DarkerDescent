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

#include "impl/GPUFrameBuffer.h"
#include "impl/LowLevelGraphicsGPU.h"
#include "graphics/Texture.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// DEPTH STENCIL BUFFER GPU
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cDepthStencilBufferGPU::cDepthStencilBufferGPU(const cVector2l& avSize, int alDepthBits, int alStencilBits)
		: iDepthStencilBuffer(avSize, alDepthBits, alStencilBits)
	{
		mpTexture = nullptr;
		// TODO: Create SDL_GPU depth/stencil texture
		// Will need SDL_CreateGPUTexture() with appropriate format based on depth/stencil bits
	}

	//-----------------------------------------------------------------------

	cDepthStencilBufferGPU::~cDepthStencilBufferGPU()
	{
		if (mpTexture)
		{
			// TODO: SDL_ReleaseGPUTexture(device, mpTexture);
			mpTexture = nullptr;
		}
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// FRAME BUFFER GPU
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cFrameBufferGPU::cFrameBufferGPU(const tString& asName, iLowLevelGraphics* apLowLevelGraphics)
		: iFrameBuffer(asName, apLowLevelGraphics)
	{
		mbIsUpdated = false;
		// Note: SDL_GPU doesn't use traditional framebuffer objects like OpenGL
		// Instead, render passes target textures directly
	}

	//-----------------------------------------------------------------------

	cFrameBufferGPU::~cFrameBufferGPU()
	{
		// SDL_GPU framebuffers don't need explicit cleanup
		// Textures are managed separately
	}

	//-----------------------------------------------------------------------

	void cFrameBufferGPU::SetTexture2D(int alColorIdx, iTexture *apTexture, int alMipmapLevel)
	{
		// TODO: Store texture attachment info for later render pass creation
		mpColorBuffer[alColorIdx] = apTexture;

		if (apTexture && mvSize.x < 0)
		{
			mvSize.x = apTexture->GetWidth();
			mvSize.y = apTexture->GetHeight();
		}

		mbIsUpdated = false;
	}

	//-----------------------------------------------------------------------

	void cFrameBufferGPU::SetTexture3D(int alColorIdx, iTexture *apTexture, int alZ, int alMipmapLevel)
	{
		// TODO: Store 3D texture attachment info
		mpColorBuffer[alColorIdx] = apTexture;

		if (apTexture && mvSize.x < 0)
		{
			mvSize.x = apTexture->GetWidth();
			mvSize.y = apTexture->GetHeight();
		}

		mbIsUpdated = false;
	}

	//-----------------------------------------------------------------------

	void cFrameBufferGPU::SetTextureCubeMap(int alColorIdx, iTexture *apTexture, int alFace, int alMipmapLevel)
	{
		// TODO: Store cubemap attachment info
		mpColorBuffer[alColorIdx] = apTexture;

		if (apTexture && mvSize.x < 0)
		{
			mvSize.x = apTexture->GetWidth();
			mvSize.y = apTexture->GetHeight();
		}

		mbIsUpdated = false;
	}

	//-----------------------------------------------------------------------

	void cFrameBufferGPU::SetDepthTexture2D(iTexture *apTexture, int alMipmapLevel)
	{
		// TODO: Store depth texture attachment info
		mpDepthBuffer = apTexture;

		if (apTexture && mvSize.x < 0)
		{
			mvSize.x = apTexture->GetWidth();
			mvSize.y = apTexture->GetHeight();
		}

		mbIsUpdated = false;
	}

	//-----------------------------------------------------------------------

	void cFrameBufferGPU::SetDepthTextureCubeMap(iTexture *apTexture, int alFace, int alMipmapLevel)
	{
		// TODO: Store depth cubemap attachment info
		mpDepthBuffer = apTexture;

		if (apTexture && mvSize.x < 0)
		{
			mvSize.x = apTexture->GetWidth();
			mvSize.y = apTexture->GetHeight();
		}

		mbIsUpdated = false;
	}

	//-----------------------------------------------------------------------

	void cFrameBufferGPU::SetDepthStencilBuffer(iDepthStencilBuffer* apBuffer)
	{
		// TODO: Store depth/stencil buffer info
		mpDepthBuffer = apBuffer;
		mpStencilBuffer = apBuffer;

		if (apBuffer && mvSize.x < 0)
		{
			mvSize = apBuffer->GetSize();
		}

		mbIsUpdated = false;
	}

	//-----------------------------------------------------------------------

	bool cFrameBufferGPU::CompileAndValidate()
	{
		// TODO: Validate attachments are compatible
		// For now, just return success since SDL_GPU handles this at render pass creation
		mbIsUpdated = false;
		return true;
	}

	//-----------------------------------------------------------------------

	void cFrameBufferGPU::PostBindUpdate()
	{
		// TODO: Update render pass configuration if attachments changed
		mbIsUpdated = true;
	}

	//-----------------------------------------------------------------------

} // namespace hpl
