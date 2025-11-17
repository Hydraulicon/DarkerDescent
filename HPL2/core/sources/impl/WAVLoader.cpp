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

#include "impl/WAVLoader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------

// Thread-local error code
static int g_lastError = WAV_NO_ERROR;

//---------------------------------------------

// WAV file format structures
#pragma pack(push, 1)

struct WAVHeader
{
	char riff[4];        // "RIFF"
	unsigned int size;   // File size - 8
	char wave[4];        // "WAVE"
};

struct WAVChunkHeader
{
	char id[4];          // Chunk ID
	unsigned int size;   // Chunk size
};

struct WAVFormatChunk
{
	unsigned short audioFormat;      // Audio format (1 = PCM)
	unsigned short numChannels;      // Number of channels
	unsigned int sampleRate;         // Sample rate
	unsigned int byteRate;           // Byte rate
	unsigned short blockAlign;       // Block align
	unsigned short bitsPerSample;    // Bits per sample
};

#pragma pack(pop)

//---------------------------------------------

int WAV_LoadFile(const char* filename, ALenum* format, void** data, ALsizei* size, ALsizei* frequency)
{
	if (!filename || !format || !data || !size || !frequency)
	{
		g_lastError = WAV_ERROR_INVALID_FORMAT;
		return g_lastError;
	}

	// Initialize outputs
	*data = NULL;
	*size = 0;
	*format = 0;
	*frequency = 0;

	// Open file
	FILE* file = fopen(filename, "rb");
	if (!file)
	{
		g_lastError = WAV_ERROR_FILE_NOT_FOUND;
		return g_lastError;
	}

	// Read and validate WAV header
	WAVHeader header;
	if (fread(&header, sizeof(WAVHeader), 1, file) != 1)
	{
		fclose(file);
		g_lastError = WAV_ERROR_INVALID_FORMAT;
		return g_lastError;
	}

	if (memcmp(header.riff, "RIFF", 4) != 0 || memcmp(header.wave, "WAVE", 4) != 0)
	{
		fclose(file);
		g_lastError = WAV_ERROR_INVALID_FORMAT;
		return g_lastError;
	}

	// Find fmt and data chunks
	WAVFormatChunk fmtChunk;
	bool foundFormat = false;
	bool foundData = false;
	unsigned int dataSize = 0;
	long dataOffset = 0;

	while (!feof(file) && (!foundFormat || !foundData))
	{
		WAVChunkHeader chunkHeader;
		if (fread(&chunkHeader, sizeof(WAVChunkHeader), 1, file) != 1)
			break;

		if (memcmp(chunkHeader.id, "fmt ", 4) == 0)
		{
			// Read format chunk
			if (chunkHeader.size < sizeof(WAVFormatChunk))
			{
				fclose(file);
				g_lastError = WAV_ERROR_INVALID_FORMAT;
				return g_lastError;
			}

			if (fread(&fmtChunk, sizeof(WAVFormatChunk), 1, file) != 1)
			{
				fclose(file);
				g_lastError = WAV_ERROR_INVALID_FORMAT;
				return g_lastError;
			}

			// Skip any extra format bytes
			if (chunkHeader.size > sizeof(WAVFormatChunk))
			{
				fseek(file, chunkHeader.size - sizeof(WAVFormatChunk), SEEK_CUR);
			}

			foundFormat = true;
		}
		else if (memcmp(chunkHeader.id, "data", 4) == 0)
		{
			// Found data chunk
			dataSize = chunkHeader.size;
			dataOffset = ftell(file);
			foundData = true;

			// Skip data for now, we'll read it after validating format
			fseek(file, chunkHeader.size, SEEK_CUR);
		}
		else
		{
			// Skip unknown chunk
			fseek(file, chunkHeader.size, SEEK_CUR);
		}
	}

	if (!foundFormat || !foundData)
	{
		fclose(file);
		g_lastError = WAV_ERROR_INVALID_FORMAT;
		return g_lastError;
	}

	// Validate format
	if (fmtChunk.audioFormat != 1) // 1 = PCM
	{
		fclose(file);
		g_lastError = WAV_ERROR_UNSUPPORTED_FORMAT;
		return g_lastError;
	}

	// Determine OpenAL format
	if (fmtChunk.numChannels == 1)
	{
		if (fmtChunk.bitsPerSample == 8)
			*format = AL_FORMAT_MONO8;
		else if (fmtChunk.bitsPerSample == 16)
			*format = AL_FORMAT_MONO16;
		else
		{
			fclose(file);
			g_lastError = WAV_ERROR_UNSUPPORTED_FORMAT;
			return g_lastError;
		}
	}
	else if (fmtChunk.numChannels == 2)
	{
		if (fmtChunk.bitsPerSample == 8)
			*format = AL_FORMAT_STEREO8;
		else if (fmtChunk.bitsPerSample == 16)
			*format = AL_FORMAT_STEREO16;
		else
		{
			fclose(file);
			g_lastError = WAV_ERROR_UNSUPPORTED_FORMAT;
			return g_lastError;
		}
	}
	else
	{
		fclose(file);
		g_lastError = WAV_ERROR_UNSUPPORTED_FORMAT;
		return g_lastError;
	}

	// Allocate buffer for audio data
	*data = malloc(dataSize);
	if (!*data)
	{
		fclose(file);
		g_lastError = WAV_ERROR_OUT_OF_MEMORY;
		return g_lastError;
	}

	// Read audio data
	fseek(file, dataOffset, SEEK_SET);
	if (fread(*data, 1, dataSize, file) != dataSize)
	{
		free(*data);
		*data = NULL;
		fclose(file);
		g_lastError = WAV_ERROR_INVALID_FORMAT;
		return g_lastError;
	}

	// Set output parameters
	*size = (ALsizei)dataSize;
	*frequency = (ALsizei)fmtChunk.sampleRate;

	// Log successful load for verification
	printf("[WAV Loader] Successfully loaded: %s (%d Hz, %d ch, %d bits, %d bytes)\n",
		filename, fmtChunk.sampleRate, fmtChunk.numChannels, fmtChunk.bitsPerSample, dataSize);

	fclose(file);
	g_lastError = WAV_NO_ERROR;
	return WAV_NO_ERROR;
}

//---------------------------------------------

void WAV_UnloadData(void* data)
{
	if (data)
	{
		free(data);
	}
}

//---------------------------------------------

int WAV_GetError()
{
	return g_lastError;
}

//---------------------------------------------
