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

 * You should have received a copy of the GNU General Public License
 * along with Amnesia: The Dark Descent.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "impl/BitmapLoaderDDS.h"

#include "graphics/Bitmap.h"
#include "system/LowLevelSystem.h"
#include "system/String.h"

#define TINYDDSLOADER_IMPLEMENTATION
#include <tinyddsloader.h>

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cBitmapLoaderDDS::cBitmapLoaderDDS()
	{
		AddSupportedExtension("dds");
	}

	cBitmapLoaderDDS::~cBitmapLoaderDDS()
	{
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cBitmap* cBitmapLoaderDDS::LoadBitmap(const tWString& asFile, tBitmapLoadFlag aFlags)
	{
		// Convert wide string filename to UTF-8
		std::string sFilename = cString::To8Char(asFile);

		// Load DDS file using tinyddsloader
		tinyddsloader::DDSFile dds;
		auto result = dds.Load(sFilename.c_str());

		if (result != tinyddsloader::Result::Success)
		{
			Error("Could not load DDS file '%s'\n", sFilename.c_str());
			return NULL;
		}

		// Get basic info
		uint32_t width = dds.GetWidth();
		uint32_t height = dds.GetHeight();
		uint32_t depth = dds.GetDepth();
		uint32_t mipCount = dds.GetMipCount();
		uint32_t arraySize = dds.GetArraySize();
		bool isCubemap = dds.IsCubemap();
		auto dxgiFormat = dds.GetFormat();

		// Determine pixel format and compression
		ePixelFormat pixelFormat = DXGIFormatToHPLPixelFormat(static_cast<int>(dxgiFormat));
		bool isCompressed = (pixelFormat == ePixelFormat_DXT1 ||
		                     pixelFormat == ePixelFormat_DXT3 ||
		                     pixelFormat == ePixelFormat_DXT5);

		// Force no compression if requested
		if ((aFlags & eBitmapLoadFlag_ForceNoCompression) && isCompressed)
		{
			Error("DDS decompression not supported. File '%s' must be loaded compressed.\n", sFilename.c_str());
			return NULL;
		}

		// Calculate number of images (cubemaps have 6 faces)
		int numImages = isCubemap ? 6 : (arraySize > 1 ? arraySize : 1);
		int numMipmaps = static_cast<int>(mipCount);

		// Create bitmap
		cBitmap* pBitmap = hplNew(cBitmap, ());

		// Set up bitmap with multiple images/mipmaps if needed
		if (numImages > 1 || numMipmaps > 1)
		{
			pBitmap->SetUpData(numImages, numMipmaps);
		}

		// Set size (from mip level 0)
		cVector3l vSize(width, height, depth > 1 ? depth : 1);
		pBitmap->SetSize(vSize);
		pBitmap->SetPixelFormat(pixelFormat);
		pBitmap->SetIsCompressed(isCompressed);

		// Calculate bytes per pixel for compressed formats
		int bytesPerPixel = 4; // Default for RGBA
		if (pixelFormat == ePixelFormat_DXT1)
			bytesPerPixel = 3;
		else if (pixelFormat == ePixelFormat_DXT3 || pixelFormat == ePixelFormat_DXT5)
			bytesPerPixel = 4;
		else if (pixelFormat == ePixelFormat_RGB)
			bytesPerPixel = 3;

		pBitmap->SetBytesPerPixel((char)bytesPerPixel);

		// Load all images and mipmaps
		for (int image = 0; image < numImages; ++image)
		{
			for (int mip = 0; mip < numMipmaps; ++mip)
			{
				// Get image data from tinyddsloader
				auto imageData = dds.GetImageData(mip, image);
				if (!imageData)
				{
					Error("Failed to get image data for mip %d, image %d from '%s'\n",
					      mip, image, sFilename.c_str());
					hplDelete(pBitmap);
					return NULL;
				}

				// Get bitmap data container
				cBitmapData* pImage = pBitmap->GetData(image, mip);

				// Allocate and copy data
				size_t dataSize = imageData->m_memSlicePitch;
				pImage->mlSize = dataSize;
				pImage->mpData = hplNewArray(unsigned char, dataSize);
				memcpy(pImage->mpData, imageData->m_mem, dataSize);
			}
		}

		Log("  Loaded DDS image: %s (%dx%d, %d mipmaps, %d images, %s)\\n",
		    sFilename.c_str(), width, height, numMipmaps, numImages,
		    isCubemap ? "cubemap" : "2D");

		return pBitmap;
	}

	//-----------------------------------------------------------------------

	bool cBitmapLoaderDDS::SaveBitmap(cBitmap* apBitmap, const tWString& asFile, tBitmapSaveFlag aFlags)
	{
		// DDS saving not implemented with tinyddsloader (it's read-only)
		Error("Saving DDS files is not currently supported\n");
		return false;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	ePixelFormat cBitmapLoaderDDS::DXGIFormatToHPLPixelFormat(int dxgiFormat)
	{
		using namespace tinyddsloader;

		switch (dxgiFormat)
		{
		// BC1 = DXT1
		case static_cast<int>(DDSFile::DXGIFormat::BC1_UNorm):
		case static_cast<int>(DDSFile::DXGIFormat::BC1_UNorm_SRGB):
			return ePixelFormat_DXT1;

		// BC2 = DXT2/DXT3
		case static_cast<int>(DDSFile::DXGIFormat::BC2_UNorm):
		case static_cast<int>(DDSFile::DXGIFormat::BC2_UNorm_SRGB):
			return ePixelFormat_DXT3;

		// BC3 = DXT4/DXT5
		case static_cast<int>(DDSFile::DXGIFormat::BC3_UNorm):
		case static_cast<int>(DDSFile::DXGIFormat::BC3_UNorm_SRGB):
			return ePixelFormat_DXT5;

		// Uncompressed RGBA formats
		case static_cast<int>(DDSFile::DXGIFormat::R8G8B8A8_UNorm):
		case static_cast<int>(DDSFile::DXGIFormat::R8G8B8A8_UNorm_SRGB):
			return ePixelFormat_RGBA;

		case static_cast<int>(DDSFile::DXGIFormat::B8G8R8A8_UNorm):
		case static_cast<int>(DDSFile::DXGIFormat::B8G8R8A8_UNorm_SRGB):
			return ePixelFormat_BGRA;

		// Fallback
		default:
			Warning("Unsupported DXGI format %d, defaulting to RGBA\n", dxgiFormat);
			return ePixelFormat_RGBA;
		}
	}

	//-----------------------------------------------------------------------
}
