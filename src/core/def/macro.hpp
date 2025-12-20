#ifndef SINEN_MACRO_HPP
#define SINEN_MACRO_HPP

// Macro to define the build type
#ifdef _DEBUG
#define SINEN_BUILD_DEBUG
#elif NDEBUG
#define SINEN_BUILD_RELEASE
#endif

// Macro to define the platform
#ifdef _WIN32
#define SINEN_PLATFORM_WINDOWS
#elif __linux__
#define SINEN_PLATFORM_LINUX
#elif __APPLE__
#define SINEN_PLATFORM_MACOS
#elif __EMSCRIPTEN__
#define SINEN_PLATFORM_EMSCRIPTEN
#endif

#endif // !SINEN_MACRO_HPP
