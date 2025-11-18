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

#include "impl/GPUShader_GPU.h"
#include "impl/LowLevelGraphicsGPU.h"

#include "system/LowLevelSystem.h"
#include "system/Platform.h"
#include "system/String.h"
#include "resources/FileSearcher.h"

// glslang for GLSL→SPIR-V compilation
#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <glslang/SPIRV/GlslangToSpv.h>

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cGPUShader_GPU::cGPUShader_GPU(const tString& asName, eGpuShaderType aType, iLowLevelGraphics *apLowLevelGraphics)
		: iGpuShader(asName, _W(""), aType, eGpuProgramFormat_SPIRV)
	{
		mpLowLevelGraphics = apLowLevelGraphics;
		mpShader = nullptr;
		mvSPIRVBytecode.clear();

		// Initialize glslang (only once globally)
		static bool sGlslangInitialized = false;
		if (!sGlslangInitialized)
		{
			glslang::InitializeProcess();
			sGlslangInitialized = true;
		}
	}

	//-----------------------------------------------------------------------

	cGPUShader_GPU::~cGPUShader_GPU()
	{
		Unload();
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	bool cGPUShader_GPU::Reload()
	{
		if (GetFullPath() == _W(""))
			return false;

		return CreateFromFile(GetFullPath());
	}

	//-----------------------------------------------------------------------

	void cGPUShader_GPU::Unload()
	{
		if (mpShader)
		{
			// TODO: SDL_ReleaseGPUShader(device, mpShader);
			mpShader = nullptr;
		}
		mvSPIRVBytecode.clear();
	}

	//-----------------------------------------------------------------------

	void cGPUShader_GPU::Destroy()
	{
		Unload();
	}

	//-----------------------------------------------------------------------

	bool cGPUShader_GPU::CreateFromFile(const tWString& asFile, const tString& asEntry, bool abPrintInfoIfFail)
	{
		SetFullPath(asFile);

		// Load shader file
		size_t lFileSize = cPlatform::GetFileSize(asFile);
		if (lFileSize == 0)
		{
			Error("Couldn't load GPU shader '%s'\n", cString::To8Char(asFile).c_str());
			return false;
		}

		tString sSource;
		sSource.resize(lFileSize);
		cPlatform::CopyFileToBuffer(asFile, &sSource[0], lFileSize);

		return CreateFromString(sSource.c_str(), asEntry, abPrintInfoIfFail);
	}

	//-----------------------------------------------------------------------

	bool cGPUShader_GPU::CreateFromString(const char *apStringData, const tString& asEntry, bool abPrintInfoIfFail)
	{
		if (!apStringData)
			return false;

		if (mbDebugInfo)
		{
			Log("-------- Compiling GPU Shader '%s' (type: %d) --------\n", msName.c_str(), (int)mShaderType);
			LogShaderCode(apStringData);
		}

		// Compile GLSL to SPIR-V
		if (!CompileGLSLToSPIRV(apStringData, asEntry))
		{
			if (abPrintInfoIfFail)
			{
				Error("Failed to compile shader '%s'\n", msName.c_str());
			}
			return false;
		}

			// Create SDL_GPUShader from SPIR-V bytecode
	cLowLevelGraphicsGPU* pGPUGraphics = static_cast<cLowLevelGraphicsGPU*>(mpLowLevelGraphics);
	SDL_GPUDevice* pDevice = pGPUGraphics->GetGPUDevice();

	if (!pDevice)
	{
		Error("CreateFromString: No GPU device available for shader '%s'\n", msName.c_str());
		return false;
	}

	// Fill out shader create info
	SDL_GPUShaderCreateInfo createInfo = {};
	createInfo.code_size = mvSPIRVBytecode.size();
	createInfo.code = (const Uint8*)mvSPIRVBytecode.data();
	createInfo.entrypoint = asEntry.c_str();
	createInfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
	createInfo.stage = (mShaderType == eGpuShaderType_Vertex) ? SDL_GPU_SHADERSTAGE_VERTEX : SDL_GPU_SHADERSTAGE_FRAGMENT;

	// Resource counts (conservative estimates)
	createInfo.num_samplers = 8;          // Max samplers we support
	createInfo.num_storage_textures = 0;  // Not using these yet
	createInfo.num_storage_buffers = 0;   // Not using these yet
	createInfo.num_uniform_buffers = 2;   // CompatibilityMatrices (binding 0) + ShaderUniforms (binding 1)
	createInfo.props = 0;

	// Create the GPU shader
	mpShader = SDL_CreateGPUShader(pDevice, &createInfo);

	if (!mpShader)
	{
		Error("CreateFromString: Failed to create GPU shader '%s': %s\n",
			msName.c_str(), SDL_GetError());
		return false;
	}

	if (mbDebugInfo)
	{
		Log("  SDL_GPUShader created successfully\n");
	}

		return true;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	bool cGPUShader_GPU::CompileGLSLToSPIRV(const char* apGLSLSource, const tString& asEntry)
	{
		// Translate GLSL 1.20 to 4.50 for Vulkan compatibility
		tString sGLSL450 = TranslateGLSL120To450(apGLSLSource);

		if (mbDebugInfo)
		{
			Log("-------- Translated Shader Source --------\n");
			LogShaderCode(sGLSL450.c_str());
		}

		// Map our shader type to glslang's EShLanguage
		EShLanguage stage;
		switch (mShaderType)
		{
		case eGpuShaderType_Vertex:
			stage = EShLangVertex;
			break;
		case eGpuShaderType_Fragment:
			stage = EShLangFragment;
			break;
		default:
			Error("CompileGLSLToSPIRV: Unsupported shader type %d\n", (int)mShaderType);
			return false;
		}

		// Create shader object
		glslang::TShader shader(stage);
		const char* sourceStrings[] = { sGLSL450.c_str() };
		shader.setStrings(sourceStrings, 1);

		// Set environment for Vulkan/SPIR-V
		shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 100);
		shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
		shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

		// Get default resource limits
		const TBuiltInResource* pResources = GetDefaultResources();

		// Parse the shader
		EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
		if (!shader.parse(pResources, 100, false, messages))
		{
			Error("CompileGLSLToSPIRV: Failed to parse shader '%s'\n", msName.c_str());
			Error("%s\n", shader.getInfoLog());
			Error("%s\n", shader.getInfoDebugLog());

			if (mbDebugInfo)
			{
				LogShaderCode(sGLSL450.c_str());
			}
			return false;
		}

		// Link into a program
		glslang::TProgram program;
		program.addShader(&shader);

		if (!program.link(messages))
		{
			Error("CompileGLSLToSPIRV: Failed to link program '%s'\n", msName.c_str());
			Error("%s\n", program.getInfoLog());
			Error("%s\n", program.getInfoDebugLog());
			return false;
		}

		// Convert to SPIR-V
		std::vector<unsigned int> spirv;
		glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

		// Store SPIR-V bytecode
		mvSPIRVBytecode.resize(spirv.size() * sizeof(unsigned int));
		memcpy(mvSPIRVBytecode.data(), spirv.data(), mvSPIRVBytecode.size());

		if (mbDebugInfo)
		{
			Log("  GLSL→SPIR-V compilation successful\n");
			Log("  SPIR-V size: %zu bytes (%zu words)\n", mvSPIRVBytecode.size(), spirv.size());
		}

		return true;
	}

	//-----------------------------------------------------------------------

	tString cGPUShader_GPU::TranslateGLSL120To450(const char* apGLSL120Source)
	{
		tString sOutput = apGLSL120Source;
		int locationCounter = 0;

		// Replace #version 120 with #version 450
		size_t versionPos = sOutput.find("#version 120");
		if (versionPos != tString::npos)
		{
			sOutput.replace(versionPos, 12, "#version 450");
		}

		// Remove unsupported/unnecessary extensions
		size_t pos = 0;
		while ((pos = sOutput.find("#extension GL_ARB_draw_buffers", pos)) != tString::npos)
		{
			size_t lineEnd = sOutput.find("\n", pos);
			if (lineEnd != tString::npos)
			{
				sOutput.erase(pos, lineEnd - pos + 1);
			}
		}
		pos = 0;
		while ((pos = sOutput.find("#extension GL_ARB_texture_rectangle", pos)) != tString::npos)
		{
			size_t lineEnd = sOutput.find("\n", pos);
			if (lineEnd != tString::npos)
			{
				sOutput.erase(pos, lineEnd - pos + 1);
			}
		}

		// Update versionPos after removals
		versionPos = sOutput.find("#version 450");

		// Add compatibility declarations after #version
		size_t versionEnd = sOutput.find("\n", versionPos);
		if (versionEnd != tString::npos)
		{
			tString compatDecls = "\n// Compatibility declarations for legacy GLSL built-ins\n";

			if (mShaderType == eGpuShaderType_Vertex)
			{
				// Vertex shader needs matrix uniforms in a UBO (Vulkan requirement)
				compatDecls += "layout(binding = 0) uniform CompatibilityMatrices {\n";
				compatDecls += "    mat4 ModelViewProjectionMatrix;\n";
				compatDecls += "    mat4 ModelViewMatrix;\n";
				compatDecls += "    mat4 ProjectionMatrix;\n";
				compatDecls += "    mat3 NormalMatrix;\n";
				compatDecls += "};\n\n";

				// Vertex attributes - these can't use gl_ prefix
				compatDecls += "layout(location = 0) in vec4 vPosition;\n";
				compatDecls += "layout(location = 1) in vec3 vNormal;\n";
				compatDecls += "layout(location = 2) in vec4 vColor;\n";

				// Add MultiTexCoord attributes if needed
				for (int i = 0; i < 8; ++i)
				{
					char searchStr[32];
					sprintf(searchStr, "gl_MultiTexCoord%d", i);
					if (sOutput.find(searchStr) != tString::npos)
					{
						char attrDecl[128];
						sprintf(attrDecl, "layout(location = %d) in vec4 vTexCoord%d;\n", 3 + i, i);
						compatDecls += attrDecl;
					}
				}
			}
			else if (mShaderType == eGpuShaderType_Fragment)
			{
				// Fragment shader might need some uniforms too
				bool needsUBO = false;
				tString uboContent;

				if (sOutput.find("gl_ModelViewMatrix") != tString::npos)
				{
					uboContent += "    mat4 ModelViewMatrix;\n";
					needsUBO = true;
				}
				if (sOutput.find("gl_ProjectionMatrix") != tString::npos)
				{
					uboContent += "    mat4 ProjectionMatrix;\n";
					needsUBO = true;
				}

				if (needsUBO)
				{
					compatDecls += "layout(binding = 0) uniform CompatibilityMatrices {\n";
					compatDecls += uboContent;
					compatDecls += "};\n\n";
				}
			}

			sOutput.insert(versionEnd + 1, compatDecls);

			// Replace all references to old gl_* names with new names
			// Matrices (now accessed from UBO)
			pos = 0;
			while ((pos = sOutput.find("gl_ModelViewProjectionMatrix", pos)) != tString::npos)
			{
				sOutput.replace(pos, 28, "ModelViewProjectionMatrix   ");
				pos += 28;
			}
			pos = 0;
			while ((pos = sOutput.find("gl_ModelViewMatrix", pos)) != tString::npos)
			{
				sOutput.replace(pos, 18, "ModelViewMatrix   ");
				pos += 18;
			}
			pos = 0;
			while ((pos = sOutput.find("gl_ProjectionMatrix", pos)) != tString::npos)
			{
				sOutput.replace(pos, 19, "ProjectionMatrix   ");
				pos += 19;
			}
			pos = 0;
			while ((pos = sOutput.find("gl_NormalMatrix", pos)) != tString::npos)
			{
				sOutput.replace(pos, 15, "NormalMatrix   ");
				pos += 15;
			}

			// Vertex attributes
			pos = 0;
			while ((pos = sOutput.find("gl_Vertex", pos)) != tString::npos)
			{
				// Make sure it's not part of a longer identifier
				if (pos + 9 >= sOutput.length() || !isalnum(sOutput[pos + 9]))
				{
					sOutput.replace(pos, 9, "vPosition");
					pos += 9;
				}
				else
				{
					pos += 9;
				}
			}
			pos = 0;
			while ((pos = sOutput.find("gl_Normal", pos)) != tString::npos)
			{
				if (pos + 9 >= sOutput.length() || !isalnum(sOutput[pos + 9]))
				{
					sOutput.replace(pos, 9, "vNormal  ");
					pos += 9;
				}
				else
				{
					pos += 9;
				}
			}
			pos = 0;
			while ((pos = sOutput.find("gl_Color", pos)) != tString::npos)
			{
				if (pos + 8 >= sOutput.length() || !isalnum(sOutput[pos + 8]))
				{
					sOutput.replace(pos, 8, "vColor  ");
					pos += 8;
				}
				else
				{
					pos += 8;
				}
			}

			// MultiTexCoord
			for (int i = 0; i < 8; ++i)
			{
				char oldStr[32], newStr[32];
				sprintf(oldStr, "gl_MultiTexCoord%d", i);
				sprintf(newStr, "vTexCoord%d       ", i);

				pos = 0;
				while ((pos = sOutput.find(oldStr, pos)) != tString::npos)
				{
					sOutput.replace(pos, strlen(oldStr), newStr);
					pos += strlen(newStr);
				}
			}

			// Update versionPos to account for inserted text
			versionPos = sOutput.find("#version 450");
		}

		// Handle varying keyword based on shader type
		if (mShaderType == eGpuShaderType_Vertex)
		{
			// Vertex shader: varying -> layout(location=N) out
			size_t pos = 0;
			while ((pos = sOutput.find("varying ", pos)) != tString::npos)
			{
				char layoutStr[64];
				sprintf(layoutStr, "layout(location = %d) out ", locationCounter++);

				// Replace "varying " with the layout-qualified version
				sOutput.replace(pos, 8, layoutStr);
				pos += strlen(layoutStr);
			}

			// Replace ftransform() with manual calculation
			// ftransform() is equivalent to: ModelViewProjectionMatrix * vPosition (using new names)
			pos = 0;
			while ((pos = sOutput.find("ftransform()")) != tString::npos)
			{
				sOutput.replace(pos, 12, "(ModelViewProjectionMatrix * vPosition)");
				pos += 39;
			}
		}
		else if (mShaderType == eGpuShaderType_Fragment)
		{
			// Fragment shader: varying -> layout(location=N) in
			size_t pos = 0;
			while ((pos = sOutput.find("varying ", pos)) != tString::npos)
			{
				char layoutStr[64];
				sprintf(layoutStr, "layout(location = %d) in ", locationCounter++);

				// Replace "varying " with the layout-qualified version
				sOutput.replace(pos, 8, layoutStr);
				pos += strlen(layoutStr);
			}

			// Replace texture2D with texture
			pos = 0;
			while ((pos = sOutput.find("texture2D(")) != tString::npos)
			{
				sOutput.replace(pos, 10, "texture  (");
				pos += 10;
			}

			// Replace texture2DRect with texture
			pos = 0;
			while ((pos = sOutput.find("texture2DRect(")) != tString::npos)
			{
				sOutput.replace(pos, 14, "texture      (");
				pos += 14;
			}

			// Replace textureCube with texture
			pos = 0;
			while ((pos = sOutput.find("textureCube(")) != tString::npos)
			{
				sOutput.replace(pos, 12, "texture    (");
				pos += 12;
			}

			// Replace texture1D with texture
			pos = 0;
			while ((pos = sOutput.find("texture1D(")) != tString::npos)
			{
				sOutput.replace(pos, 10, "texture  (");
				pos += 10;
			}

			// Handle gl_FragData[N] - replace with explicit output variables
			if (sOutput.find("gl_FragData") != tString::npos)
			{
				// Find all unique indices used
				bool usedIndices[8] = {false};
				for (int i = 0; i < 8; ++i)
				{
					char searchStr[32];
					sprintf(searchStr, "gl_FragData[%d]", i);
					if (sOutput.find(searchStr) != tString::npos)
					{
						usedIndices[i] = true;
					}
				}

				// Add output variable declarations
				size_t versionEnd = sOutput.find("\n", versionPos);
				if (versionEnd != tString::npos)
				{
					tString outputDecls = "\n// Fragment outputs (gl_FragData replacement)\n";
					for (int i = 0; i < 8; ++i)
					{
						if (usedIndices[i])
						{
							char declStr[128];
							sprintf(declStr, "layout(location = %d) out vec4 fragData%d;\n", i, i);
							outputDecls += declStr;
						}
					}
					sOutput.insert(versionEnd + 1, outputDecls);
				}

				// Replace all gl_FragData[N] with fragDataN
				for (int i = 0; i < 8; ++i)
				{
					if (usedIndices[i])
					{
						char oldStr[32], newStr[32];
						sprintf(oldStr, "gl_FragData[%d]", i);
						sprintf(newStr, "fragData%d      ", i);

						pos = 0;
						while ((pos = sOutput.find(oldStr, pos)) != tString::npos)
						{
							sOutput.replace(pos, strlen(oldStr), newStr);
							pos += strlen(newStr);
						}
					}
				}

				// Update versionPos after insertions
				versionPos = sOutput.find("#version 450");
			}

			// Add output variable declaration after #version if gl_FragColor is used
			if (sOutput.find("gl_FragColor") != tString::npos)
			{
				size_t versionEnd = sOutput.find("\n", versionPos);
				if (versionEnd != tString::npos)
				{
					sOutput.insert(versionEnd + 1, "\nlayout(location = 0) out vec4 fragColor;\n");
				}

				// Replace gl_FragColor with fragColor
				size_t fragColorPos = 0;
				while ((fragColorPos = sOutput.find("gl_FragColor", fragColorPos)) != tString::npos)
				{
					sOutput.replace(fragColorPos, 12, "fragColor   ");
					fragColorPos += 12;
				}
			}
		}

		// Replace gl_TexCoord[N] with custom varyings (for both vertex and fragment)
		// We'll need to declare these varyings at the top after the version directive
		bool hasTexCoord = sOutput.find("gl_TexCoord") != tString::npos;
		if (hasTexCoord)
		{
			// Find where to insert varying declarations (after #version line)
			size_t insertPos = sOutput.find("\n", versionPos);
			if (insertPos != tString::npos)
			{
				tString varyingDecls = "\n// Texture coordinate varyings\n";

				// Check which tex coord indices are used and declare them
				for (int i = 0; i < 8; ++i)
				{
					char searchStr[32];
					sprintf(searchStr, "gl_TexCoord[%d]", i);
					if (sOutput.find(searchStr) != tString::npos)
					{
						char varyingDecl[128];
						sprintf(varyingDecl, "layout(location = %d) %s vec4 v_texCoord%d;\n",
							locationCounter++,
							mShaderType == eGpuShaderType_Vertex ? "out" : "in ", i);
						varyingDecls += varyingDecl;
					}
				}

				sOutput.insert(insertPos + 1, varyingDecls);

				// Replace all gl_TexCoord[N] with v_texCoordN
				for (int i = 0; i < 8; ++i)
				{
					char oldStr[32], newStr[32];
					sprintf(oldStr, "gl_TexCoord[%d]", i);
					sprintf(newStr, "v_texCoord%d    ", i);

					size_t pos = 0;
					while ((pos = sOutput.find(oldStr, pos)) != tString::npos)
					{
						sOutput.replace(pos, strlen(oldStr), newStr);
						pos += strlen(newStr);
					}
				}
			}
		}

			// Collect all non-sampler uniforms and move them to a UBO
	// Vulkan requires all non-opaque uniforms to be in uniform blocks
	std::vector<tString> uniformDeclarations;
	pos = 0;

	while ((pos = sOutput.find("uniform ", pos)) != tString::npos)
	{
		// Check if this is a sampler (opaque type) - those get binding qualifiers instead
		size_t afterUniform = pos + 8;
		tString restOfLine = sOutput.substr(afterUniform, sOutput.find("\n", afterUniform) - afterUniform);

		// Skip if it's a sampler (opaque type) or already in a block
		if (restOfLine.find("sampler") != tString::npos ||
			restOfLine.find("image") != tString::npos)
		{
			pos += 8;
			continue;
		}

		// Find the end of this uniform declaration (semicolon)
		size_t semicolon = sOutput.find(";", pos);
		if (semicolon == tString::npos)
		{
			pos += 8;
			continue;
		}

		// Extract the uniform declaration
		tString uniformDecl = sOutput.substr(pos, semicolon - pos + 1);

		// Check if it's already in a layout-qualified block
		size_t lineStart = sOutput.rfind("\n", pos);
		if (lineStart == tString::npos) lineStart = 0;
		else lineStart++;

		tString linePrefix = sOutput.substr(lineStart, pos - lineStart);
		if (linePrefix.find("layout") != tString::npos || linePrefix.find("}") != tString::npos)
		{
			pos = semicolon + 1;
			continue;
		}

		// Store this uniform and remove it from the shader
		uniformDeclarations.push_back(uniformDecl.substr(8)); // Skip "uniform "
		sOutput.erase(lineStart, semicolon - lineStart + 2); // +2 for semicolon and newline

		// Don't advance pos since we removed text
	}

	// If we found non-sampler uniforms, create a UBO for them
	if (!uniformDeclarations.empty())
	{
		versionPos = sOutput.find("#version 450");
		versionEnd = sOutput.find("\n", versionPos);

		if (versionEnd != tString::npos)
		{
			tString uboDecl = "\n// Non-sampler uniforms (Vulkan UBO requirement)\n";
			uboDecl += "layout(binding = 1) uniform ShaderUniforms {\n";
			for (size_t i = 0; i < uniformDeclarations.size(); ++i)
			{
				uboDecl += "    " + uniformDeclarations[i] + "\n";
			}
			uboDecl += "};\n";

			sOutput.insert(versionEnd + 1, uboDecl);
		}
	}

	// Add layout(binding=X) to all sampler uniforms
	// Binding 0 is reserved for CompatibilityMatrices, binding 1 for ShaderUniforms
	// Start samplers at binding 2
	int bindingCounter = 2;
		pos = 0;

		// Find all "uniform sampler" declarations and add binding qualifiers
		while ((pos = sOutput.find("uniform sampler", pos)) != tString::npos)
		{
			// Check if this uniform already has a layout qualifier
			size_t lineStart = sOutput.rfind("\n", pos);
			if (lineStart == tString::npos) lineStart = 0;
			else lineStart++; // Move past the newline

			tString linePrefix = sOutput.substr(lineStart, pos - lineStart);

			// If no layout qualifier exists on this line, add one
			if (linePrefix.find("layout") == tString::npos)
			{
				char layoutStr[64];
				sprintf(layoutStr, "layout(binding = %d) ", bindingCounter++);
				sOutput.insert(pos, layoutStr);
				pos += strlen(layoutStr);
			}

			pos += 15; // Skip past "uniform sampler"
		}

		if (mbDebugInfo)
		{
			Log("  Translated GLSL 1.20 -> 4.50 for Vulkan compatibility\n");
			Log("  Added layout(binding=X) qualifiers to sampler uniforms\n");
		}

		return sOutput;
	}

	//-----------------------------------------------------------------------

	void cGPUShader_GPU::LogShaderInfoLog(const tString& asError)
	{
		Error("-------- GPU Shader Error -------\n");
		Error(" Shader '%s':\n", msName.c_str());
		Error("%s", asError.c_str());
		Error("----------------------------------\n");
	}

	//-----------------------------------------------------------------------

	void cGPUShader_GPU::LogShaderCode(const char *apStringData)
	{
		if (!apStringData) return;

		Log("-------- Shader Source Code -------\n");

		int lLineNum = 1;
		const char* pLine = apStringData;
		while (*pLine != '\0')
		{
			const char* pLineEnd = pLine;
			while (*pLineEnd != '\0' && *pLineEnd != '\n')
				pLineEnd++;

			Log("%3d: %.*s\n", lLineNum, (int)(pLineEnd - pLine), pLine);

			if (*pLineEnd == '\n')
				pLineEnd++;
			pLine = pLineEnd;
			lLineNum++;
		}

		Log("-----------------------------------\n");
	}

	//-----------------------------------------------------------------------

} // namespace hpl
