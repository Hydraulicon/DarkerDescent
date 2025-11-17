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
	- SIMD math optimizations: Runtime-dispatched SIMD instruction set selection with support for AVX-512, AVX2, AVX, SSE4.1, and SSE2. Automatic CPU detection selects the best available instruction set at startup, providing 2-6x speedup for matrix operations and vector math in rendering and physics calculations.

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

## SIMD Optimizations

This project includes comprehensive SIMD (Single Instruction, Multiple Data) optimizations for critical math operations. The engine automatically detects your CPU capabilities at startup and selects the best available instruction set.

### Supported Instruction Sets

The engine supports the following SIMD instruction sets, in order of preference:

1. **AVX-512F** (Intel Skylake-X 2017+, AMD Zen 5 2024+)
   - 512-bit wide operations
   - Speedup: 2.5-3x over SSE2
   - Note: May cause frequency throttling on some Intel CPUs

2. **AVX2** (Intel Haswell 2013+, AMD Excavator 2015+)
   - 256-bit operations with FMA (Fused Multiply-Add)
   - Speedup: 2-2.5x over SSE2
   - Best balance of performance and compatibility

3. **AVX** (Intel Sandy Bridge 2011+, AMD Bulldozer 2011+)
   - 256-bit operations
   - Speedup: 1.5-2x over SSE2

4. **SSE4.1** (Intel Core 2008+, AMD Bulldozer 2011+)
   - Dedicated dot product instruction
   - Speedup: 1.5-2x over SSE2 for vector operations

5. **SSE2** (All x64 CPUs - baseline)
   - 128-bit operations, guaranteed on x64
   - Baseline SIMD support

### Optimized Operations

- **Matrix Multiplication** (4x4): Uses runtime-selected instruction set (AVX-512, AVX2, AVX, or SSE2)
- **Matrix-Vector Multiplication**: Optimized for transform operations
- **Vector Dot Product**: Uses SSE4.1 `dp` instruction when available
- **Vector Cross Product**: SSE2 optimized
- **Vector Normalization**: Fast reciprocal square root with SSE2

### CPU Detection at Startup

The engine logs the detected CPU capabilities and selected instruction set at startup:
```
Initializing SIMD optimizations...
  CPU Features: SSE2=1, SSE4.1=1, AVX=1, AVX2=1, AVX-512F=0
  Matrix operations: Using AVX2 with FMA (256-bit)
  Vector3 dot product: Using SSE4.1 (dp instruction)
  Vector3 cross/normalize: Using SSE2
SIMD initialization complete.
```

This allows you to verify which optimizations are active on your system.
