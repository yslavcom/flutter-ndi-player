#include "interfaces/input-observer.hpp"

#include <iostream>
#include <functional>

class NdiSrcObserver: public InputObserver
{
    using UiUpdateCb = std::function<int32_t(int32_t)>;

public:
    void updateInputState(std::vector<std::string> inputNames)
    {
        mInputNames = inputNames;
        std::cout << __func__ << std::endl;
        for (auto& el : inputNames)
        {
            std::cout << el << std::endl;
        }
        if (mUiUpdateCb)
        {
            mUiUpdateCb(mInputNames.size());
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