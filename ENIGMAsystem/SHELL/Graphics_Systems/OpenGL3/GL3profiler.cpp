/** Copyright (C) 2008-2013 Josh Ventura, Robert B. Colton
***
*** This file is a part of the ENIGMA Development Environment.
***
*** ENIGMA is free software: you can redistribute it and/or modify it under the
*** terms of the GNU General Public License as published by the Free Software
*** Foundation, version 3 of the license or any later version.
***
*** This application and its source code is distributed AS-IS, WITHOUT ANY
*** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
*** FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
*** details.
***
*** You should have received a copy of the GNU General Public License along
*** with this code. If not, see <http://www.gnu.org/licenses/>
**/

#include "GL3profiler.h"

// -- NVIDIA Spec query defines
#define GL_GPU_MEM_INFO_DEDICATED_VIDMEM_NVX		0x9047
#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX		0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_VMEM_NVX  0x9049
#define GL_GPU_MEM_INFO_EVICTION_COUNT_NVX			0x904A
#define GL_GPU_MEM_INFO_EVICTED_MEM_NVX				0x904B


size_t query_graphics_total_vmem()
{
	GLint total_mem_kb = 64 * 1024;

	// -- NVIDIA
	if ( glewIsSupported("GL_NVX_gpu_memory_info") )
	{
		glGetIntegerv( GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, &total_mem_kb);
		return total_mem_kb;
	}

	// -- AMD
	#if defined( _LINUX )
	if ( glxewIsSupported("GLX_AMD_GPU_association") )
	{
		uint32_t n = glXGetGPUIDsAMD(0, 0);
		uint32_t *ids = new uint32_t[n];
		size_t total_mem_mb = 0;

		// -- Get the list of GPU's available
		glXGetGPUIDsAMD(n, ids);

		// -- Sum the available memory
		for ( uint32_t i=0; i<n; i++ )
		{
			size_t memory = 0;
			glXGetGPUInfoAMD(ids[i], GLX_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof(size_t), &memory);
			total_mem_mb += memory;
		}

		delete [] ids;

	#elif defined( _WIN32 )
	if ( wglewIsSupported("WGL_AMD_GPU_association") )
	{
		uint32_t n = wglGetGPUIDsAMD(0, 0);
		uint32_t *ids = new uint32_t[n];
		size_t total_mem_mb = 0;

		// -- Get the list of GPU's available
		wglGetGPUIDsAMD(n, ids);

		// -- Sum the available memory
		for ( uint32_t i=0; i<n; i++ )
		{
			size_t memory = 0;
			wglGetGPUInfoAMD(ids[i], WGL_GPU_RAM_AMD, GL_UNSIGNED_INT, sizeof(size_t), &memory);
			total_mem_mb += memory;
		}

		delete [] ids;

	#else
	if ( false )
	{
		size_t total_mem_mb = 0;
	#endif
		total_mem_kb = total_mem_mb * 1024;
		return total_mem_kb;
	}

	// -- Intel / Mesa3D / Other
	return 64 * 1024; // Default to 64MiB for now
}
