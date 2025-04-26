#pragma once

enum SERVER_RESP_CODES : int
{
    OK = 0,
    ERROR,
    NO_USER_NICKNAME,
    WRONG_NICKNAME_PSSWD,
    NICKNAME_TAKEN,


    MAX_CODE
};

static inline SERVER_RESP_CODES int2SRV_RSP_CD(int val)
{
    if (OK <= val && val < SERVER_RESP_CODES::MAX_CODE)
    {
        return static_cast<SERVER_RESP_CODES>(val);
    }

    return SERVER_RESP_CODES::ERROR;
}
