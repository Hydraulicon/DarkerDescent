/*
 * Copyright © 2009-2020 Frictional Games
 *
 * This file is part of Amnesia: The Dark Descent.
 *
 * Amnesia: The Dark Descent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Amnesia: The Dark Descent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Amnesia: The Dark Descent.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "impl/BitmapLoaderSTB.h"

#include "graphics/Bitmap.h"
#include "system/LowLevelSystem.h"
#include "system/String.h"
#include "system/Platform.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8
#include <stb_image.h>

// STB image write for saving
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cBitmapLoaderSTB::cBitmapLoaderSTB()
	{
		AddSupportedExtension("jpg");
		AddSupportedExtension("jpeg");
		AddSupportedExtension("png");
		AddSupportedExtension("tga");
		AddSupportedExtension("bmp");
	}

	cBitmapLoaderSTB::~cBitmapLoaderSTB()
	{
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cBitmap* cBitmapLoaderSTB::LoadBitmap(const tWString& asFile, tBitmapLoadFlag aFlags)
	{
		// Convert wide string filename to UTF-8
		std::string sFilename = cString::To8Char(asFile);

		// Load image using stb_image
		int width, height, channels;
		unsigned char* data = stbi_load(sFilename.c_str(), &width, &height, &channels, 0);

		if (!data)
		{
			Error("Could not load image '%s': %s\n", sFilename.c_str(), stbi_failure_reason());
			return NULL;
		}

		// Create bitmap
		cBitmap* pBitmap = hplNew(cBitmap, ());

		// Set size
		cVector3l vSize(width, height, 1);
		pBitmap->SetSize(vSize);

		// Determine pixel format based on channels
		ePixelFormat pixelFormat = STBChannelsToHPLPixelFormat(channels);
		pBitmap->SetPixelFormat(pixelFormat);
		pBitmap->SetBytesPerPixel((char)channels);
		pBitmap->SetIsCompressed(false);

		// Copy data to bitmap
		cBitmapData* pImage = pBitmap->GetData(0, 0);
		size_t dataSize = width * height * channels;
		pImage->SetData(data, dataSize);

		// Free stb_image data
		stbi_image_free(data);

		Log("  Loaded STB image: %s (%dx%d, %d channels)\n",
			sFilename.c_str(), width, height, channels);

		return pBitmap;
	}

	//-----------------------------------------------------------------------

	bool cBitmapLoaderSTB::SaveBitmap(cBitmap* apBitmap, const tWString& asFile, tBitmapSaveFlag aFlags)
	{
		if (!apBitmap)
		{
			Error("Cannot save NULL bitmap!\n");
			return false;
		}

		// Convert filename
		std::string sFilename = cString::To8Char(asFile);

		// Get file extension
		tWString sExt = cString::ToLowerCaseW(cString::GetFileExtW(asFile));

		// Get bitmap data
		cBitmapData* pData = apBitmap->GetData(0, 0);
		if (!pData || !pData->mpData)
		{
			Error("Bitmap has no data to save!\n");
			return false;
		}

		int width = apBitmap->GetWidth();
		int height = apBitmap->GetHeight();
		int channels = apBitmap->GetBytesPerPixel();

		// Save using appropriate format
		int result = 0;
		if (sExt == _W("png"))
		{
			result = stbi_write_png(sFilename.c_str(), width, height, channels, pData->mpData, width * channels);
		}
		else if (sExt == _W("jpg") || sExt == _W("jpeg"))
		{
			result = stbi_write_jpg(sFilename.c_str(), width, height, channels, pData->mpData, 90);
		}
		else if (sExt == _W("tga"))
		{
			result = stbi_write_tga(sFilename.c_str(), width, height, channels, pData->mpData);
		}
		else if (sExt == _W("bmp"))
		{
			result = stbi_write_bmp(sFilename.c_str(), width, height, channels, pData->mpData);
		}
		else
		{
			Error("Unsupported image format for saving: %s\n", cString::To8Char(asFile).c_str());
			return false;
		}

		if (result == 0)
		{
			Error("Failed to save image: %s\n", sFilename.c_str());
			return false;
		}

		return true;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	ePixelFormat cBitmapLoaderSTB::STBChannelsToHPLPixelFormat(int channels)
	{
		switch (channels)
		{
		case 1: return ePixelFormat_Luminance;
		case 2: return ePixelFormat_LuminanceAlpha;
		case 3: return ePixelFormat_RGB;
		case 4: return ePixelFormat_RGBA;
		default:
			Warning("Unknown channel count %d, defaulting to RGBA\n", channels);
			return ePixelFormat_RGBA;
		}
	}

	//-----------------------------------------------------------------------
}
