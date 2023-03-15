#pragma once

#include <set>
#include <string>
#include <array>

class SourceContainer
{
public:
    struct Source
    {
        Source()
        {
        }

        Source(const char* name, const char* url)
            : mSourceName(name)
            , mSourceUrl(url)
        {
        }

        std::string mSourceName;
        std::string mSourceUrl;

        bool operator==(const Source& rhs) const
        {
            return mSourceName == rhs.mSourceName;
        }

        bool operator!=(const Source& rhs) const
        {
            return !(*this == rhs);
        }

        bool operator>(const Source& rhs) const
        {
            return mSourceName > rhs.mSourceName;
        }

        bool operator<(const Source& rhs) const
        {
            return mSourceName < rhs.mSourceName;
        }

        bool urlChanged(const Source& rhs) const
        {
            return (*this == rhs && this->mSourceUrl != rhs.mSourceUrl);
        }

        bool isChanged(const Source& rhs) const
        {
            return (*this == rhs && urlChanged(rhs));
        }

        bool isSame(const Source& rhs) const
        {
            return (*this == rhs && !urlChanged(rhs));
        }
    };

    bool addSource(const Source& src)
    {
        bool isChanged = false;

        auto iterCur = getSources().find(src);
        if (iterCur == getSources().end())
        {
            // didn't exist before
            isChanged = true;
        }
        else if (iterCur->isSame(src))
        {
            // existed before
        }
        else
        {
            // existed, but changed
            isChanged = true;
        }
        getNextSources().emplace(src);

        return isChanged;
    }

    void startAdd()
    {
        mNextBank = mSourcesBank ^ 1;
        getNextSources().clear();
    }

    void commit()
    {
        mSourcesBank = mNextBank;
    }

    unsigned getSourceCount() const
    {
        return getSources().size();
    }

    Source getSource(unsigned idx) const
    {
        auto iter = getIter(idx);
        return iter == getSources().cend() ? Source{} : *iter;
    }

private:
    std::array <std::set<Source>, 2> mSources;
    unsigned mSourcesBank = 0;
    unsigned mNextBank = 1;
    std::set<Source>& getSources()
    {
        return mSources[mSourcesBank];
    }
    const std::set<Source>& getSources() const
    {
        return mSources[mSourcesBank];
    }
    std::set<Source>& getNextSources()
    {
        return mSources[mNextBank];
    }

    std::set<SourceContainer::Source>::iterator getIter(unsigned idx) const
    {
        if (idx < getSourceCount())
        {
            return std::next(getSources().cbegin(), idx);
        }
        return getSources().cend();
    }
};
