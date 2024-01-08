#include "control_player.hpp"

ControlPlayer::ControlPlayer(Monitor::NdiSourceChangeNotify ndiSourceChangeNotify)
    : mMsgQueue{}
    , mProgramSwitch(ndiSourceChangeNotify, mMsgQueue)
{
    runControlThread();
}

void ControlPlayer::runControlThread()
{
    mControlThread.start("ControlThread", [this](const bool stop){
        (void)stop;

        Msg::Msg msg;
        if (mMsgQueue.pop(msg))
        {
            switch (msg.type)
            {
            case Msg::Type::StartProgram:
                mProgramSwitch.startProgram(std::get<uint32_t>(msg.body));
            break;

            case Msg::Type::DisconnectProgram:
                mProgramSwitch.stopProgram();
            break;

            case Msg::Type::RestartProgram:
                mProgramSwitch.reStartProgram();
            break;

            case Msg::Type::SwitchToUncompressed:
                mProgramSwitch.switchToUncompressed();
            break;

            case Msg::Type::StopProgram:
                mProgramSwitch.stopProgram();
            break;

            case Msg::Type::ScanNdiSources:
                mProgramSwitch.scanNdiSources();
            break;
            }
        }
    });
}
