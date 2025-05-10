#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#include <chrono>
#include <string>

struct Message
{
    bool system = false;
    int sender;
    int receiver;
    std::string text;
    unsigned long long timestamp;
    int chat;
};

static unsigned long long generate_timestamp()
{
    auto now = std::chrono::system_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch()).count();

    return milliseconds;
}

#endif // COMMON_DEFS_H
