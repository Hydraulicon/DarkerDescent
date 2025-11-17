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

#ifndef HPL_OPENGL_EXTENSIONS_H
#define HPL_OPENGL_EXTENSIONS_H

//---------------------------------------------

#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>

//---------------------------------------------

// Extension availability flags
namespace hpl {

	struct OpenGLExtensions
	{
		// ARB Extensions
		bool ARB_vertex_buffer_object;
		bool ARB_multisample;
		bool ARB_multitexture;
		bool ARB_texture_compression;
		bool ARB_texture_float;
		bool ARB_fragment_program;
		bool ARB_fragment_shader;
		bool ARB_occlusion_query;

		// EXT Extensions
		bool EXT_stencil_two_side;
		bool EXT_texture_filter_anisotropic;
		bool EXT_texture_compression_s3tc;
		bool EXT_framebuffer_object;
		bool EXT_packed_depth_stencil;
		bool EXT_gpu_shader4;
		bool EXT_blend_func_separate;
		bool EXT_swap_control;
		bool EXT_secondary_color;

		// Vendor Extensions
		bool ATI_separate_stencil;
		bool ATI_shader_texture_lod;
		bool ATI_fragment_shader;
		bool NV_vertex_program3;
		bool SGIS_generate_mipmap;
	};

	// Global extension availability
	extern OpenGLExtensions g_OpenGLExtensions;

	//---------------------------------------------

	// ARB_vertex_buffer_object
	typedef void (APIENTRYP PFNGLGENBUFFERSARBPROC)(GLsizei n, GLuint *buffers);
	typedef void (APIENTRYP PFNGLDELETEBUFFERSARBPROC)(GLsizei n, const GLuint *buffers);
	typedef void (APIENTRYP PFNGLBINDBUFFERARBPROC)(GLenum target, GLuint buffer);
	typedef void (APIENTRYP PFNGLBUFFERDATAARBPROC)(GLenum target, GLsizeiptrARB size, const void *data, GLenum usage);
	typedef void (APIENTRYP PFNGLBUFFERSUBDATAARBPROC)(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const void *data);

	extern PFNGLGENBUFFERSARBPROC glGenBuffersARB;
	extern PFNGLDELETEBUFFERSARBPROC glDeleteBuffersARB;
	extern PFNGLBINDBUFFERARBPROC glBindBufferARB;
	extern PFNGLBUFFERDATAARBPROC glBufferDataARB;
	extern PFNGLBUFFERSUBDATAARBPROC glBufferSubDataARB;

	//---------------------------------------------

	// ARB_multitexture
	typedef void (APIENTRYP PFNGLACTIVETEXTUREARBPROC)(GLenum texture);
	typedef void (APIENTRYP PFNGLCLIENTACTIVETEXTUREARBPROC)(GLenum texture);
	typedef void (APIENTRYP PFNGLMULTITEXCOORD2FARBPROC)(GLenum target, GLfloat s, GLfloat t);
	typedef void (APIENTRYP PFNGLMULTITEXCOORD3FARBPROC)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
	typedef void (APIENTRYP PFNGLMULTITEXCOORD4FARBPROC)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);

	extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
	extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
	extern PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
	extern PFNGLMULTITEXCOORD3FARBPROC glMultiTexCoord3fARB;
	extern PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARB;

	//---------------------------------------------

	// ARB_occlusion_query
	typedef void (APIENTRYP PFNGLGENQUERIESARBPROC)(GLsizei n, GLuint *ids);
	typedef void (APIENTRYP PFNGLDELETEQUERIESARBPROC)(GLsizei n, const GLuint *ids);
	typedef void (APIENTRYP PFNGLBEGINQUERYARBPROC)(GLenum target, GLuint id);
	typedef void (APIENTRYP PFNGLENDQUERYARBPROC)(GLenum target);
	typedef void (APIENTRYP PFNGLGETQUERYOBJECTIVARBPROC)(GLuint id, GLenum pname, GLint *params);
	typedef void (APIENTRYP PFNGLGETQUERYOBJECTUIVARBPROC)(GLuint id, GLenum pname, GLuint *params);

	extern PFNGLGENQUERIESARBPROC glGenQueriesARB;
	extern PFNGLDELETEQUERIESARBPROC glDeleteQueriesARB;
	extern PFNGLBEGINQUERYARBPROC glBeginQueryARB;
	extern PFNGLENDQUERYARBPROC glEndQueryARB;
	extern PFNGLGETQUERYOBJECTIVARBPROC glGetQueryObjectivARB;
	extern PFNGLGETQUERYOBJECTUIVARBPROC glGetQueryObjectuivARB;

	//---------------------------------------------

	// EXT_framebuffer_object
	typedef void (APIENTRYP PFNGLGENRENDERBUFFERSEXTPROC)(GLsizei n, GLuint *renderbuffers);
	typedef void (APIENTRYP PFNGLDELETERENDERBUFFERSEXTPROC)(GLsizei n, const GLuint *renderbuffers);
	typedef void (APIENTRYP PFNGLBINDRENDERBUFFEREXTPROC)(GLenum target, GLuint renderbuffer);
	typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEEXTPROC)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
	typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSEXTPROC)(GLsizei n, GLuint *framebuffers);
	typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSEXTPROC)(GLsizei n, const GLuint *framebuffers);
	typedef void (APIENTRYP PFNGLBINDFRAMEBUFFEREXTPROC)(GLenum target, GLuint framebuffer);
	typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
	typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
	typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)(GLenum target);

	extern PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
	extern PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
	extern PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
	extern PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
	extern PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
	extern PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
	extern PFNGLBINDFRAMEBUFFEREXTPROC glBindFramebufferEXT;
	extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
	extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
	extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;

	//---------------------------------------------

	// EXT_stencil_two_side
	typedef void (APIENTRYP PFNGLACTIVESTENCILFACEEXTPROC)(GLenum face);

	extern PFNGLACTIVESTENCILFACEEXTPROC glActiveStencilFaceEXT;

	//---------------------------------------------

	// EXT_blend_func_separate
	typedef void (APIENTRYP PFNGLBLENDFUNCSEPARATEEXTPROC)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);

	extern PFNGLBLENDFUNCSEPARATEEXTPROC glBlendFuncSeparateEXT;

	//---------------------------------------------

	// EXT_secondary_color
	typedef void (APIENTRYP PFNGLSECONDARYCOLORPOINTEREXTPROC)(GLint size, GLenum type, GLsizei stride, const void *pointer);

	extern PFNGLSECONDARYCOLORPOINTEREXTPROC glSecondaryColorPointerEXT;

	//---------------------------------------------

	#ifdef _WIN32
	// WGL types
	DECLARE_HANDLE(HPBUFFERARB);

	// WGL constants
	#ifndef WGL_DRAW_TO_PBUFFER_ARB
	#define WGL_DRAW_TO_PBUFFER_ARB 0x202D
	#define WGL_BIND_TO_TEXTURE_RGBA_ARB 0x2071
	#define WGL_DEPTH_BITS_ARB 0x2022
	#define WGL_STENCIL_BITS_ARB 0x2023
	#define WGL_COLOR_BITS_ARB 0x2014
	#define WGL_PBUFFER_LARGEST_ARB 0x2033
	#define WGL_TEXTURE_FORMAT_ARB 0x2072
	#define WGL_TEXTURE_RGBA_ARB 0x2076
	#define WGL_TEXTURE_TARGET_ARB 0x2073
	#define WGL_TEXTURE_2D_ARB 0x207A
	#define WGL_MIPMAP_TEXTURE_ARB 0x2074
	#define WGL_PBUFFER_WIDTH_ARB 0x2034
	#define WGL_PBUFFER_HEIGHT_ARB 0x2035
	#define WGL_FRONT_LEFT_ARB 0x2083
	#endif

	// WGL_EXT_swap_control
	typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int interval);
	typedef int (WINAPI * PFNWGLGETSWAPINTERVALEXTPROC)(void);

	extern PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
	extern PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;

	// WGL_ARB_pbuffer
	typedef HPBUFFERARB (WINAPI * PFNWGLCREATEPBUFFERARBPROC)(HDC hDC, int iPixelFormat, int iWidth, int iHeight, const int *piAttribList);
	typedef HDC (WINAPI * PFNWGLGETPBUFFERDCARBPROC)(HPBUFFERARB hPbuffer);
	typedef int (WINAPI * PFNWGLRELEASEPBUFFERDCARBPROC)(HPBUFFERARB hPbuffer, HDC hDC);
	typedef BOOL (WINAPI * PFNWGLDESTROYPBUFFERARBPROC)(HPBUFFERARB hPbuffer);
	typedef BOOL (WINAPI * PFNWGLQUERYPBUFFERARBPROC)(HPBUFFERARB hPbuffer, int iAttribute, int *piValue);
	typedef BOOL (WINAPI * PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc, const int *piAttribIList, const FLOAT *pfAttribFList, UINT nMaxFormats, int *piFormats, UINT *nNumFormats);
	typedef BOOL (WINAPI * PFNWGLBINDTEXIMAGEARBPROC)(HPBUFFERARB hPbuffer, int iBuffer);
	typedef BOOL (WINAPI * PFNWGLRELEASETEXIMAGEARBPROC)(HPBUFFERARB hPbuffer, int iBuffer);

	extern PFNWGLCREATEPBUFFERARBPROC wglCreatePbufferARB;
	extern PFNWGLGETPBUFFERDCARBPROC wglGetPbufferDCARB;
	extern PFNWGLRELEASEPBUFFERDCARBPROC wglReleasePbufferDCARB;
	extern PFNWGLDESTROYPBUFFERARBPROC wglDestroyPbufferARB;
	extern PFNWGLQUERYPBUFFERARBPROC wglQueryPbufferARB;
	extern PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
	extern PFNWGLBINDTEXIMAGEARBPROC wglBindTexImageARB;
	extern PFNWGLRELEASETEXIMAGEARBPROC wglReleaseTexImageARB;
	#endif

	//---------------------------------------------

	// OpenGL 2.0+ Core Functions (GLSL, FBO, etc.)
	// These are core in OpenGL 2.0+ but may need manual loading on Windows

	// Shader functions
	extern PFNGLCREATEPROGRAMPROC glCreateProgram;
	extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
	extern PFNGLUSEPROGRAMPROC glUseProgram;
	extern PFNGLATTACHSHADERPROC glAttachShader;
	extern PFNGLDETACHSHADERPROC glDetachShader;
	extern PFNGLLINKPROGRAMPROC glLinkProgram;
	extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
	extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
	extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
	extern PFNGLUNIFORM1IPROC glUniform1i;
	extern PFNGLUNIFORM1FPROC glUniform1f;
	extern PFNGLUNIFORM2FPROC glUniform2f;
	extern PFNGLUNIFORM3FPROC glUniform3f;
	extern PFNGLUNIFORM4FPROC glUniform4f;
	extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
	extern PFNGLCREATESHADERPROC glCreateShader;
	extern PFNGLDELETESHADERPROC glDeleteShader;
	extern PFNGLSHADERSOURCEPROC glShaderSource;
	extern PFNGLCOMPILESHADERPROC glCompileShader;
	extern PFNGLGETSHADERIVPROC glGetShaderiv;
	extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;

	// Framebuffer functions
	extern PFNGLDRAWBUFFERSPROC glDrawBuffers;
	extern PFNGLISQUERYPROC glIsQuery;

	// 3D texture functions (OpenGL 1.2+)
	extern PFNGLTEXIMAGE3DPROC glTexImage3D;
	extern PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D;

	// Texture compression functions
	extern PFNGLCOMPRESSEDTEXIMAGE1DARBPROC glCompressedTexImage1DARB;
	extern PFNGLCOMPRESSEDTEXIMAGE2DARBPROC glCompressedTexImage2DARB;
	extern PFNGLCOMPRESSEDTEXIMAGE3DARBPROC glCompressedTexImage3DARB;

	// Additional FBO functions
	typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
	typedef void (APIENTRYP PFNGLGENERATEMIPMAPEXTPROC)(GLenum target);

	extern PFNGLFRAMEBUFFERTEXTURE3DEXTPROC glFramebufferTexture3DEXT;
	extern PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT;

	// ATI separate stencil (vendor extension)
	typedef void (APIENTRYP PFNGLSTENCILOPSEPARATEATIPROC)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
	typedef void (APIENTRYP PFNGLSTENCILFUNCSEPARATEATIPROC)(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);

	extern PFNGLSTENCILOPSEPARATEATIPROC glStencilOpSeparateATI;
	extern PFNGLSTENCILFUNCSEPARATEATIPROC glStencilFuncSeparateATI;

	//---------------------------------------------

	/**
	 * Initialize OpenGL extensions using SDL3's extension loading
	 * Must be called after OpenGL context creation
	 * @return true if all required extensions are available
	 */
	bool InitOpenGLExtensions();

	/**
	 * Check if a specific extension is supported
	 * @param extension Extension name (e.g., "GL_ARB_vertex_buffer_object")
	 * @return true if supported
	 */
	bool IsExtensionSupported(const char* extension);

} // namespace hpl

//---------------------------------------------

#endif // HPL_OPENGL_EXTENSIONS_H
