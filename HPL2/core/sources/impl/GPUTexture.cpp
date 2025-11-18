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

#include "impl/GPUTexture.h"
#include "impl/LowLevelGraphicsGPU.h"
#include "graphics/Bitmap.h"
#include "system/LowLevelSystem.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cGPUTexture::cGPUTexture(const tString& asName, eTextureType aType, eTextureUsage aUsage, iLowLevelGraphics* apLowLevelGraphics)
		: iTexture(asName, _W(""), aType, aUsage, apLowLevelGraphics)
	{
		mpTexture = nullptr;
		mpSampler = nullptr;
		mlCurrentFrame = 0;
		mfTimeCount = 0;
		mfTimeDir = 1;
	}

	//-----------------------------------------------------------------------

	cGPUTexture::~cGPUTexture()
	{
		// TODO: Release SDL_GPU texture and sampler resources
		if (mpTexture)
		{
			// SDL_ReleaseGPUTexture(...);
			mpTexture = nullptr;
		}

		if (mpSampler)
		{
			// SDL_ReleaseGPUSampler(...);
			mpSampler = nullptr;
		}

		for (size_t i = 0; i < mvTextureHandles.size(); i++)
		{
			if (mvTextureHandles[i])
			{
				// SDL_ReleaseGPUTexture(...);
			}
		}
		mvTextureHandles.clear();
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	bool cGPUTexture::CreateFromBitmap(cBitmap* pBmp)
	{
		// TODO: Implement texture creation from bitmap
		if (!pBmp) return false;

		// For now, just set the size and return success so initialization can continue
		cVector3l vSize = pBmp->GetSize();
		mvSize = vSize;
		mPixelFormat = pBmp->GetPixelFormat();

		return true;
	}

	//-----------------------------------------------------------------------

	bool cGPUTexture::CreateAnimFromBitmapVec(std::vector<cBitmap*> *avBitmaps)
	{
		// TODO: Implement animated texture creation
		if (!avBitmaps || avBitmaps->empty()) return false;

		// Just use first frame for now
		return CreateFromBitmap((*avBitmaps)[0]);
	}

	//-----------------------------------------------------------------------

	bool cGPUTexture::CreateCubeFromBitmapVec(std::vector<cBitmap*> *avBitmaps)
	{
		// TODO: Implement cubemap creation
		if (!avBitmaps || avBitmaps->size() < 6) return false;

		// Just use first face for now
		return CreateFromBitmap((*avBitmaps)[0]);
	}

	//-----------------------------------------------------------------------

	bool cGPUTexture::CreateFromRawData(const cVector3l &avSize, ePixelFormat aPixelFormat, unsigned char *apData)
	{
		// TODO: Implement texture creation from raw data
		mvSize = avSize;
		mPixelFormat = aPixelFormat;

		return true;
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::SetRawData(int alLevel, const cVector3l& avOffset, const cVector3l& avSize,
								  ePixelFormat aPixelFormat, void *apData)
	{
		// TODO: Implement texture data update
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::SetFilter(eTextureFilter aFilter)
	{
		mFilter = aFilter;
		// TODO: Update sampler filter mode
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::SetAnisotropyDegree(float afX)
	{
		mfAnisotropyDegree = afX;
		// TODO: Update sampler anisotropy
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::SetWrapS(eTextureWrap aMode)
	{
		mWrapS = aMode;
		// TODO: Update sampler wrap mode
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::SetWrapT(eTextureWrap aMode)
	{
		mWrapT = aMode;
		// TODO: Update sampler wrap mode
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::SetWrapR(eTextureWrap aMode)
	{
		mWrapR = aMode;
		// TODO: Update sampler wrap mode
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::SetWrapSTR(eTextureWrap aMode)
	{
		SetWrapS(aMode);
		SetWrapT(aMode);
		SetWrapR(aMode);
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::SetCompareMode(eTextureCompareMode aMode)
	{
		mCompareMode = aMode;
		// TODO: Update sampler compare mode
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::SetCompareFunc(eTextureCompareFunc aFunc)
	{
		mCompareFunc = aFunc;
		// TODO: Update sampler compare function
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::AutoGenerateMipmaps()
	{
		// TODO: Generate mipmaps for texture
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::Update(float afTimeStep)
	{
		if (!HasAnimation()) return;

		mfTimeCount += afTimeStep * (1.0f / mfFrameTime) * mfTimeDir;

		if (mfTimeDir > 0)
		{
			if (mfTimeCount >= (float)mvTextureHandles.size())
			{
				if (mAnimMode == eTextureAnimMode_Loop)
				{
					mfTimeCount = 0;
				}
				else
				{
					mfTimeCount = (float)(mvTextureHandles.size() - 1);
					mfTimeDir = -1;
				}
			}
		}
		else
		{
			if (mfTimeCount < 0)
			{
				mfTimeCount = 1;
				mfTimeDir = 1;
			}
		}
	}

	//-----------------------------------------------------------------------

	bool cGPUTexture::HasAnimation()
	{
		return mvTextureHandles.size() > 1;
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::NextFrame()
	{
		if (!HasAnimation()) return;

		mfTimeCount += 1.0f;
		if (mfTimeCount >= (float)mvTextureHandles.size())
			mfTimeCount = 0;
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::PrevFrame()
	{
		if (!HasAnimation()) return;

		mfTimeCount -= 1.0f;
		if (mfTimeCount < 0)
			mfTimeCount = (float)(mvTextureHandles.size() - 1);
	}

	//-----------------------------------------------------------------------

	float cGPUTexture::GetT()
	{
		if (!HasAnimation()) return 0;

		return mfTimeCount / (float)mvTextureHandles.size();
	}

	//-----------------------------------------------------------------------

	float cGPUTexture::GetTimeCount()
	{
		return mfTimeCount;
	}

	//-----------------------------------------------------------------------

	void cGPUTexture::SetTimeCount(float afX)
	{
		mfTimeCount = afX;
	}

	//-----------------------------------------------------------------------

	int cGPUTexture::GetCurrentLowlevelHandle()
	{
		if (HasAnimation())
		{
			int lFrame = (int)mfTimeCount;
			if (lFrame < 0) lFrame = 0;
			if (lFrame >= (int)mvTextureHandles.size()) lFrame = (int)mvTextureHandles.size() - 1;
			return lFrame;
		}
		return 0;
	}

	//-----------------------------------------------------------------------

	SDL_GPUTexture* cGPUTexture::GetTextureHandle()
	{
		if (HasAnimation())
		{
			int lFrame = GetCurrentLowlevelHandle();
			return mvTextureHandles[lFrame];
		}
		return mpTexture;
	}

	//-----------------------------------------------------------------------

} // namespace hpl
