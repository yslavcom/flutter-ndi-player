#pragma once

#include "interfaces/input-observer.hpp"

#include "common/logger.hpp"

#include <iostream>
#include <functional>

class NdiSrcObserver: public InputObserver
{
    using UiUpdateCb = std::function<int32_t(std::vector<std::string>)>;

public:
    virtual void updateInputState(std::vector<std::string> inputNames) override
    {
        mInputNames = inputNames;
        LOGW("%s\n", __func__);
        for (auto& el : inputNames)
        {
            // debug only
            LOGW("%s\n", el.c_str());
        }
        if (mUiUpdateCb)
        {
            mUiUpdateCb(inputNames);
        }
    }

    void setup(UiUpdateCb cb)
    {
        mUiUpdateCb = cb;
    }

    private:
        UiUpdateCb mUiUpdateCb;
        std::vector<std::string> mInputNames;

};