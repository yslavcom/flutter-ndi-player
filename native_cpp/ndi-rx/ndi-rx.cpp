#include "ndi-rx.hpp"

#include <iostream>
#include <vector>

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
    if (!NDIlib_initialize())
    {
        return false;
    }

    pNDI_find = NDIlib_find_create_v2();
    if (!pNDI_find)
    {
        return false;
    }
    return true;
}

unsigned NdiRx::trackNdiSourcesBackgroundBlock(bool& risChanged) // a blocking function
{
    auto deleteSources = [this](){
        // effectively delete ndi sources
        mSourceContainer.startAdd();
        mSourceContainer.commit();
    };

    if (!pNDI_find)
    {
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
        auto ndiSources = NDIlib_find_get_current_sources(pNDI_find, &sourcesCount);

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
    }
    return mSourceContainer.getSourceCount();
}

void NdiRx::updateObserversAboutInputState()
{
    std::vector<std::string> sources;
    auto count = mSourceContainer.getSourceCount();
    for (unsigned i = 0; i < count; i ++)
    {
        auto source = mSourceContainer.getSource(i);
        sources.push_back({source.mSourceName});
    }
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
    mShadowsourceTrackThread = std::thread([this]()
    {
        if (!start())
        {
            return false;
        }
        for (;;)
        {
            bool isContentsChanged = false; // must be preset to false
            auto count = trackNdiSourcesBackgroundBlock(isContentsChanged);
            bool countChanged = (mSourceCount != count);
            if (countChanged || isContentsChanged)
            {
                std::cout << "NDI Source Count changed:" << count << std::endl;
                updateObserversAboutInputState();
                mSourceCount = count;
            }
        }
        return true;
    });

    return true;
}