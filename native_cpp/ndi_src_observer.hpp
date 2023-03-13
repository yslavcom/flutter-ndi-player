#include "interfaces/input-observer.hpp"

#include <iostream>
#include <functional>

class NdiSrcObserver: public InputObserver
{
    using UiUpdateCb = std::function<int32_t(std::vector<std::string>)>;

public:
    void updateInputState(std::vector<std::string> inputNames)
    {
        mInputNames = inputNames;
        std::cout << __func__ << std::endl;
        for (auto& el : inputNames)
        {
            // debug only
            std::cout << el << std::endl;
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