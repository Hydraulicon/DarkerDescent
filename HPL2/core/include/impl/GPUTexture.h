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

#ifndef HPL_GPU_TEXTURE_H
#define HPL_GPU_TEXTURE_H

#include "graphics/Texture.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <vector>

namespace hpl {

	class cBitmapData;
	class iLowLevelGraphics;

	class cGPUTexture : public iTexture
	{
	public:
		cGPUTexture(const tString& asName, eTextureType aType, eTextureUsage aUsage, iLowLevelGraphics* apLowLevelGraphics);
		~cGPUTexture();

		bool CreateFromBitmap(cBitmap* pBmp);
		bool CreateAnimFromBitmapVec(std::vector<cBitmap*> *avBitmaps);
		bool CreateCubeFromBitmapVec(std::vector<cBitmap*> *avBitmaps);
		bool CreateFromRawData(const cVector3l &avSize, ePixelFormat aPixelFormat, unsigned char *apData);

		virtual void SetRawData(int alLevel, const cVector3l& avOffset, const cVector3l& avSize,
								ePixelFormat aPixelFormat, void *apData);

		void SetFilter(eTextureFilter aFilter);
		void SetAnisotropyDegree(float afX);

		void SetWrapS(eTextureWrap aMode);
		void SetWrapT(eTextureWrap aMode);
		void SetWrapR(eTextureWrap aMode);
		void SetWrapSTR(eTextureWrap aMode);

		void SetCompareMode(eTextureCompareMode aMode);
		void SetCompareFunc(eTextureCompareFunc aFunc);

		void AutoGenerateMipmaps();

		void Update(float afTimeStep);

		bool HasAnimation();
		void NextFrame();
		void PrevFrame();
		float GetT();
		float GetTimeCount();
		void SetTimeCount(float afX);
		int GetCurrentLowlevelHandle();

		/// SDL_GPU Specific ///////////
		SDL_GPUTexture* GetTextureHandle();

	private:
		SDL_GPUTexture* mpTexture;
		SDL_GPUSampler* mpSampler;

		std::vector<SDL_GPUTexture*> mvTextureHandles; // For animations
		int mlCurrentFrame;
		float mfTimeCount;
		float mfTimeDir;
	};

};
#endif // HPL_GPU_TEXTURE_H
