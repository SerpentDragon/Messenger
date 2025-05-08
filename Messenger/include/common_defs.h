#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#include <string>

struct Message
{
    int sender;
    int receiver;
    std::string text;
    unsigned long long timestamp;
    int chat;
};

#endif // COMMON_DEFS_H
