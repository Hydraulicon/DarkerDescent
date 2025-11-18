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

#ifndef HPL_GPU_PROGRAM_GPU_H
#define HPL_GPU_PROGRAM_GPU_H

#include "system/SystemTypes.h"
#include "math/MathTypes.h"
#include "graphics/GPUShader.h"
#include "graphics/GPUProgram.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <map>

namespace hpl {

	//-----------------------------------------------------

	class cGPUProgram_GPU_Param
	{
	public:
		cGPUProgram_GPU_Param() : mlLocation(-1), msName(""){}
		cGPUProgram_GPU_Param(int alLocation, const tString& asName) : mlLocation(alLocation), msName(asName){}

		tString msName;
		int mlLocation;
	};

	//-----------------------------------------------------

	class cGPUProgram_GPU : public iGpuProgram
	{
	public:
		cGPUProgram_GPU(const tString& asName);
		~cGPUProgram_GPU();

		bool Link();

		void Bind();
		void UnBind();

		bool CanAccessAPIMatrix(){ return false;} // Modern approach: use uniforms
		bool SamplerNeedsTextureUnitSetup(){ return false; }

		/**
		 * Does not need to bind/unbind in SDL_GPU
		 */
		bool SetSamplerToUnit(const tString& asSamplerName, int alUnit);

		int GetVariableId(const tString& asName);
		bool GetVariableAsId(const tString& asName, int alId);

		bool SetInt(int alVarId, int alX);
		bool SetFloat(int alVarId, float afX);
		bool SetVec2f(int alVarId, float afX,float afY);
		bool SetVec3f(int alVarId, float afX,float afY,float afZ);
		bool SetVec4f(int alVarId, float afX,float afY,float afZ, float afW);

		bool SetMatrixf(int alVarId, const cMatrixf& aMtx);
		bool SetMatrixf(int alVarId, eGpuShaderMatrix aType, eGpuShaderMatrixOp aOp);

		// SDL_GPU Specific
		SDL_GPUGraphicsPipeline* GetPipeline() { return mpPipeline; }

	private:
		void LogProgramInfoLog(const tString& asError);

		SDL_GPUGraphicsPipeline* mpPipeline;
		static cGPUProgram_GPU* mpCurrentProgram;

		std::map<tString, cGPUProgram_GPU_Param> m_mapParameters;
	};
};
#endif // HPL_GPU_PROGRAM_GPU_H
