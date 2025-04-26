#include "../../include/Client/cryptographer.h"

std::string Cryptographer::encode(const std::string& str)
{
    std::string result;

    return result;
}

std::string Cryptographer::decode(const std::string& str)
{
    std::string result;

    return result;
}

std::string Cryptographer::hash(const std::string& str)
{
    unsigned char hash[SHA512_DIGEST_LENGTH];
    SHA512((const unsigned char*)str.c_str(), str.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA512_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}
