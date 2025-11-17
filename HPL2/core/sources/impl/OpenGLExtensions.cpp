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

#include "impl/OpenGLExtensions.h"
#include <SDL3/SDL.h>
#include <stdio.h>

//---------------------------------------------

namespace hpl {

	// Global extension availability
	OpenGLExtensions g_OpenGLExtensions = {};

	//---------------------------------------------
	// ARB_vertex_buffer_object

	PFNGLGENBUFFERSARBPROC glGenBuffersARB = NULL;
	PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB = NULL;
	PFNGLBINDBUFFERARBPROC glBindBufferARB = NULL;
	PFNGLBUFFERDATAARBPROC glBufferDataARB = NULL;
	PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB = NULL;

	//---------------------------------------------
	// ARB_multitexture

	PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = NULL;
	PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB = NULL;
	PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB = NULL;
	PFNGLMULTITEXCOORD3FARBPROC glMultiTexCoord3fARB = NULL;
	PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARB = NULL;

	//---------------------------------------------
	// ARB_occlusion_query

	PFNGLGENQUERIESARBPROC glGenQueriesARB = NULL;
	PFNGLDELETEQUERIESARBPROC glDeleteQueriesARB = NULL;
	PFNGLBEGINQUERYARBPROC glBeginQueryARB = NULL;
	PFNGLENDQUERYARBPROC glEndQueryARB = NULL;
	PFNGLGETQUERYOBJECTIVARBPROC glGetQueryObjectivARB = NULL;
	PFNGLGETQUERYOBJECTUIVARBPROC glGetQueryObjectuivARB = NULL;

	//---------------------------------------------
	// EXT_framebuffer_object

	PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT = NULL;
	PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT = NULL;
	PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT = NULL;
	PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT = NULL;
	PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT = NULL;
	PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT = NULL;
	PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT = NULL;
	PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT = NULL;
	PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT = NULL;
	PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT = NULL;

	//---------------------------------------------
	// EXT_stencil_two_side

	PFNGLACTIVESTENCILFACEEXTPROC glActiveStencilFaceEXT = NULL;

	//---------------------------------------------
	// EXT_blend_func_separate

	PFNGLBLENDFUNCSEPARATEEXTPROC glBlendFuncSeparateEXT = NULL;

	//---------------------------------------------
	// EXT_secondary_color

	PFNGLSECONDARYCOLORPOINTEREXTPROC glSecondaryColorPointerEXT = NULL;

	//---------------------------------------------

	#ifdef _WIN32
	// WGL_EXT_swap_control
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;
	PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = NULL;

	// WGL_ARB_pbuffer
	PFNWGLCREATEPBUFFERARBPROC wglCreatePbufferARB = NULL;
	PFNWGLGETPBUFFERDCARBPROC wglGetPbufferDCARB = NULL;
	PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDCARB = NULL;
	PFNWGLDESTROYPBUFFERARBPROC wglDestroyPbufferARB = NULL;
	PFNWGLQUERYPBUFFERARBPROC wglQueryPbufferARB = NULL;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
	PFNWGLBINDTEXIMAGEARBPROC wglBindTexImageARB = NULL;
	PFNWGLRELEASETEXIMAGEARBPROC wglReleaseTexImageARB = NULL;
	#endif

	//---------------------------------------------

	// OpenGL 2.0+ Core Functions
	PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
	PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
	PFNGLUSEPROGRAMPROC glUseProgram = NULL;
	PFNGLATTACHSHADERPROC glAttachShader = NULL;
	PFNGLDETACHSHADERPROC glDetachShader = NULL;
	PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
	PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
	PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
	PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
	PFNGLUNIFORM1IPROC glUniform1i = NULL;
	PFNGLUNIFORM1FPROC glUniform1f = NULL;
	PFNGLUNIFORM2FPROC glUniform2f = NULL;
	PFNGLUNIFORM3FPROC glUniform3f = NULL;
	PFNGLUNIFORM4FPROC glUniform4f = NULL;
	PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;
	PFNGLCREATESHADERPROC glCreateShader = NULL;
	PFNGLDELETESHADERPROC glDeleteShader = NULL;
	PFNGLSHADERSOURCEPROC glShaderSource = NULL;
	PFNGLCOMPILESHADERPROC glCompileShader = NULL;
	PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
	PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;

	PFNGLDRAWBUFFERSPROC glDrawBuffers = NULL;
	PFNGLISQUERYPROC glIsQuery = NULL;

	PFNGLCOMPRESSEDTEXIMAGE1DARBPROC glCompressedTexImage1DARB = NULL;
	PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB = NULL;
	PFNGLCOMPRESSEDTEXIMAGE3DARBPROC glCompressedTexImage3DARB = NULL;

	PFNGLTEXIMAGE3DPROC glTexImage3D = NULL;
	PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D = NULL;

	PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT = NULL;
	PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT = NULL;

	PFNGLSTENCILOPSEPARATEATIPROC glStencilOpSeparateATI = NULL;
	PFNGLSTENCILFUNCSEPARATEATIPROC glStencilFuncSeparateATI = NULL;

	//---------------------------------------------

	bool InitOpenGLExtensions()
	{
		// Reset extension flags
		g_OpenGLExtensions = {};

		// Check ARB extensions
		g_OpenGLExtensions.ARB_vertex_buffer_object = SDL_GL_ExtensionSupported("GL_ARB_vertex_buffer_object");
		g_OpenGLExtensions.ARB_multisample = SDL_GL_ExtensionSupported("GL_ARB_multisample");
		g_OpenGLExtensions.ARB_multitexture = SDL_GL_ExtensionSupported("GL_ARB_multitexture");
		g_OpenGLExtensions.ARB_texture_compression = SDL_GL_ExtensionSupported("GL_ARB_texture_compression");
		g_OpenGLExtensions.ARB_texture_float = SDL_GL_ExtensionSupported("GL_ARB_texture_float");
		g_OpenGLExtensions.ARB_fragment_program = SDL_GL_ExtensionSupported("GL_ARB_fragment_program");
		g_OpenGLExtensions.ARB_fragment_shader = SDL_GL_ExtensionSupported("GL_ARB_fragment_shader");
		g_OpenGLExtensions.ARB_occlusion_query = SDL_GL_ExtensionSupported("GL_ARB_occlusion_query");

		// Check EXT extensions
		g_OpenGLExtensions.EXT_stencil_two_side = SDL_GL_ExtensionSupported("GL_EXT_stencil_two_side");
		g_OpenGLExtensions.EXT_texture_filter_anisotropic = SDL_GL_ExtensionSupported("GL_EXT_texture_filter_anisotropic");
		g_OpenGLExtensions.EXT_texture_compression_s3tc = SDL_GL_ExtensionSupported("GL_EXT_texture_compression_s3tc");
		g_OpenGLExtensions.EXT_framebuffer_object = SDL_GL_ExtensionSupported("GL_EXT_framebuffer_object");
		g_OpenGLExtensions.EXT_packed_depth_stencil = SDL_GL_ExtensionSupported("GL_EXT_packed_depth_stencil");
		g_OpenGLExtensions.EXT_gpu_shader4 = SDL_GL_ExtensionSupported("GL_EXT_gpu_shader4");
		g_OpenGLExtensions.EXT_blend_func_separate = SDL_GL_ExtensionSupported("GL_EXT_blend_func_separate");
		g_OpenGLExtensions.EXT_swap_control = SDL_GL_ExtensionSupported("GL_EXT_swap_control");
		g_OpenGLExtensions.EXT_secondary_color = SDL_GL_ExtensionSupported("GL_EXT_secondary_color");

		// Check vendor extensions
		g_OpenGLExtensions.ATI_separate_stencil = SDL_GL_ExtensionSupported("GL_ATI_separate_stencil");
		g_OpenGLExtensions.ATI_shader_texture_lod = SDL_GL_ExtensionSupported("GL_ATI_shader_texture_lod");
		g_OpenGLExtensions.ATI_fragment_shader = SDL_GL_ExtensionSupported("GL_ATI_fragment_shader");
	g_OpenGLExtensions.NV_vertex_program3 = SDL_GL_ExtensionSupported("GL_NV_vertex_program3");
		g_OpenGLExtensions.SGIS_generate_mipmap = SDL_GL_ExtensionSupported("GL_SGIS_generate_mipmap");

		// Load ARB_vertex_buffer_object functions
		if (g_OpenGLExtensions.ARB_vertex_buffer_object)
		{
			glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)SDL_GL_GetProcAddress("glGenBuffersARB");
			glDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC)SDL_GL_GetProcAddress("glDeleteBuffersARB");
			glBindBufferARB = (PFNGLBINDBUFFERARBPROC)SDL_GL_GetProcAddress("glBindBufferARB");
			glBufferDataARB = (PFNGLBUFFERDATAARBPROC)SDL_GL_GetProcAddress("glBufferDataARB");
			glBufferSubDataARB = (PFNGLBUFFERSUBDATAARBPROC)SDL_GL_GetProcAddress("glBufferSubDataARB");
		}

		// Load ARB_multitexture functions
		if (g_OpenGLExtensions.ARB_multitexture)
		{
			glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glActiveTextureARB");
			glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)SDL_GL_GetProcAddress("glClientActiveTextureARB");
			glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord2fARB");
			glMultiTexCoord3fARB = (PFNGLMULTITEXCOORD3FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord3fARB");
			glMultiTexCoord4fARB = (PFNGLMULTITEXCOORD4FARBPROC)SDL_GL_GetProcAddress("glMultiTexCoord4fARB");
		}

		// Load ARB_occlusion_query functions
		if (g_OpenGLExtensions.ARB_occlusion_query)
		{
			glGenQueriesARB = (PFNGLGENQUERIESARBPROC)SDL_GL_GetProcAddress("glGenQueriesARB");
			glDeleteQueriesARB = (PFNGLDELETEQUERIESARBPROC)SDL_GL_GetProcAddress("glDeleteQueriesARB");
			glBeginQueryARB = (PFNGLBEGINQUERYARBPROC)SDL_GL_GetProcAddress("glBeginQueryARB");
			glEndQueryARB = (PFNGLENDQUERYARBPROC)SDL_GL_GetProcAddress("glEndQueryARB");
			glGetQueryObjectivARB = (PFNGLGETQUERYOBJECTIVARBPROC)SDL_GL_GetProcAddress("glGetQueryObjectivARB");
			glGetQueryObjectuivARB = (PFNGLGETQUERYOBJECTUIVARBPROC)SDL_GL_GetProcAddress("glGetQueryObjectuivARB");
		}

		// Load EXT_framebuffer_object functions
		if (g_OpenGLExtensions.EXT_framebuffer_object)
		{
			glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenRenderbuffersEXT");
			glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteRenderbuffersEXT");
			glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindRenderbufferEXT");
			glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)SDL_GL_GetProcAddress("glRenderbufferStorageEXT");
			glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glGenFramebuffersEXT");
			glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)SDL_GL_GetProcAddress("glDeleteFramebuffersEXT");
			glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)SDL_GL_GetProcAddress("glBindFramebufferEXT");
			glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)SDL_GL_GetProcAddress("glFramebufferRenderbufferEXT");
			glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture2DEXT");
			glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)SDL_GL_GetProcAddress("glCheckFramebufferStatusEXT");
		}

		// Load EXT_stencil_two_side functions
		if (g_OpenGLExtensions.EXT_stencil_two_side)
		{
			glActiveStencilFaceEXT = (PFNGLACTIVESTENCILFACEEXTPROC)SDL_GL_GetProcAddress("glActiveStencilFaceEXT");
		}

		// Load EXT_blend_func_separate functions
		if (g_OpenGLExtensions.EXT_blend_func_separate)
		{
			glBlendFuncSeparateEXT = (PFNGLBLENDFUNCSEPARATEEXTPROC)SDL_GL_GetProcAddress("glBlendFuncSeparateEXT");
		}

		// Load EXT_secondary_color functions
		if (g_OpenGLExtensions.EXT_secondary_color)
		{
			glSecondaryColorPointerEXT = (PFNGLSECONDARYCOLORPOINTEREXTPROC)SDL_GL_GetProcAddress("glSecondaryColorPointerEXT");
		}

		#ifdef _WIN32
		// Load WGL_EXT_swap_control functions (Windows only)
		if (g_OpenGLExtensions.EXT_swap_control)
		{
			wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)SDL_GL_GetProcAddress("wglSwapIntervalEXT");
			wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)SDL_GL_GetProcAddress("wglGetSwapIntervalEXT");
		}

		// Load WGL_ARB_pbuffer functions (Windows only)
		wglCreatePbufferARB = (PFNWGLCREATEPBUFFERARBPROC)SDL_GL_GetProcAddress("wglCreatePbufferARB");
		wglGetPbufferDCARB = (PFNWGLGETPBUFFERDCARBPROC)SDL_GL_GetProcAddress("wglGetPbufferDCARB");
		wglReleasePbufferDCARB = (PFNWGLRELEASEPBUFFERDCARBPROC)SDL_GL_GetProcAddress("wglReleasePbufferDCARB");
		wglDestroyPbufferARB = (PFNWGLDESTROYPBUFFERARBPROC)SDL_GL_GetProcAddress("wglDestroyPbufferARB");
		wglQueryPbufferARB = (PFNWGLQUERYPBUFFERARBPROC)SDL_GL_GetProcAddress("wglQueryPbufferARB");
		wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)SDL_GL_GetProcAddress("wglChoosePixelFormatARB");
		wglBindTexImageARB = (PFNWGLBINDTEXIMAGEARBPROC)SDL_GL_GetProcAddress("wglBindTexImageARB");
		wglReleaseTexImageARB = (PFNWGLRELEASETEXIMAGEARBPROC)SDL_GL_GetProcAddress("wglReleaseTexImageARB");
		#endif

		// Load OpenGL 2.0+ core functions (GLSL shaders)
		glCreateProgram = (PFNGLCREATEPROGRAMPROC)SDL_GL_GetProcAddress("glCreateProgram");
		glDeleteProgram = (PFNGLDELETEPROGRAMPROC)SDL_GL_GetProcAddress("glDeleteProgram");
		glUseProgram = (PFNGLUSEPROGRAMPROC)SDL_GL_GetProcAddress("glUseProgram");
		glAttachShader = (PFNGLATTACHSHADERPROC)SDL_GL_GetProcAddress("glAttachShader");
		glDetachShader = (PFNGLDETACHSHADERPROC)SDL_GL_GetProcAddress("glDetachShader");
		glLinkProgram = (PFNGLLINKPROGRAMPROC)SDL_GL_GetProcAddress("glLinkProgram");
		glGetProgramiv = (PFNGLGETPROGRAMIVPROC)SDL_GL_GetProcAddress("glGetProgramiv");
		glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)SDL_GL_GetProcAddress("glGetProgramInfoLog");
		glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)SDL_GL_GetProcAddress("glGetUniformLocation");
		glUniform1i = (PFNGLUNIFORM1IPROC)SDL_GL_GetProcAddress("glUniform1i");
		glUniform1f = (PFNGLUNIFORM1FPROC)SDL_GL_GetProcAddress("glUniform1f");
		glUniform2f = (PFNGLUNIFORM2FPROC)SDL_GL_GetProcAddress("glUniform2f");
		glUniform3f = (PFNGLUNIFORM3FPROC)SDL_GL_GetProcAddress("glUniform3f");
		glUniform4f = (PFNGLUNIFORM4FPROC)SDL_GL_GetProcAddress("glUniform4f");
		glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)SDL_GL_GetProcAddress("glUniformMatrix4fv");
		glCreateShader = (PFNGLCREATESHADERPROC)SDL_GL_GetProcAddress("glCreateShader");
		glDeleteShader = (PFNGLDELETESHADERPROC)SDL_GL_GetProcAddress("glDeleteShader");
		glShaderSource = (PFNGLSHADERSOURCEPROC)SDL_GL_GetProcAddress("glShaderSource");
		glCompileShader = (PFNGLCOMPILESHADERPROC)SDL_GL_GetProcAddress("glCompileShader");
		glGetShaderiv = (PFNGLGETSHADERIVPROC)SDL_GL_GetProcAddress("glGetShaderiv");
		glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)SDL_GL_GetProcAddress("glGetShaderInfoLog");

		// Load framebuffer and query functions
		glDrawBuffers = (PFNGLDRAWBUFFERSPROC)SDL_GL_GetProcAddress("glDrawBuffers");
		glIsQuery = (PFNGLISQUERYPROC)SDL_GL_GetProcAddress("glIsQuery");

		// Load 3D texture functions (OpenGL 1.2+)
		glTexImage3D = (PFNGLTEXIMAGE3DPROC)SDL_GL_GetProcAddress("glTexImage3D");
		glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)SDL_GL_GetProcAddress("glTexSubImage3D");

		// Load texture compression functions
		if (g_OpenGLExtensions.ARB_texture_compression)
		{
			glCompressedTexImage1DARB = (PFNGLCOMPRESSEDTEXIMAGE1DARBPROC)SDL_GL_GetProcAddress("glCompressedTexImage1DARB");
			glCompressedTexImage2DARB = (PFNGLCOMPRESSEDTEXIMAGE2DARBPROC)SDL_GL_GetProcAddress("glCompressedTexImage2DARB");
			glCompressedTexImage3DARB = (PFNGLCOMPRESSEDTEXIMAGE3DARBPROC)SDL_GL_GetProcAddress("glCompressedTexImage3DARB");
		}

		// Load additional FBO functions
		if (g_OpenGLExtensions.EXT_framebuffer_object)
		{
			glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)SDL_GL_GetProcAddress("glFramebufferTexture3DEXT");
			glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)SDL_GL_GetProcAddress("glGenerateMipmapEXT");
		}

		// Load ATI separate stencil functions
		if (g_OpenGLExtensions.ATI_separate_stencil)
		{
			glStencilOpSeparateATI = (PFNGLSTENCILOPSEPARATEATIPROC)SDL_GL_GetProcAddress("glStencilOpSeparateATI");
			glStencilFuncSeparateATI = (PFNGLSTENCILFUNCSEPARATEATIPROC)SDL_GL_GetProcAddress("glStencilFuncSeparateATI");
		}

		printf("[OpenGL] Extension initialization complete\n");
		printf("[OpenGL] VBO support: %s\n", g_OpenGLExtensions.ARB_vertex_buffer_object ? "YES" : "NO");
		printf("[OpenGL] FBO support: %s\n", g_OpenGLExtensions.EXT_framebuffer_object ? "YES" : "NO");
		printf("[OpenGL] Multitexture support: %s\n", g_OpenGLExtensions.ARB_multitexture ? "YES" : "NO");

		// Check for critical extensions
		bool hasRequiredExtensions =
			g_OpenGLExtensions.ARB_vertex_buffer_object &&
			g_OpenGLExtensions.ARB_multitexture &&
			g_OpenGLExtensions.EXT_framebuffer_object;

		if (!hasRequiredExtensions)
		{
			printf("[OpenGL] ERROR: Missing required OpenGL extensions!\n");
			return false;
		}

		return true;
	}

	//---------------------------------------------

	bool IsExtensionSupported(const char* extension)
	{
		return SDL_GL_ExtensionSupported(extension);
	}

} // namespace hpl

//---------------------------------------------
