#include "../../include/Client/cryptographer.h"

std::string Cryptographer::encode(const QString& str)
{
    std::string result;

    return result;
}

std::string Cryptographer::decode(const QString& str)
{
    std::string result;

    return result;
}

std::string Cryptographer::hash(const QString& str)
{
    QByteArray ba = str.toUtf8();

    unsigned char hash[SHA512_DIGEST_LENGTH];
    SHA512(reinterpret_cast<const unsigned char*>(ba.constData()), ba.size(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA512_DIGEST_LENGTH; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];

    return ss.str();
}
