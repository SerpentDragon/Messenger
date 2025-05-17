#pragma once

namespace USER_DATA
{
    inline constexpr char log_in[] = "data.log_in";
    inline constexpr char nickname[] = "data.user.nickname";
    inline constexpr char password[] = "data.user.password";
};

namespace SERVER_RESPONSE
{
    inline constexpr char status[] = "response.status";
    inline constexpr char id[] = "response.id";
};

namespace MSG_TAGS
{
    inline constexpr char msg[] = "msg";
    inline constexpr char system[] = "msg.system";
    inline constexpr char sender[] = "msg.sender";
    inline constexpr char receiver[] = "msg.receiver";
    inline constexpr char text[] = "msg.text";
    inline constexpr char timestamp[] = "msg.timestamp";
    inline constexpr char chat[] = "msg.chat";
};

namespace SYSTEM_MSG_DATA
{
    inline constexpr char system[] = "msg.system";
    inline constexpr char sender[] = "msg.sender";
    inline constexpr char cmd[] = "msg.cmd";
    inline constexpr char data[] = "msg.data";
    inline constexpr char contact[] = "msg.data.contact";
};

enum SYSTEM_MSG : unsigned int
{
    FIND_CONTACT = 1,
};
