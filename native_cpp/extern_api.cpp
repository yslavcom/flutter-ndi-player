#include "program_switch.hpp"
#include "DartApiDL/include/dart_api_dl.c"

#include "common/logger.hpp"

#include <memory>
#include <mutex>
#include <vector>

#include <iostream>

#define EXPORT extern "C" __attribute__((visibility("default"))) __attribute__((used))

namespace
{
int64_t DartApiMessagePort = -1;

struct CharFromSources
{
    CharFromSources() = default;
    CharFromSources(std::vector<std::string> sources)
    {
        mCharLen = 0;
        for (auto& el : sources)
        {
            if ((mCharLen + el.size() + END_STRING_LEN) > mSourcesChars.size())
            {
                mSourcesChars.resize(mCharLen + el.size() + END_STRING_LEN);
            }

            memcpy(&mSourcesChars[mCharLen], el.c_str(), el.size());
            mSourcesChars[mCharLen + el.size()] = '\r';
            mSourcesChars[mCharLen + el.size() + 1] = '\n';
            mCharLen += (el.size() + END_STRING_LEN);
        }
    }

    // The strings are copied in the single array and are seprated by 0.
    // Pre-allocate long enough array for storing the NDI source names.
    // We do not want to change it's location dynamically because the contents mightbe updated at any moment.
    std::vector<uint8_t> mSourcesChars;
    unsigned mCharLen;
    static constexpr unsigned END_STRING_LEN = 2;
};
CharFromSources mCharFromSources;

void sendMsgToFlutter(std::vector<std::string> sources)
{
    if (DartApiMessagePort == -1)
    {
        return;
    }
    mCharFromSources = CharFromSources{sources};

    Dart_CObject obj;
    obj.type = Dart_CObject_kTypedData;
    obj.value.as_typed_data.type = Dart_TypedData_kUint8;
    obj.value.as_typed_data.length = mCharFromSources.mCharLen;
    obj.value.as_typed_data.values = mCharFromSources.mSourcesChars.data();
    Dart_PostCObject_DL(DartApiMessagePort, &obj);
}

int32_t notifyUI_NdiSourceChange(std::vector<std::string> sources)
{
    LOGW("%s, count:%d\n", __func__, sources.size());
    sendMsgToFlutter(sources);
    return sources.size();
}

Monitor::ProgramSwitch mProgramSwitch(notifyUI_NdiSourceChange);

} // anonymous namespace

EXPORT
int64_t initializeApiDLData(void *data)
{
    LOGW("%s, %p\n", __func__, data);

    return Dart_InitializeApiDL(data);
}

EXPORT
void setDartApiMessagePort(int64_t port)
{
    LOGW("%s:%ld\n", __func__, port);
    DartApiMessagePort = port;
}

EXPORT
void stopProgram(int64_t progrIdx)
{
    (void)progrIdx;

    mProgramSwitch.stopProgram();
}

EXPORT
void startProgram(int64_t progrIdx)
{
    mProgramSwitch.startProgram(progrIdx);
}

EXPORT
int32_t scanNdiSources()
{
    return mProgramSwitch.scanNdiSources();
}

enum
{
    kVideoQueueType = 0,
    kAudioQueueType = 1,
};

EXPORT
int getOverflowCount(int type)
{
    if (type == kVideoQueueType)
    {
        return mProgramSwitch.getVideoOverflowCount();
    }
    else if (type == kAudioQueueType)
    {
        return mProgramSwitch.getAudioOverflowCount();
    }
    return 0;
}

EXPORT
int getRxQueueLen(int type)
{
    if (type == kVideoQueueType)
    {
        return mProgramSwitch.getVideoQueueLen();
    }
    else if (type == kAudioQueueType)
    {
        return mProgramSwitch.getAudioQueueLen();
    }
    return 0;
}

enum
{
    kGetVidAudFramesCount = 0,
    kGetSourceProgramInfo = 1,
};

EXPORT
void* getArray(int type, int* size)
{
    if (!size)
    {
        return nullptr;
    }
    else if (type == kGetVidAudFramesCount)
    {
        auto count = mProgramSwitch.getRxFrameCount();
        auto ptr = (int*)malloc(sizeof(int)*2);
        if (ptr)
        {
            ptr[0] = count.first;
            ptr[1] = count.second;
        }
        *size = 2;
        return (void*)ptr;
    }
    return nullptr;
}

// all words as int32_t
// args => [size, length of args message including 'type', count in words], [type], ...[etc]

EXPORT
void* getArrayArgs(void* args, int* size)
{
    if (!args || !size)
    {
        return nullptr;
    }

    int32_t argsSize = *((int32_t*)args);
    if (argsSize < 1)
    {
        return nullptr;
    }

    int32_t type = *((int32_t*)args+1);

    if (type == kGetVidAudFramesCount)
    {
        return getArray(kGetVidAudFramesCount, size);
    }
    else if (type == kGetSourceProgramInfo)
    {
        // Retrieve complete info on NDI Source
        if (argsSize == 2)
        {
            int32_t sourceIdx = *((int32_t*)args+2);
            (void)sourceIdx;
        }

    }
    return nullptr;
}

EXPORT
void freeArray(void* addr)
{
    if (addr)
    {
        free(addr);
    }
}