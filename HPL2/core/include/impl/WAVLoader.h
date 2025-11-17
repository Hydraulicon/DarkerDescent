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

#ifndef HPL_WAV_LOADER_H
#define HPL_WAV_LOADER_H

#include <AL/al.h>

//---------------------------------------------

// WAV file loader error codes
#define WAV_NO_ERROR           0
#define WAV_ERROR_FILE_NOT_FOUND   1
#define WAV_ERROR_INVALID_FORMAT   2
#define WAV_ERROR_UNSUPPORTED_FORMAT 3
#define WAV_ERROR_OUT_OF_MEMORY    4

//---------------------------------------------

/**
 * Load a WAV file into memory
 * @param filename Path to the WAV file
 * @param format Output: OpenAL format (AL_FORMAT_MONO8, AL_FORMAT_MONO16, etc.)
 * @param data Output: PCM audio data buffer (caller must free with WAV_UnloadData)
 * @param size Output: Size of the data buffer in bytes
 * @param frequency Output: Sample rate in Hz
 * @return WAV_NO_ERROR on success, error code otherwise
 */
int WAV_LoadFile(const char* filename, ALenum* format, void** data, ALsizei* size, ALsizei* frequency);

/**
 * Free memory allocated by WAV_LoadFile
 * @param data Pointer to data buffer returned by WAV_LoadFile
 */
void WAV_UnloadData(void* data);

/**
 * Get the last error code from WAV loading operations
 * @return Error code from last operation
 */
int WAV_GetError();

//---------------------------------------------

#endif // HPL_WAV_LOADER_H
