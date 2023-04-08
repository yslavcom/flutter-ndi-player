#include "player.hpp"

void Player::onFrame(FrameQueue::VideoFrame* frame, size_t remainingCount)
{
    if (!frame)
    {
        return;
    }

    // render the frame
}