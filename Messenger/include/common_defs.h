#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#include "../../API/common_structure.h"

#include <chrono>
#include <QString>
#include <string>

// defines for variables

constexpr int max_msg_count = 20;
constexpr int avg_reading_speed = 20;
constexpr int addl_time = 5;

enum P2P_CONNECTION_STATUS : unsigned short
{
    SUCCESSFUL      = 0,
    SERVER_FALLBACK = 1,
    DISCONNECTED    = 2
};

using ULL = unsigned long long;

// data structures and functions

struct SocketMessage
{
    bool system = false;
    int sender;
    std::vector<int> receiver;
    std::string text;
    ULL timestamp;
    int chat;
    bool vanishing;
    P2P_CONNECTION_TYPE p2p;
};

struct ClientMessage
{
    int id;
    std::string sender;
    int sender_id;
    std::string receiver;
    int receiver_id;
    std::string text;
    std::string time;
    int chat;
    bool vanishing;

    explicit ClientMessage(int Id, const std::string& sndr, int sndr_id,
        const std::string& rcvr, int rcvr_id, const std::string& txt, ULL tm, int cht, bool vnshng)
        : id(Id), sender(sndr), sender_id(sndr_id), receiver(rcvr),
        receiver_id(rcvr_id), text(txt), chat(cht), vanishing(vnshng)
    {
        build_timestamp(tm);
    }

private:

    void build_timestamp(ULL tm)
    {
        std::chrono::milliseconds ms(tm);
        std::chrono::time_point<std::chrono::system_clock> tp(ms);
        std::time_t time_type = std::chrono::system_clock::to_time_t(tp);
        std::tm* local_tm = std::localtime(&time_type);

        std::ostringstream oss;
        oss << std::put_time(local_tm, "%d.%m.%Y %H:%M");

        this->time = oss.str();
    }
};

static unsigned long long generate_timestamp()
{
    auto now = std::chrono::system_clock::now();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                            now.time_since_epoch()).count();

    return milliseconds;
}

#endif // COMMON_DEFS_H
