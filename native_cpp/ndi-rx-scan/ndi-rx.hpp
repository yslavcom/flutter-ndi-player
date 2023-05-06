#pragma once

#include <cstddef>
#include <stdint.h>

//#include </home/iaroslav/work/streaming/NDI/5.5.3/NDI-Advanced-SDK-for-Linux/include/Processing.NDI.Advanced.h>
#include <Processing.NDI.Lib.h>

#include "source-container.hpp"
#include "interfaces/input-observer.hpp"

#include <thread>
#include <mutex>

class NdiRx
{
public:
    NdiRx();
    ~NdiRx();

    bool start();
    bool scanNdiSources();

    std::string getSourceName(unsigned idx) const
    {
        return mSourceContainer.getSource(idx).mSourceName;
    }
    std::string getSourceUrl(unsigned idx) const
    {
        return mSourceContainer.getSource(idx).mSourceUrl;
    }
    const SourceContainer& getSourceContainer() const
    {
        return mSourceContainer;
    }

    void addObserver(InputObserver* obs);
    void removeObserver(InputObserver* obs);

private:
    NDIlib_find_instance_t pNDI_find;

    unsigned mSourceCount;
    SourceContainer mSourceContainer;
    unsigned trackNdiSourcesBackgroundBlock(bool& risChanged); // a blocking function
    std::thread mShadowsourceTrackThread;

    void updateObserversAboutInputState(std::vector<std::string> sources);
    std::set<InputObserver*> mInputSinkObservers;
    std::mutex mSourceMutex;

#if 0
#ifdef ANDROID_OUT
    NsdManager m_nsdManager;
#endif
#endif
};

