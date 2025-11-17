# Darker Descent
64-bit Windows port of Amnesia: The Dark Descent, based on "Amnesia64" from buzer2020.

Currently pre-alpha; stable, as per my independent testing.

## Key changes:
	- 64-bit only solution for VS2022 with C++23 and ISO C17 compliance; builds all projects at once.
	- Static linkage for all external dependencies.
	- SDL2 2.0.12 was upgraded to SDL3 3.2.26.
	- Gamma correction implemented via post-processing shader to replace SDL3's removed hardware gamma functionality.
	- OpenAL Soft 1.24.3 integrated as static library with SDL3 audio backend.
	- freealut dependency removed; replaced with custom lightweight WAV loader.
	- GLEW dependency removed; replaced with SDL3's native OpenGL extension loading.
	- DevIL, jpeg, and png dependencies removed; replaced with stb_image and tinyddsloader single-header libraries.
	- Newton Dynamics was upgraded from 2.08 to 2.32.
	- FBX support not available at this time.
	- 64-bit optimizations: Converted vertex/index counts, file I/O operations, and bitmap/texture memory allocation from fixed-width types (int/unsigned long) to platform-appropriate size_t for improved performance and large asset support (>2GB textures, >4GB files).
	- SIMD math optimizations: Implemented SSE2 intrinsics for critical math operations (matrix multiplication, vector dot/cross products, normalization) providing 2-6x speedup in rendering and scene graph calculations.

## Third-Party Libraries and Licenses

### OpenAL Soft
This project uses [OpenAL Soft](https://github.com/kcat/openal-soft) version 1.24.3, an open-source 3D audio library licensed under the **GNU Library General Public License v2 (LGPL v2)**.

**LGPL Compliance:**
- OpenAL Soft is statically linked into this project
- The complete source code for OpenAL Soft is included in `HPL2/dependencies/sources/openal-soft/`
- The LGPL license is available in `HPL2/dependencies/sources/openal-soft/COPYING`
- Users may replace the OpenAL Soft library by recompiling the project with their modified version
- OpenAL Soft copyright © Frictional Games contributors

For more information about OpenAL Soft, visit: https://github.com/kcat/openal-soft

### stb_image
This project uses [stb_image](https://github.com/nothings/stb) and [stb_image_write](https://github.com/nothings/stb), public domain single-header image loading/writing libraries.

**Public Domain License:**
- stb_image and stb_image_write are in the public domain (or MIT licensed, at your option)
- The complete header file is included in `HPL2/dependencies/include/stb_image.h`
- No attribution required, but credit is appreciated

For more information about stb libraries, visit: https://github.com/nothings/stb

### tinyddsloader
This project uses [tinyddsloader](https://github.com/benikabocha/tinyddsloader), a header-only DDS (DirectDraw Surface) texture loader licensed under the **MIT License**.

**MIT License Compliance:**
- tinyddsloader is used as a header-only library
- The complete header file is included in `HPL2/dependencies/include/tinyddsloader.h`
- Copyright (c) 2020 benikabocha

For more information about tinyddsloader, visit: https://github.com/benikabocha/tinyddsloader
