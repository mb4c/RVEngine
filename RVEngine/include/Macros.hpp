#pragma once

#include <string_view>

constexpr const char* rv_pretty_function(const char* s)
{
	std::string_view prettyFunction(s);
	size_t bracket = prettyFunction.rfind("(");
	size_t space = prettyFunction.rfind(" ", bracket) + 1;
	return prettyFunction.substr(space, bracket-space).data();
}

#define RV_PRETTY_FUNCTION rv_pretty_function(__PRETTY_FUNCTION__)

// #define RV_ENABLE_PROFILER

#ifdef RV_ENABLE_PROFILER
	#include <glad/gl.h>
	#include <tracy/Tracy.hpp>
	#include <tracy/TracyOpenGL.hpp>
	#define RV_PROFILE_SCOPE(name) ZoneScopedN(name)
	#define RV_PROFILE_FUNCTION() RV_PROFILE_SCOPE(RV_PRETTY_FUNCTION)
	#define RV_PROFILE_FRAME() FrameMark;
	#define RV_PROFILE_GPU_CONTEXT() TracyGpuContext;
	#define RV_PROFILE_GPU_COLLECT() TracyGpuCollect;
	#define RV_PROFILE_GPU_ZONE(name) TracyGpuZone(name);
#else
	#define RV_PROFILE_SCOPE(name)
	#define RV_PROFILE_FUNCTION()
	#define RV_PROFILE_FRAME()
	#define RV_PROFILE_GPU_CONTEXT()
	#define RV_PROFILE_GPU_COLLECT()
	#define RV_PROFILE_GPU_ZONE(name)
#endif