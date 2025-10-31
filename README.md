# Darker Descent
64-bit Windows port of Amnesia: The Dark Descent, based on "Amnesia64" from buzer2020.

Currently pre-alpha; stable, as per my independent testing, but currently lacking gamma control.
In-game gamma slider does not function due to breaking changes in SDL3, and will require a gamma post-process, which I plan to integrate.
Default game brightness without gamma correction may potentially be too dark to produce legible image in darker areas.

## Key changes:
	- 64bit solution for VS2022 and C++14 included; builds all projects at once.
	- Static linkage for most external dependencies, excluding OpenAL Soft, which requires a DLL to launch.
	- SDL2 2.0.12 was upgraded to SDL3 3.2.26.
	- alut was replaced with freealut.
	- Newton Dynamics was upgraded from 2.08 to 2.32.
	- FBX support not available at this time.
