#include "PlatformFactory.h"
#include "DesktopPlatform.h"

std::unique_ptr<PlatformInterface> PlatformFactory::create()
{
#if defined(Q_OS_ANDROID)
    // TODO: return std::make_unique<AndroidPlatform>();
    return std::make_unique<DesktopPlatform>();
#elif defined(Q_OS_IOS)
    // TODO: return std::make_unique<IOSPlatform>();
    return std::make_unique<DesktopPlatform>();
#else
    return std::make_unique<DesktopPlatform>();
#endif
}
