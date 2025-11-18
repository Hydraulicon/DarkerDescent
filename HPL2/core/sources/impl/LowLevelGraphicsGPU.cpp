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

#include "impl/LowLevelGraphicsGPU.h"
#include "impl/GPUShader_GPU.h"
#include "impl/GPUProgram_GPU.h"
#include "impl/VertexBufferGPU.h"
#include "impl/GPUTexture.h"
#include "impl/GPUFrameBuffer.h"

#include "system/LowLevelSystem.h"
#include "system/String.h"
#include "graphics/Texture.h"
#include "graphics/VertexBuffer.h"
#include "graphics/FrameBuffer.h"
#include "graphics/GPUProgram.h"
#include "graphics/GPUShader.h"
#include "graphics/FontData.h"
#include "graphics/OcclusionQuery.h"
#include "graphics/Bitmap.h"
#include "graphics/GraphicsTypes.h"

#include "math/Math.h"

// glslang for shader compilation
#include <glslang/Public/ShaderLang.h>

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cLowLevelGraphicsGPU::cLowLevelGraphicsGPU()
	{
		mpWindow = nullptr;
		mpGPUDevice = nullptr;
		mpFrameBuffer = nullptr;

		mlCurrentFrame = 0;
		for (int i = 0; i < kMaxFramesInFlight; ++i)
		{
			mpCommandBuffers[i] = nullptr;
		}

		// Initialize render state
		mbColorWriteR = true;
		mbColorWriteG = true;
		mbColorWriteB = true;
		mbColorWriteA = true;
		mbDepthWrite = true;

		mbCullActive = false;
		mCullMode = eCullMode_Clockwise;

		mbDepthTestActive = false;
		mDepthTestFunc = eDepthTestFunc_LessOrEqual;

		mbAlphaTestActive = false;
		mAlphaTestFunc = eAlphaTestFunc_GreaterOrEqual;
		mfAlphaTestFuncRef = 0.0f;

		mbScissorActive = false;

		mbBlendActive = false;
		mBlendSrcFactor = eBlendFunc_One;
		mBlendDestFactor = eBlendFunc_Zero;

		mfGammaCorrection = 1.0f;

		mbInitHasBeenRun = false;
		mbGrab = false;

		// Clear values
		mClearColor = cColor(0.0f, 0.0f, 0.0f, 1.0f);
		mfClearDepth = 1.0f;
		mlClearStencil = 0;

		// Vertex batching
		mpVertexArray = nullptr;
		mlVertexCount = 0;
		mpIndexArray = nullptr;
		mlIndexCount = 0;
		mlBatchStride = 0;
		mlBatchArraySize = 0;

		for (int i = 0; i < kMaxTextureUnits; ++i)
		{
			mpTexCoordArray[i] = nullptr;
			mbTexCoordArrayActive[i] = false;
			mlTexCoordArrayCount[i] = 0;
		}

		// Initialize matrix stacks
		mCurrentModelViewMatrix = cMatrixf::Identity;
		mCurrentProjectionMatrix = cMatrixf::Identity;
		mCurrentTextureMatrix = cMatrixf::Identity;
	}

	//-----------------------------------------------------------------------

	cLowLevelGraphicsGPU::~cLowLevelGraphicsGPU()
	{
		// Destroy vertex batching arrays
		DestroyBatchArrays();

		// Finalize glslang
		if (mbInitHasBeenRun)
		{
			glslang::FinalizeProcess();
		}

		// Destroy command buffers (if any)
		// Note: SDL_GPU manages command buffer lifecycle, no explicit cleanup needed

		// Destroy GPU device
		if (mpGPUDevice)
		{
			SDL_DestroyGPUDevice(mpGPUDevice);
			mpGPUDevice = nullptr;
		}

		// Destroy window
		if (mpWindow)
		{
			SDL_DestroyWindow(mpWindow);
			mpWindow = nullptr;
		}
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// GENERAL SETUP
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	bool cLowLevelGraphicsGPU::Init(int alWidth, int alHeight, int alDisplay, int alBpp, int abFullscreen, int alMultisampling,
		eGpuProgramFormat aGpuProgramFormat, const tString& asWindowCaption,
		const cVector2l& avWindowPos)
	{
		if (mbInitHasBeenRun)
		{
			Log("WARNING: cLowLevelGraphicsGPU::Init() called multiple times!\n");
			return true;
		}

		Log("-------- Initializing SDL_GPU Renderer --------\n");

		// Store parameters
		mvScreenSize = cVector2l(alWidth, alHeight);
		mlDisplay = alDisplay;
		mlBpp = alBpp;
		mbFullscreen = abFullscreen != 0;
		mlMultisampling = alMultisampling;
		mGpuProgramFormat = aGpuProgramFormat;

		// Create window
		Uint32 windowFlags = 0;
		if (mbFullscreen)
		{
			windowFlags |= SDL_WINDOW_FULLSCREEN;
		}

		Log("  Creating SDL window (%dx%d, %s)...\n", alWidth, alHeight, mbFullscreen ? "fullscreen" : "windowed");

		mpWindow = SDL_CreateWindow(
			asWindowCaption.c_str(),
			alWidth,
			alHeight,
			windowFlags
		);

		if (!mpWindow)
		{
			Error("Failed to create SDL window: %s\n", SDL_GetError());
			return false;
		}

		// Set window position (if not fullscreen)
		if (!mbFullscreen && avWindowPos.x >= 0 && avWindowPos.y >= 0)
		{
			SDL_SetWindowPosition(mpWindow, avWindowPos.x, avWindowPos.y);
		}

		// Create SDL_GPU device (preferring Vulkan backend)
		Log("  Creating SDL_GPU device (Vulkan preferred)...\n");

		mpGPUDevice = SDL_CreateGPUDevice(
			SDL_GPU_SHADERFORMAT_SPIRV,  // Request SPIR-V shader format (Vulkan)
			true,                        // Enable debug mode in debug builds
			nullptr                      // Use default device name
		);

		if (!mpGPUDevice)
		{
			Error("Failed to create SDL_GPU device: %s\n", SDL_GetError());
			return false;
		}

		// Claim window for GPU rendering
		if (!SDL_ClaimWindowForGPUDevice(mpGPUDevice, mpWindow))
		{
			Error("Failed to claim window for GPU device: %s\n", SDL_GetError());
			return false;
		}

		// Log success
		Log("  SDL_GPU device created successfully\n");
		Log("  Shader format: SPIR-V (Vulkan)\n");
		Log("  Window claimed for GPU rendering\n");

		// Initialize glslang for shader compilation
		Log("  Initializing glslang for GLSL→SPIR-V compilation...\n");
		glslang::InitializeProcess();

		// Setup vertex batching
		SetUpBatchArrays();

		mbInitHasBeenRun = true;

		Log("-------- SDL_GPU Renderer Initialized Successfully --------\n");

		return true;
	}

	//-----------------------------------------------------------------------

	int cLowLevelGraphicsGPU::GetCaps(eGraphicCaps aType)
	{
		// TODO: Query actual GPU capabilities
		switch (aType)
		{
		case eGraphicCaps_MaxTextureImageUnits:
			return 16; // GPU can handle many texture units
		case eGraphicCaps_MaxTextureCoordUnits:
			return 8;
		case eGraphicCaps_MaxAnisotropicFiltering:
			return 16;
		case eGraphicCaps_TextureCompression:
			return 1; // Assume supported
		case eGraphicCaps_TextureCompression_DXTC:
			return 1; // DDS/DXT compression supported
		case eGraphicCaps_VertexBufferObject:
			return 1; // Always supported in modern APIs
		case eGraphicCaps_RenderToTexture:
			return 1; // Framebuffers supported
		case eGraphicCaps_TwoSideStencil:
			return 1; // Modern feature
		case eGraphicCaps_AnisotropicFiltering:
			return 1; // Supported
		case eGraphicCaps_Multisampling:
			return 1; // MSAA supported
		case eGraphicCaps_AutoGenerateMipMaps:
			return 1; // Can auto-generate mipmaps
		case eGraphicCaps_TextureTargetRectangle:
			return 0; // Legacy OpenGL feature, not needed
		case eGraphicCaps_MaxUserClipPlanes:
			return 0; // Use shader-based clipping instead
		default:
			return 0;
		}
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::ShowCursor(bool abX)
	{
		if (abX)
			SDL_ShowCursor();
		else
			SDL_HideCursor();
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetWindowGrab(bool abX)
	{
		mbGrab = abX;
		SDL_SetWindowMouseGrab(mpWindow, abX);
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetRelativeMouse(bool abX)
	{
		SDL_SetWindowRelativeMouseMode(mpWindow, abX);
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetWindowCaption(const tString& asName)
	{
		SDL_SetWindowTitle(mpWindow, asName.c_str());
	}

	//-----------------------------------------------------------------------

	bool cLowLevelGraphicsGPU::GetWindowMouseFocus()
	{
		return (SDL_GetWindowFlags(mpWindow) & SDL_WINDOW_MOUSE_FOCUS) != 0;
	}

	//-----------------------------------------------------------------------

	bool cLowLevelGraphicsGPU::GetWindowInputFocus()
	{
		return (SDL_GetWindowFlags(mpWindow) & SDL_WINDOW_INPUT_FOCUS) != 0;
	}

	//-----------------------------------------------------------------------

	bool cLowLevelGraphicsGPU::GetWindowIsVisible()
	{
		return !(SDL_GetWindowFlags(mpWindow) & SDL_WINDOW_HIDDEN);
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetVsyncActive(bool abX, bool abAdaptive)
	{
		// TODO: Implement vsync control
		// SDL_GPU handles vsync via present mode, will implement later
		Log("SetVsyncActive: TODO (vsync=%d, adaptive=%d)\n", abX, abAdaptive);
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetMultisamplingActive(bool abX)
	{
		// TODO: Implement multisampling control
		// SDL_GPU handles MSAA via sample count in pipeline creation
		Log("SetMultisamplingActive: TODO (active=%d)\n", abX);
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetGammaCorrection(float afX)
	{
		mfGammaCorrection = afX;
		// Note: Gamma correction will be handled via post-processing shader
		// SDL3 removed hardware gamma functionality
	}

	//-----------------------------------------------------------------------

	float cLowLevelGraphicsGPU::GetGammaCorrection()
	{
		return mfGammaCorrection;
	}

	//-----------------------------------------------------------------------

	cVector2f cLowLevelGraphicsGPU::GetScreenSizeFloat()
	{
		return cVector2f((float)mvScreenSize.x, (float)mvScreenSize.y);
	}

	//-----------------------------------------------------------------------

	const cVector2l& cLowLevelGraphicsGPU::GetScreenSizeInt()
	{
		return mvScreenSize;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// DATA CREATION
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iFontData* cLowLevelGraphicsGPU::CreateFontData(const tString& asName)
	{
		// TODO: Implement font data creation
		Error("CreateFontData: TODO\n");
		return nullptr;
	}

	//-----------------------------------------------------------------------

	iTexture* cLowLevelGraphicsGPU::CreateTexture(const tString& asName, eTextureType aType, eTextureUsage aUsage)
	{
		return hplNew(cGPUTexture, (asName, aType, aUsage, this));
	}

	//-----------------------------------------------------------------------

	iVertexBuffer* cLowLevelGraphicsGPU::CreateVertexBuffer(eVertexBufferType aType,
		eVertexBufferDrawType aDrawType,
		eVertexBufferUsageType aUsageType,
		int alReserveVtxSize, int alReserveIdxSize)
	{
		return hplNew(cVertexBufferGPU, (this, aType, aDrawType, aUsageType, alReserveVtxSize, alReserveIdxSize));
	}

	//-----------------------------------------------------------------------

	iGpuProgram* cLowLevelGraphicsGPU::CreateGpuProgram(const tString& asName)
	{
		return hplNew(cGPUProgram_GPU, (asName));
	}

	//-----------------------------------------------------------------------

	iGpuShader* cLowLevelGraphicsGPU::CreateGpuShader(const tString& asName, eGpuShaderType aType)
	{
		return hplNew(cGPUShader_GPU, (asName, aType, this));
	}

	//-----------------------------------------------------------------------

	iFrameBuffer* cLowLevelGraphicsGPU::CreateFrameBuffer(const tString& asName)
	{
		return hplNew(cFrameBufferGPU, (asName, this));
	}

	//-----------------------------------------------------------------------

	iDepthStencilBuffer* cLowLevelGraphicsGPU::CreateDepthStencilBuffer(const cVector2l& avSize, int alDepthBits, int alStencilBits)
	{
		return hplNew(cDepthStencilBufferGPU, (avSize, alDepthBits, alStencilBits));
	}

	//-----------------------------------------------------------------------

	iOcclusionQuery* cLowLevelGraphicsGPU::CreateOcclusionQuery()
	{
		// TODO: Implement occlusion query (if SDL_GPU supports)
		Error("CreateOcclusionQuery: TODO\n");
		return nullptr;
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// FRAME BUFFER OPERATIONS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::ClearFrameBuffer(tClearFrameBufferFlag aFlags)
	{
		// TODO: Implement clear operations
		// Will need to start a render pass with load op = CLEAR
		// For now, this is a stub
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetClearColor(const cColor& aCol)
	{
		mClearColor = aCol;
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetClearDepth(float afDepth)
	{
		mfClearDepth = afDepth;
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetClearStencil(int alVal)
	{
		mlClearStencil = alVal;
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::CopyFrameBufferToTexure(iTexture* apTex, const cVector2l& avPos,
		const cVector2l& avSize, const cVector2l& avTexOffset)
	{
		// TODO: Implement framebuffer to texture copy
		Error("CopyFrameBufferToTexure: TODO\n");
	}

	//-----------------------------------------------------------------------

	cBitmap* cLowLevelGraphicsGPU::CopyFrameBufferToBitmap(const cVector2l& avScreenPos, const cVector2l& avScreenSize)
	{
		// TODO: Implement framebuffer to bitmap copy (for screenshots)
		Error("CopyFrameBufferToBitmap: TODO\n");
		return nullptr;
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::WaitAndFinishRendering()
	{
		if (mpGPUDevice)
		{
			SDL_WaitForGPUIdle(mpGPUDevice);
		}
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::FlushRendering()
	{
		// TODO: Implement rendering flush
		// For now, submit any pending command buffers
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SwapBuffers()
	{
		// TODO: Implement buffer swap
		// This will involve:
		// 1. Ending current render pass
		// 2. Submitting command buffer
		// 3. Presenting swap chain
		// 4. Advancing frame index

		// For now, just clear the window
		if (mpGPUDevice && mpWindow)
		{
			// Acquire swapchain texture
			SDL_GPUCommandBuffer* cmdBuf = SDL_AcquireGPUCommandBuffer(mpGPUDevice);
			if (cmdBuf)
			{
				SDL_GPUTexture* swapchainTexture = nullptr;
				if (SDL_AcquireGPUSwapchainTexture(cmdBuf, mpWindow, &swapchainTexture, nullptr, nullptr))
				{
					if (swapchainTexture)
					{
						// Begin render pass with clear
						SDL_GPUColorTargetInfo colorTarget{};
						colorTarget.texture = swapchainTexture;
						colorTarget.clear_color.r = mClearColor.r;
						colorTarget.clear_color.g = mClearColor.g;
						colorTarget.clear_color.b = mClearColor.b;
						colorTarget.clear_color.a = mClearColor.a;
						colorTarget.load_op = SDL_GPU_LOADOP_CLEAR;
						colorTarget.store_op = SDL_GPU_STOREOP_STORE;

						SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdBuf, &colorTarget, 1, nullptr);
						if (renderPass)
						{
							// End render pass immediately (just clearing)
							SDL_EndGPURenderPass(renderPass);
						}
					}
				}

				// Submit command buffer
				SDL_SubmitGPUCommandBuffer(cmdBuf);
			}
		}

		// Advance frame index
		mlCurrentFrame = (mlCurrentFrame + 1) % kMaxFramesInFlight;
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetCurrentFrameBuffer(iFrameBuffer* apFrameBuffer, const cVector2l& avPos, const cVector2l& avSize)
	{
		mpFrameBuffer = apFrameBuffer;
		mvFrameBufferPos = avPos;
		mvFrameBufferSize = avSize;
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetFrameBufferDrawTargets(int* apTargets, int alNumOfTargets)
	{
		// TODO: Implement MRT draw target selection
		Error("SetFrameBufferDrawTargets: TODO\n");
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// RENDER STATE
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetColorWriteActive(bool abR, bool abG, bool abB, bool abA)
	{
		mbColorWriteR = abR;
		mbColorWriteG = abG;
		mbColorWriteB = abB;
		mbColorWriteA = abA;
		// State will be applied when creating pipeline
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetDepthWriteActive(bool abX)
	{
		mbDepthWrite = abX;
		// State will be applied when creating pipeline
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetCullActive(bool abX)
	{
		mbCullActive = abX;
		// State will be applied when creating pipeline
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetCullMode(eCullMode aMode)
	{
		mCullMode = aMode;
		// State will be applied when creating pipeline
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetDepthTestActive(bool abX)
	{
		mbDepthTestActive = abX;
		// State will be applied when creating pipeline
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetDepthTestFunc(eDepthTestFunc aFunc)
	{
		mDepthTestFunc = aFunc;
		// State will be applied when creating pipeline
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetAlphaTestActive(bool abX)
	{
		mbAlphaTestActive = abX;
		// Alpha test will be emulated in shader (discard)
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetAlphaTestFunc(eAlphaTestFunc aFunc, float afRef)
	{
		mAlphaTestFunc = aFunc;
		mfAlphaTestFuncRef = afRef;
		// Alpha test will be emulated in shader (discard)
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetStencilActive(bool abX)
	{
		// TODO: Implement stencil state
		// State will be applied when creating pipeline
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetStencilWriteMask(unsigned int alMask)
	{
		// TODO: Implement stencil write mask
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetStencil(eStencilFunc aFunc, int alRef, unsigned int aMask,
		eStencilOp aFailOp, eStencilOp aZFailOp, eStencilOp aZPassOp)
	{
		// TODO: Implement stencil state
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetStencilTwoSide(eStencilFunc aFrontFunc, eStencilFunc aBackFunc,
		int alRef, unsigned int aMask,
		eStencilOp aFrontFailOp, eStencilOp aFrontZFailOp, eStencilOp aFrontZPassOp,
		eStencilOp aBackFailOp, eStencilOp aBackZFailOp, eStencilOp aBackZPassOp)
	{
		// TODO: Implement two-sided stencil state
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetScissorActive(bool abX)
	{
		mbScissorActive = abX;
		// Scissor can be set dynamically in SDL_GPU
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetScissorRect(const cVector2l& avPos, const cVector2l& avSize)
	{
		mvScissorPos = avPos;
		mvScissorSize = avSize;
		// Scissor can be set dynamically in SDL_GPU
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetClipPlane(int alIdx, const cPlanef& aPlane)
	{
		if (alIdx >= 0 && alIdx < kMaxClipPlanes)
		{
			mvClipPlanes[alIdx] = aPlane;
		}
		// Clip planes will be implemented via shader (if needed)
	}

	//-----------------------------------------------------------------------

	cPlanef cLowLevelGraphicsGPU::GetClipPlane(int alIdx)
	{
		if (alIdx >= 0 && alIdx < kMaxClipPlanes)
		{
			return mvClipPlanes[alIdx];
		}
		return cPlanef();
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetClipPlaneActive(int alIdx, bool abX)
	{
		// TODO: Track clip plane active state
		// Clip planes will be implemented via shader (if needed)
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetColor(const cColor& aColor)
	{
		// TODO: Set current drawing color
		// This will be passed to shaders via push constants or uniforms
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetBlendActive(bool abX)
	{
		mbBlendActive = abX;
		// State will be applied when creating pipeline
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetBlendFunc(eBlendFunc aSrcFactor, eBlendFunc aDestFactor)
	{
		mBlendSrcFactor = aSrcFactor;
		mBlendDestFactor = aDestFactor;
		// State will be applied when creating pipeline
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetBlendFuncSeparate(eBlendFunc aSrcFactorColor, eBlendFunc aDestFactorColor,
		eBlendFunc aSrcFactorAlpha, eBlendFunc aDestFactorAlpha)
	{
		// TODO: Implement separate blend functions
		// For now, just use the color blend factors
		SetBlendFunc(aSrcFactorColor, aDestFactorColor);
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetPolygonOffsetActive(bool abX)
	{
		// TODO: Implement polygon offset
		// State will be applied when creating pipeline
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetPolygonOffset(float afBias, float afSlopeScaleBias)
	{
		// TODO: Implement polygon offset parameters
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// MATRIX OPERATIONS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::PushMatrix(eMatrix aMtxType)
	{
		switch (aMtxType)
		{
		case eMatrix_ModelView:
			mvModelViewMatrixStack.push_back(mCurrentModelViewMatrix);
			break;
		case eMatrix_Projection:
			mvProjectionMatrixStack.push_back(mCurrentProjectionMatrix);
			break;
		case eMatrix_Texture:
			mvTextureMatrixStack.push_back(mCurrentTextureMatrix);
			break;
		}
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::PopMatrix(eMatrix aMtxType)
	{
		switch (aMtxType)
		{
		case eMatrix_ModelView:
			if (!mvModelViewMatrixStack.empty())
			{
				mCurrentModelViewMatrix = mvModelViewMatrixStack.back();
				mvModelViewMatrixStack.pop_back();
			}
			break;
		case eMatrix_Projection:
			if (!mvProjectionMatrixStack.empty())
			{
				mCurrentProjectionMatrix = mvProjectionMatrixStack.back();
				mvProjectionMatrixStack.pop_back();
			}
			break;
		case eMatrix_Texture:
			if (!mvTextureMatrixStack.empty())
			{
				mCurrentTextureMatrix = mvTextureMatrixStack.back();
				mvTextureMatrixStack.pop_back();
			}
			break;
		}
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetIdentityMatrix(eMatrix aMtxType)
	{
		switch (aMtxType)
		{
		case eMatrix_ModelView:
			mCurrentModelViewMatrix = cMatrixf::Identity;
			break;
		case eMatrix_Projection:
			mCurrentProjectionMatrix = cMatrixf::Identity;
			break;
		case eMatrix_Texture:
			mCurrentTextureMatrix = cMatrixf::Identity;
			break;
		}
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetMatrix(eMatrix aMtxType, const cMatrixf& a_mtxA)
	{
		switch (aMtxType)
		{
		case eMatrix_ModelView:
			mCurrentModelViewMatrix = a_mtxA;
			break;
		case eMatrix_Projection:
			mCurrentProjectionMatrix = a_mtxA;
			break;
		case eMatrix_Texture:
			mCurrentTextureMatrix = a_mtxA;
			break;
		}
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetOrthoProjection(const cVector2f& avSize, float afMin, float afMax)
	{
		// Create orthographic projection matrix
		mCurrentProjectionMatrix = cMath::MatrixOrthographicProjection(afMin, afMax, avSize);
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetOrthoProjection(const cVector3f& avMin, const cVector3f& avMax)
	{
		// Create orthographic projection matrix
		// Note: May need custom implementation for min/max variant
		cVector2f vSize(avMax.x - avMin.x, avMax.y - avMin.y);
		mCurrentProjectionMatrix = cMath::MatrixOrthographicProjection(avMin.z, avMax.z, vSize);
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// TEXTURE OPERATIONS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetTexture(unsigned int alUnit, iTexture* apTex)
	{
		// TODO: Bind texture to unit
		// Textures will be bound via descriptor sets in SDL_GPU
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetActiveTextureUnit(unsigned int alUnit)
	{
		// TODO: Set active texture unit
		// Not needed in modern APIs - textures are bound by slot
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetTextureEnv(eTextureParam aParam, int alVal)
	{
		// TODO: Set texture environment (legacy fixed-function)
		// Not applicable in modern APIs
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetTextureConstantColor(const cColor& aColor)
	{
		// TODO: Set texture constant color (legacy)
		// Not applicable in modern APIs
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// DRAWING
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawTriangle(tVertexVec& avVtx)
	{
		// TODO: Implement triangle drawing via vertex batching
		Error("DrawTriangle: TODO\n");
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawQuad(const cVector3f& avPos, const cVector2f& avSize, const cColor& aColor)
	{
		// TODO: Implement quad drawing via vertex batching
		// For now, stub
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawQuad(const cVector3f& avPos, const cVector2f& avSize,
		const cVector2f& avMinTexCoord, const cVector2f& avMaxTexCoord,
		const cColor& aColor)
	{
		// TODO: Implement textured quad drawing
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawQuad(const cVector3f& avPos, const cVector2f& avSize,
		const cVector2f& avMinTexCoord0, const cVector2f& avMaxTexCoord0,
		const cVector2f& avMinTexCoord1, const cVector2f& avMaxTexCoord1,
		const cColor& aColor)
	{
		// TODO: Implement multi-textured quad drawing
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawQuad(const tVertexVec& avVtx)
	{
		// TODO: Implement quad drawing from vertex array
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawQuad(const tVertexVec& avVtx, const cColor aCol)
	{
		// TODO: Implement quad drawing with color override
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawQuad(const tVertexVec& avVtx, const float afZ)
	{
		// TODO: Implement quad drawing with Z override
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawQuad(const tVertexVec& avVtx, const float afZ, const cColor& aCol)
	{
		// TODO: Implement quad drawing with Z and color override
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawQuadMultiTex(const tVertexVec& avVtx, const tVector3fVec& avExtraUvs)
	{
		// TODO: Implement multi-textured quad drawing
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawLine(const cVector3f& avBegin, const cVector3f& avEnd, cColor aCol)
	{
		// TODO: Implement line drawing
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawLine(const cVector3f& avBegin, const cColor& aBeginCol, const cVector3f& avEnd, const cColor& aEndCol)
	{
		// TODO: Implement line drawing with vertex colors
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawBoxMinMax(const cVector3f& avMin, const cVector3f& avMax, cColor aCol)
	{
		// TODO: Implement box drawing (12 lines)
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawSphere(const cVector3f& avPos, float afRadius, cColor aCol)
	{
		// TODO: Implement sphere drawing (circles)
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawSphere(const cVector3f& avPos, float afRadius, cColor aColX, cColor aColY, cColor aColZ)
	{
		// TODO: Implement sphere drawing with axis colors
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawLineQuad(const cRect2f& aRect, float afZ, cColor aCol)
	{
		// TODO: Implement line quad drawing
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DrawLineQuad(const cVector3f& avPos, const cVector2f& avSize, cColor aCol)
	{
		// TODO: Implement line quad drawing
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// VERTEX BATCHING
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::AddVertexToBatch(const cVertex* apVtx)
	{
		// TODO: Implement vertex batching
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::AddVertexToBatch(const cVertex* apVtx, const cVector3f* avTransform)
	{
		// TODO: Implement vertex batching with transform
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::AddVertexToBatch(const cVertex* apVtx, const cMatrixf* aMtx)
	{
		// TODO: Implement vertex batching with matrix transform
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::AddVertexToBatch_Size2D(const cVertex* apVtx, const cVector3f* avTransform,
		const cColor* apCol, const float& mfW, const float& mfH)
	{
		// TODO: Implement 2D vertex batching
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::AddVertexToBatch_Raw(const cVector3f& avPos, const cColor& aColor,
		const cVector3f& avTex)
	{
		// TODO: Implement raw vertex batching
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::AddTexCoordToBatch(unsigned int alUnit, const cVector3f* apCoord)
	{
		// TODO: Implement texture coordinate batching
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetBatchTextureUnitActive(unsigned int alUnit, bool abActive)
	{
		if (alUnit < kMaxTextureUnits)
		{
			mbTexCoordArrayActive[alUnit] = abActive;
		}
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::AddIndexToBatch(int alIndex)
	{
		// TODO: Implement index batching
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::FlushTriBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear)
	{
		// TODO: Implement triangle batch flush
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::FlushQuadBatch(tVtxBatchFlag aTypeFlags, bool abAutoClear)
	{
		// TODO: Implement quad batch flush
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::ClearBatch()
	{
		mlVertexCount = 0;
		mlIndexCount = 0;

		for (int i = 0; i < kMaxTextureUnits; ++i)
		{
			mlTexCoordArrayCount[i] = 0;
		}
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// HELPER FUNCTIONS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::SetUpBatchArrays()
	{
		// Allocate vertex batching arrays
		mlBatchArraySize = 20000; // Default batch size

		mpVertexArray = hplNewArray(float, mlBatchArraySize * 9); // Position (3) + Color (4) + TexCoord (2)
		mpIndexArray = hplNewArray(unsigned int, mlBatchArraySize);

		for (int i = 0; i < kMaxTextureUnits; ++i)
		{
			mpTexCoordArray[i] = hplNewArray(float, mlBatchArraySize * 3); // 3D texture coordinates
		}

		mlBatchStride = 9; // 3 (pos) + 4 (color) + 2 (uv)

		ClearBatch();

		Log("Vertex batching arrays allocated (size=%d)\n", mlBatchArraySize);
	}

	//-----------------------------------------------------------------------

	void cLowLevelGraphicsGPU::DestroyBatchArrays()
	{
		if (mpVertexArray)
		{
			hplDeleteArray(mpVertexArray);
			mpVertexArray = nullptr;
		}

		if (mpIndexArray)
		{
			hplDeleteArray(mpIndexArray);
			mpIndexArray = nullptr;
		}

		for (int i = 0; i < kMaxTextureUnits; ++i)
		{
			if (mpTexCoordArray[i])
			{
				hplDeleteArray(mpTexCoordArray[i]);
				mpTexCoordArray[i] = nullptr;
			}
		}
	}

	//-----------------------------------------------------------------------

}; // namespace hpl
