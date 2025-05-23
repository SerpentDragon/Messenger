#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <optional>
#include <queue>
#include <unordered_map>

#include "../../API/protocols.h"

class MessageQueue
{
private:

    using BUFFER = std::vector<uint8_t>;

public:

    MessageQueue(const MessageQueue&) = delete;

    MessageQueue& operator=(const MessageQueue&) = delete;

    static MessageQueue& get_queue();

    bool check_msg_queue(int client_id);

    void add_msg(int client, bool need_to_encrypt, BUFFER data);

    std::optional<std::pair<bool, MessageQueue::BUFFER>> get_next_msg(int client_id);

private:

    MessageQueue() = default;

    ~MessageQueue() = default;

private:

    std::unordered_map<int, std::queue<std::pair<bool, BUFFER>>> queue;
};
