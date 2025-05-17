#ifndef CRYPTOGRAPHER_H
#define CRYPTOGRAPHER_H

#include <iomanip>
#include <openssl/sha.h>
#include <QString>
#include <sstream>
#include <string>

class Cryptographer
{
public:

    Cryptographer() = default;

    static std::string encode(const QString& str);

    static std::string decode(const QString& str);

    static std::string hash(const QString& str);

private:

};

#endif // CRYPTOGRAPHER_H
