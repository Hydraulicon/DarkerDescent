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

#include "impl/GPUProgram_GPU.h"
#include "impl/GPUShader_GPU.h"
#include "impl/LowLevelGraphicsGPU.h"

#include "system/LowLevelSystem.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// STATIC
	//////////////////////////////////////////////////////////////////////////

	cGPUProgram_GPU* cGPUProgram_GPU::mpCurrentProgram = nullptr;

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cGPUProgram_GPU::cGPUProgram_GPU(const tString& asName)
		: iGpuProgram(asName, eGpuProgramFormat_SPIRV)
	{
		mpPipeline = nullptr;
	}

	//-----------------------------------------------------------------------

	cGPUProgram_GPU::~cGPUProgram_GPU()
	{
		if (mpPipeline)
		{
			// TODO: Need to get GPU device - may need to store it or pass in LowLevelGraphics
			// SDL_ReleaseGPUGraphicsPipeline(pDevice, mpPipeline);
			mpPipeline = nullptr;
		}
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	bool cGPUProgram_GPU::Link()
	{
		Log("-------- Linking GPU Program '%s' --------\n", msName.c_str());

		// Get the vertex and fragment shaders
		cGPUShader_GPU* pVertexShader = static_cast<cGPUShader_GPU*>(mpShader[eGpuShaderType_Vertex]);
		cGPUShader_GPU* pFragmentShader = static_cast<cGPUShader_GPU*>(mpShader[eGpuShaderType_Fragment]);

		if (!pVertexShader)
		{
			Error("GPU Program '%s': No vertex shader attached\n", msName.c_str());
			return false;
		}

		if (!pFragmentShader)
		{
			Error("GPU Program '%s': No fragment shader attached\n", msName.c_str());
			return false;
		}

		if (!pVertexShader->GetHandle())
		{
			Error("GPU Program '%s': Vertex shader not compiled\n", msName.c_str());
			return false;
		}

		if (!pFragmentShader->GetHandle())
		{
			Error("GPU Program '%s': Fragment shader not compiled\n", msName.c_str());
			return false;
		}

		// TODO: Create graphics pipeline
		// For now, we'll just store the shader pointers and defer pipeline creation
		// until we have vertex format information

		Log("  GPU Program '%s' ready (pipeline creation deferred)\n", msName.c_str());
		return true;
	}

	//-----------------------------------------------------------------------

	void cGPUProgram_GPU::Bind()
	{
		mpCurrentProgram = this;

		// TODO: Bind the pipeline when we have render pass context
		// In SDL_GPU, binding happens via SDL_BindGPUGraphicsPipeline during a render pass
	}

	//-----------------------------------------------------------------------

	void cGPUProgram_GPU::UnBind()
	{
		mpCurrentProgram = nullptr;

		// TODO: Unbind pipeline if needed
	}

	//-----------------------------------------------------------------------

	bool cGPUProgram_GPU::SetSamplerToUnit(const tString& asSamplerName, int alUnit)
	{
		// SDL_GPU handles sampler binding automatically through the pipeline
		// Just store the mapping for later use
		return true;
	}

	//-----------------------------------------------------------------------

	int cGPUProgram_GPU::GetVariableId(const tString& asName)
	{
		// Check if we already have this parameter
		auto it = m_mapParameters.find(asName);
		if (it != m_mapParameters.end())
		{
			return it->second.mlLocation;
		}

		// Assign a new location
		int lLocation = (int)m_mapParameters.size();
		m_mapParameters[asName] = cGPUProgram_GPU_Param(lLocation, asName);

		return lLocation;
	}

	//-----------------------------------------------------------------------

	bool cGPUProgram_GPU::GetVariableAsId(const tString& asName, int alId)
	{
		auto it = m_mapParameters.find(asName);
		return (it != m_mapParameters.end() && it->second.mlLocation == alId);
	}

	//-----------------------------------------------------------------------

	bool cGPUProgram_GPU::SetInt(int alVarId, int alX)
	{
		// TODO: Push constants or uniform buffers
		return false;
	}

	//-----------------------------------------------------------------------

	bool cGPUProgram_GPU::SetFloat(int alVarId, float afX)
	{
		// TODO: Push constants or uniform buffers
		return false;
	}

	//-----------------------------------------------------------------------

	bool cGPUProgram_GPU::SetVec2f(int alVarId, float afX, float afY)
	{
		// TODO: Push constants or uniform buffers
		return false;
	}

	//-----------------------------------------------------------------------

	bool cGPUProgram_GPU::SetVec3f(int alVarId, float afX, float afY, float afZ)
	{
		// TODO: Push constants or uniform buffers
		return false;
	}

	//-----------------------------------------------------------------------

	bool cGPUProgram_GPU::SetVec4f(int alVarId, float afX, float afY, float afZ, float afW)
	{
		// TODO: Push constants or uniform buffers
		return false;
	}

	//-----------------------------------------------------------------------

	bool cGPUProgram_GPU::SetMatrixf(int alVarId, const cMatrixf& aMtx)
	{
		// TODO: Push constants or uniform buffers
		return false;
	}

	//-----------------------------------------------------------------------

	bool cGPUProgram_GPU::SetMatrixf(int alVarId, eGpuShaderMatrix aType, eGpuShaderMatrixOp aOp)
	{
		// TODO: API matrices are not directly accessible in modern APIs
		// Need to maintain our own matrix stack in LowLevelGraphicsGPU
		return false;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cGPUProgram_GPU::LogProgramInfoLog(const tString& asError)
	{
		Error("-------- GPU Program Error -------\n");
		Error(" Program '%s':\n", msName.c_str());
		Error("%s", asError.c_str());
		Error("----------------------------------\n");
	}

	//-----------------------------------------------------------------------

} // namespace hpl
