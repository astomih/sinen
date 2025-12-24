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
#define SINEN_PLATFORM_DESKTOP
#elif __linux__
#define SINEN_PLATFORM_LINUX
#define SINEN_PLATFORM_DESKTOP
#elif __APPLE__
#define SINEN_PLATFORM_MACOS
#define SINEN_PLATFORM_DESKTOP
#elif __ANDROID__
#define SINEN_PLATFORM_ANDROID
#define SINEN_PLATFORM_MOBILE
#elif __EMSCRIPTEN__
#define SINEN_PLATFORM_EMSCRIPTEN
#define SINEN_PLATFORM_WEB
#endif

#endif // !SINEN_MACRO_HPP
