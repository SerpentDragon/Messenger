#ifndef CRYPTOGRAPHER_H
#define CRYPTOGRAPHER_H

#include <iomanip>
#include <openssl/sha.h>
#include <sstream>
#include <string>

class Cryptographer
{
public:

    Cryptographer() = default;

    static std::string encode(const std::string& str);

    static std::string decode(const std::string& str);

    static std::string hash(const std::string& str);

private:

};

#endif // CRYPTOGRAPHER_H
