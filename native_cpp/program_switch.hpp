#pragma once

#include "ndi_src_observer.hpp"
#include "player/sps_pps_parser.hpp"

#include <cstdint>
#include <optional>
#include <mutex>
#include <functional>
#include <string>

namespace Monitor
{
using NdiSourceChangeNotify = NdiSrcObserver::UiUpdateCb;

class ProgramSwitch
{
public:
    ProgramSwitch(NdiSourceChangeNotify ndiSourceChangeNotify);

    void startProgram(int64_t progrIdx);
    void stopProgram();
    int32_t scanNdiSources();
    uint32_t getVideoOverflowCount() const;
    uint32_t getAudioOverflowCount() const;
    uint32_t getVideoQueueLen() const;
    uint32_t getAudioQueueLen() const;
    std::pair<unsigned, unsigned> getRxFrameCount() const;

private:
    std::optional<int64_t> mCurrentProgramIdx;
    std::mutex m;

    void restartProgramResources();
    void reStartProgram();
    void startProgramUnsafe();

    void updateAboutChange();

    NdiSourceChangeNotify mNdiSourceChangeNotify;

    void onInformCompressedType(H26x::FourCcType vidFourCcType);
};
}