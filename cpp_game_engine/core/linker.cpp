/*
* This file exists for the purpose of specifying libraries for the linker and for
* compiling header-only libraries such as STB.
* 
* CURRENTLY ONLY MSVC IS SUPPORTED.
*/

#include <cstdint>


// Hint to NVIDIA and AMD GPUs that we prefer a high-performance GPU, if multiple
// GPUs are available.
extern "C" {
	__declspec(dllexport) uint32_t NvOptimusEnablement = 1;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}


#ifdef _DEBUG
#pragma comment(lib, "assimp-vc142-mtd.lib")
#pragma comment(lib, "zlibstaticd.lib")
#else
#pragma comment(lib, "assimp-vc142-mt.lib")
#pragma comment(lib, "zlibstatic.lib")
#endif

#pragma comment(lib, "glfw3_mt.lib")

#pragma comment(lib, "glew32s.lib")

#pragma comment(lib, "OpenGL32.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"