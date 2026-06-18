#ifndef SINEN_PROFILER_HPP
#define SINEN_PROFILER_HPP

#ifdef SINEN_MODULE_PROFILER
#include <tracy/Tracy.hpp>
#else
#define ZoneScoped
#define ZoneScopedN(x)
#define FrameMark
#define FrameMarkNamed(x)
#endif

#endif
