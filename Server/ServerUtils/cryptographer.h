#ifndef CRYPTOGRAPHER_H
#define CRYPTOGRAPHER_H

#include <iomanip>
#include <openssl/aes.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

class Cryptographer
{
private:

    using AES_VECTOR = std::vector<unsigned char>;

public:

    struct EncryptedPayload
    {
        AES_VECTOR rsa_encrypted_key;
        AES_VECTOR aes_iv;
        AES_VECTOR aes_encrypted_data;
    };

public:

    Cryptographer(const Cryptographer&) = delete;

    Cryptographer operator=(const Cryptographer&) = delete;

    ~Cryptographer();

    static std::shared_ptr<Cryptographer> get_cryptographer();

    void generate_RSA_keys();

    std::pair<std::string, std::string> serialize_RSA_keys() const;

    void deserialize_RSA_keys(const std::pair<std::string, std::string>& keys);

    std::string serialize_private_key() const;

    std::string serialize_public_key() const;

    EVP_PKEY* deserialize_private_key(const std::string& key);

    EVP_PKEY* deserialize_public_key(const std::string& key);

    AES_VECTOR encrypt_AES(const std::string& plaintext, const std::string& pub_key);

    std::string decrypt_AES(const AES_VECTOR& buffer);

private:

    std::pair<AES_VECTOR, AES_VECTOR> generate_AES_key();

    std::vector<unsigned char> encrypt_AES_key_RSA(const AES_VECTOR& aes_key,
                                                   EVP_PKEY* recipient_pubkey);

    AES_VECTOR decrypt_AES_key_RSA(const AES_VECTOR& encrypted_key);

    EncryptedPayload encrypt_AES_msg(const std::string& plaintext, const std::string& pub_key);

    std::string decrypt_AES(const EncryptedPayload& payload);

private:

    Cryptographer() = default;

    static std::shared_ptr<Cryptographer> cr_ptr;

private:

    EVP_PKEY* private_key_ = nullptr;
    EVP_PKEY* public_key_ = nullptr;
};

#endif // CRYPTOGRAPHER_H
