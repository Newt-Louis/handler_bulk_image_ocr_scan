#pragma once

#include <memory>

class PlatformInterface;

class PlatformFactory
{
public:
    static std::unique_ptr<PlatformInterface> create();
};
