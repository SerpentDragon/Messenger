#include "messagequeue.h"

MessageQueue& MessageQueue::get_queue()
{
    static MessageQueue msgq;
    return msgq;
}

bool MessageQueue::check_msg_queue(int client_id)
{
    return queue.contains(client_id) && !queue[client_id].empty();
}

void MessageQueue::add_msg(int client, BUFFER data)
{
    queue[client].push(data);
}

std::optional<MessageQueue::BUFFER> MessageQueue::get_next_msg(int client_id)
{
    std::optional<BUFFER> res = std::nullopt;

    if (queue.contains(client_id) && !queue[client_id].empty())
    {
        res = queue[client_id].front();
        queue[client_id].pop();
    }

    return res;
}
