#include "ndi-rx.hpp"

#include "common/logger.hpp"

#include <iostream>
#include <vector>

#define _DBG_RX

#ifdef _DBG_RX
    #define DBG_RX LOGW
#else
    #define DBG_RX
#endif

NdiRx::NdiRx()
    : pNDI_find(nullptr)
    , mSourceCount(0)
{
}

NdiRx::~NdiRx()
{
    NDIlib_destroy();
}

bool NdiRx::start()
{
#if 0
#ifdef ANDROID_OUT
    m_nsdManager = (NsdManager)getSystemService(Context.NSD_SERVICE);
#endif
#endif

    if (!NDIlib_initialize())
    {
        LOGE("%s:Failed to NDIlib_initialize\n", __func__);
        return false;
    }

    pNDI_find = NDIlib_find_create_v2();
    if (!pNDI_find)
    {
        LOGE("%s:Failed to NDIlib_find_create_v2\n", __func__);
        return false;
    }

    LOGE("%s:OK, version:%s\n", __func__, NDIlib_version());

    return true;
}

unsigned NdiRx::trackNdiSourcesBackgroundBlock(bool& risChanged) // a blocking function
{
    auto deleteSources = [this](){
        // effectively delete ndi sources
        mSourceContainer.startAdd();
        mSourceContainer.commit();
    };

    unsigned count = 0;

    if (!pNDI_find)
    {
        std::lock_guard lk(mSourceMutex);
        if (mSourceContainer.getSourceCount())
        {
            risChanged = true;
            deleteSources();
        }
        return 0;
    }
    else
    {
        uint32_t sourcesCount = 0;
        NDIlib_find_wait_for_sources(pNDI_find, 1000/* One second */);

        {
            std::lock_guard lk(mSourceMutex);

            auto ndiSources = NDIlib_find_get_current_sources(pNDI_find, &sourcesCount);
            DBG_RX("%s:sources:%p, count:%d", __func__, ndiSources, sourcesCount);
            if (sourcesCount && ndiSources)
            {
                if (mSourceContainer.getSourceCount() != sourcesCount)
                {
                    risChanged = true;
                }

                mSourceContainer.startAdd();
                for (uint32_t count = 0; count < sourcesCount; count ++)
                {
                    auto ptr = ndiSources + count;
                    if (mSourceContainer.addSource({ptr->p_ndi_name, ptr->p_url_address}))
                    {
                        risChanged = true;
                    }
                }
                mSourceContainer.commit();
            }
            else
            {
                if (mSourceContainer.getSourceCount())
                {
                    risChanged = true;
                    deleteSources();
                }
            }

            count = mSourceContainer.getSourceCount();
        }
    }
    return count;
}

void NdiRx::updateObserversAboutInputState(std::vector<std::string> sources)
{
    for (auto &obs : mInputSinkObservers)
    {
        obs->updateInputState(sources);
    }
}

void NdiRx::addObserver(InputObserver* obs)
{
    if (obs)
    {
        auto iter = mInputSinkObservers.find(obs);
        if (iter == mInputSinkObservers.cend())
        {
            mInputSinkObservers.emplace(obs);
        }
    }
}

void NdiRx::removeObserver(InputObserver* obs)
{
    if (obs)
    {
        auto iter = mInputSinkObservers.find(obs);
        if (iter != mInputSinkObservers.cend())
        {
            mInputSinkObservers.erase(iter);
        }
    }
}

bool NdiRx::scanNdiSources()
{
    mShadowsourceTrackThread.start([this](bool stop){
        if (!start())
        {
            return false;
        }
        while(!stop)
        {
            bool isContentsChanged = false; // must be preset to false
            auto count = trackNdiSourcesBackgroundBlock(isContentsChanged);
            DBG_RX("scanNdiSources[%s]:%d\n", __func__, count);
            bool countChanged = (mSourceCount != count);
            if (countChanged || isContentsChanged)
            {
                std::vector<std::string> sources;
                {
                    std::lock_guard lk(mSourceMutex);
                    auto count = mSourceContainer.getSourceCount();
                    for (unsigned i = 0; i < count; i ++)
                    {
                        auto source = mSourceContainer.getSource(i);
                        sources.push_back({source.mSourceName});
                    }
                }
                mSourceCount = count;
                LOGW("NDI Source Count changed:%d\n", count);
                updateObserversAboutInputState(sources);

            }
            std::this_thread::yield();
        }
        return true;
    });

    return true;
}