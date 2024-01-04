#pragma once

#include "source-container.hpp"
#include "interfaces/input-observer.hpp"
#include "common/custom_thread.hpp"
#include <stddef.h>

//#include <Processing.NDI.Lib.h>
#include <Processing.NDI.Advanced.h>


#include <cstddef>
#include <stdint.h>
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
    CustomThread mShadowsourceTrackThread;

    void updateObserversAboutInputState(std::vector<std::string> sources);
    std::set<InputObserver*> mInputSinkObservers;
    std::mutex mSourceMutex;
};

