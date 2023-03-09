#pragma once

#include <vector>
#include <string>

class InputObserver
{
public:
    virtual void updateInputState(std::vector<std::string> inputNames) = 0;
};