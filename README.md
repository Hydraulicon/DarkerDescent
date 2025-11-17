# Darker Descent
64-bit Windows port of Amnesia: The Dark Descent, based on "Amnesia64" from buzer2020.

Currently pre-alpha; stable, as per my independent testing.

## Key changes:
	- 64bit solution for VS2022 and C++14 included; builds all projects at once.
	- Static linkage for all external dependencies.
	- SDL2 2.0.12 was upgraded to SDL3 3.2.26.
	- Gamma correction implemented via post-processing shader to replace SDL3's removed hardware gamma functionality.
	- OpenAL Soft 1.24.3 integrated as static library with SDL3 audio backend.
	- alut was replaced with freealut.
	- Newton Dynamics was upgraded from 2.08 to 2.32.
	- FBX support not available at this time.

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
