#pragma once

#include "program_switch.hpp"
#include "common/custom_thread.hpp"
#include "common/msg_queue.hpp"

class ControlPlayer
{
public:
    ControlPlayer(Monitor::NdiSourceChangeNotify ndiSourceChangeNotify);

    Msg::Queue& getMsgQueue()
    {
        return mMsgQueue;
    }

    const Monitor::ProgramSwitch& getProgramSwitch()
    {
        return mProgramSwitch;
    }

private:
    Msg::Queue mMsgQueue;
    Monitor::ProgramSwitch mProgramSwitch;
    CustomThread mControlThread;

    void runControlThread();
};