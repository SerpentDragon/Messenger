#pragma once

inline constexpr int max_msg_length = 5000;
inline constexpr char msg_end[] = "<END>";
inline constexpr char recv_open_tag[] = "<receiver>";
inline constexpr char recv_close_tag[] = "</receiver>";

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
    inline constexpr char vanishing[] = "msg.vanishing";
};

namespace SYSTEM_MSG_DATA
{
    inline constexpr char system[] = "msg.system";
    inline constexpr char sender[] = "msg.sender";
    inline constexpr char cmd[] = "msg.cmd";
    inline constexpr char data[] = "msg.data";
    inline constexpr char contact[] = "msg.data.contact";
    inline constexpr char key[] = "msg.data.key";
    inline constexpr char chat_id[] = "msg.data.chat_id";
    inline constexpr char chat_name[] = "msg.data.chat_name";
    inline constexpr char chat_time[] = "msg.data.chat_time";
    inline constexpr char chat_member[] = "msg.data.chat_member";
};

enum SYSTEM_MSG : unsigned int
{
    LOAD_RSA_KEY      = 0,
    FIND_CONTACT      = 1,
    GET_CONTACT       = 2,
    NEW_GROUP_CHAT    = 3,
    DELETE_GROUP_CHAT = 4,
    DELETE_MESSAGE    = 5,
};
