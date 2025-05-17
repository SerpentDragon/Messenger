#pragma once

#include <boost/shared_ptr.hpp>
#include <boost/array.hpp>
#include <optional>
#include <queue>
#include <unordered_map>

#include "../../API/common_structure.h"

class MessageQueue
{
private:

    using BUFFER = boost::shared_ptr<boost::array<char, max_msg_length>>;

public:

    MessageQueue(const MessageQueue&) = delete;

    MessageQueue& operator=(const MessageQueue&) = delete;

    static MessageQueue& get_queue();

    bool check_msg_queue(int client_id);

    void add_msg(int client, BUFFER data);

    std::optional<BUFFER> get_next_msg(int client_id);

private:

    MessageQueue() = default;

    ~MessageQueue() = default;

private:

    std::unordered_map<int, std::queue<BUFFER>> queue;
};
