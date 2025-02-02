#pragma once

#include <Foundation/Basics.h>

#if EZ_ENABLED(EZ_PLATFORM_WINDOWS_DESKTOP)

#  include <Foundation/Application/Implementation/Win/ApplicationEntryPoint_win.h>

#elif EZ_ENABLED(EZ_PLATFORM_WINDOWS_UWP)

#  include <Foundation/Application/Implementation/uwp/ApplicationEntryPoint_uwp.h>

#elif EZ_ENABLED(EZ_PLATFORM_OSX) || EZ_ENABLED(EZ_PLATFORM_LINUX)

#  include <Foundation/Application/Implementation/Posix/ApplicationEntryPoint_posix.h>

#elif EZ_ENABLED(EZ_PLATFORM_ANDROID)

#  include <Foundation/Application/Implementation/Android/ApplicationEntryPoint_android.h>

#elif EZ_ENABLED(EZ_PLATFORM_WEB)

#  include <Foundation/Application/Implementation/Web/ApplicationEntryPoint_web.h>

#else
#  error "Missing definition of platform specific entry point!"
#endif
