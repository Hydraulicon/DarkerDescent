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

#ifndef HPL_GPU_SHADER_GPU_H
#define HPL_GPU_SHADER_GPU_H

#include "system/SystemTypes.h"
#include "math/MathTypes.h"
#include "graphics/GPUShader.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <vector>

namespace hpl {

	//----------------------------------------------

	class iLowLevelGraphics;

	//----------------------------------------------

	class cGPUShader_GPU : public iGpuShader
	{
	public:
		cGPUShader_GPU(const tString& asName, eGpuShaderType aType, iLowLevelGraphics *apLowLevelGraphics);
		~cGPUShader_GPU();

		bool Reload();
		void Unload();
		void Destroy();

		bool SamplerNeedsTextureUnitSetup(){ return false;} // SDL_GPU handles this

		tString GetProgramName(){ return msName; }

		bool CreateFromFile(const tWString& asFile, const tString& asEntry="main", bool abPrintInfoIfFail=true);
		bool CreateFromString(const char *apStringData, const tString& asEntry="main", bool abPrintInfoIfFail=true);

		// SDL_GPU Specific
		SDL_GPUShader* GetHandle(){ return mpShader;}
		const std::vector<unsigned char>& GetSPIRV() const { return mvSPIRVBytecode; }

	protected:
		void LogShaderInfoLog(const tString& asError);
		void LogShaderCode(const char *apStringData);

		bool CompileGLSLToSPIRV(const char* apGLSLSource, const tString& asEntry);
		tString TranslateGLSL120To450(const char* apGLSL120Source);

		iLowLevelGraphics *mpLowLevelGraphics;

		SDL_GPUShader* mpShader;
		std::vector<unsigned char> mvSPIRVBytecode;
	};
};
#endif // HPL_GPU_SHADER_GPU_H
