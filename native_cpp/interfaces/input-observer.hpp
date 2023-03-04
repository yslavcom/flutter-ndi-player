#pragma once

#include <string_view>

class InputObserver;
class InputObserverServer
{
public:
    virtual void registerObserver(InputObserver* obs) = 0;
    virtual void deregisterObserver(InputObserver* obs) = 0;
};
class InputObserver
{
public:
    virtual void updateInputState() = 0;
};