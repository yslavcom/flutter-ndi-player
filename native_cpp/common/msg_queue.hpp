#pragma once

#include "common/logger.hpp"

#include <mutex>
#include <queue>
#include <vector>

#define _DBG_MSG_QUEUE

#ifdef _DBG_MSG_QUEUE
    #define DBG_MSG_QUEUE(format, ...) LOGW(format, ## __VA_ARGS__)
#else
    #define DBG_MSG_QUEUE(format, ...)
#endif

namespace Msg
{
using MsgBody = std::vector<uint8_t>;

enum class MsgType
{
    StartProgram,
    DisconnectProgram,
    RestartProgram,
    SwitchToUncompressed
};
struct Msg
{
    MsgType type;
    // contents depend on type
    MsgBody body;

    Msg() = default;
    Msg(MsgType _type, MsgBody _body)
        : type(_type)
        , body(_body)
    {}
    Msg(const Msg& rhs) = delete;
    Msg& operator=(const Msg& rhs) = delete;
    Msg(Msg&& rhs)
    {
        type = std::move(rhs.type);
        body = std::move(rhs.body);
    }
    Msg& operator=(Msg&& rhs)
    {
        type = std::move(rhs.type);
        body = std::move(rhs.body);
        return *this;
    }
};

class Queue
{
public:

    bool push(Msg&& msg)
    {
        std::lock_guard lk(m);
        try {
            mQueue.push(std::move(msg));
            return true;
        }
        catch (std::exception& e){
            DBG_MSG_QUEUE("Push failed: %s\n", e.what());
            return false;
        }
    }

    bool pop(Msg& msg)
    {
        std::lock_guard lk(m);

        if (mQueue.size())
        {
            msg = std::move(mQueue.front());
            mQueue.pop();
            return true;
        }
        return false;
    }

private:
    std::queue<Msg> mQueue;
    std::mutex m;
};
}