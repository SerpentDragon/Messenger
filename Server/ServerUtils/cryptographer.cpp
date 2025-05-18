#include "cryptographer.h"

#include <iostream>
#include <cassert>

std::shared_ptr<Cryptographer> Cryptographer::cr_ptr = nullptr;

Cryptographer::~Cryptographer()
{
    if (public_key_) EVP_PKEY_free(public_key_);
    if (private_key_) EVP_PKEY_free(private_key_);
}

std::shared_ptr<Cryptographer> Cryptographer::get_cryptographer()
{
    if (cr_ptr == nullptr)
    {
        cr_ptr = std::shared_ptr<Cryptographer>(new Cryptographer());
    }

    return cr_ptr;
}

void Cryptographer::generate_RSA_keys()
{
    public_key_ = nullptr;
    private_key_ = nullptr;

    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    

    std::cout << __func__ << '\n';

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) return;

    std::cout << 0 << '\n';

    std::cout << 1 << '\n';

    if (EVP_PKEY_keygen_init(ctx) <= 0) {
        std::cerr << "keygen_init failed\n";
        ERR_print_errors_fp(stdout);
        EVP_PKEY_CTX_free(ctx);
        return;
    }

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
        std::cerr << "set_rsa_keygen_bits failed\n";
        ERR_print_errors_fp(stdout);
        EVP_PKEY_CTX_free(ctx);
        return;
    }

    if (EVP_PKEY_keygen(ctx, &private_key_) <= 0) {
        std::cerr << "keygen failed\n";
        ERR_print_errors_fp(stdout);
        EVP_PKEY_CTX_free(ctx);
        return;
    }

    std::cout << 2 << '\n';

    EVP_PKEY_CTX_free(ctx);

    std::cout << 3 << '\n';

    BIO* pub_bio = BIO_new(BIO_s_mem());
    std::cout << 4 << '\n';

    if (!PEM_write_bio_PUBKEY(pub_bio, private_key_))
    {
        BIO_free(pub_bio);
        EVP_PKEY_free(private_key_);
        return;
    }

    std::cout << 5 << '\n';


    public_key_ = PEM_read_bio_PUBKEY(pub_bio, nullptr, nullptr, nullptr);
    BIO_free(pub_bio);

    std::cout << 6 << '\n';

    if (!public_key_)
    {
        EVP_PKEY_free(private_key_);
        return;
    }
    std::cout << 7 << '\n';

    std::cout << "leave " << __func__ << '\n';
}

std::pair<std::string, std::string> Cryptographer::serialize_RSA_keys() const
{
    std::cout << __func__ << '\n';

    auto k1 = serialize_private_key();
    auto k2 = serialize_public_key();

    // std::cout << k1 << "\n\n" << k2 << '\n';

    std::cout << "leave " << __func__ << '\n';

    return { k1, k2 };
}

void Cryptographer::deserialize_RSA_keys(const std::pair<std::string, std::string>& keys)
{
    std::cout << __func__ << '\n';
    private_key_ = deserialize_private_key(keys.first);
    public_key_ = deserialize_public_key(keys.second);

    std::cout << "leave " << __func__ << '\n';

}

std::string Cryptographer::serialize_private_key() const
{
    std::cout << __func__ << '\n';
    BIO* mem = BIO_new(BIO_s_mem());
    PEM_write_bio_PrivateKey(mem, private_key_, nullptr, nullptr, 0, nullptr, nullptr);

    char* data = nullptr;
    auto len = BIO_get_mem_data(mem, &data);
    std::string private_key(data, len);

    BIO_free(mem);

    std::cout << "leave " << __func__ << '\n';

    return private_key;
}

std::string Cryptographer::serialize_public_key() const
{
    std::cout << __func__ << '\n';
    BIO* mem = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(mem, public_key_);

    char* data = nullptr;
    auto len = BIO_get_mem_data(mem, &data);
    std::string public_key(data, len);

    BIO_free(mem);

    std::cout << "leave " << __func__ << '\n';

    return public_key;
}

EVP_PKEY* Cryptographer::deserialize_private_key(const std::string& key)
{
    std::cout << __func__ << '\n';
    BIO* mem = BIO_new_mem_buf(key.data(), static_cast<int>(key.size()));
    EVP_PKEY* private_key = PEM_read_bio_PrivateKey(mem, nullptr, nullptr, nullptr);
    BIO_free(mem);

    std::cout << "leave " << __func__ << '\n';

    return private_key;
}

EVP_PKEY* Cryptographer::deserialize_public_key(const std::string& key)
{
    std::cout << __func__ << '\n';
    BIO* mem = BIO_new_mem_buf(key.data(), static_cast<int>(key.size()));
    EVP_PKEY* public_key = PEM_read_bio_PUBKEY(mem, nullptr, nullptr, nullptr);
    BIO_free(mem);

    std::cout << "leave " << __func__ << '\n';

    return public_key;
}

Cryptographer::AES_VECTOR Cryptographer::encrypt_AES(const std::string& plaintext, const std::string& pub_key)
{
    std::cout << __func__ << '\n';
    auto result = encrypt_AES_msg(plaintext, pub_key);

    std::vector<uint8_t> full_message;
    full_message.insert(full_message.end(), result.rsa_encrypted_key.begin(), result.rsa_encrypted_key.end());
    full_message.insert(full_message.end(), result.aes_iv.begin(), result.aes_iv.end());
    full_message.insert(full_message.end(), result.aes_encrypted_data.begin(), result.aes_encrypted_data.end());

    std::cout << "AES DATA\n";

    std::cout << "KEY\n";
    for(uint8_t c : result.rsa_encrypted_key) std::cout << (int)c;
    std::cout << "\n\n";
    std::cout << "IV\n";
    for(uint8_t c : result.aes_iv) std::cout << (int)c;
    std::cout << "\n\n";
    std::cout << "AES\n";
    for(uint8_t c : result.aes_encrypted_data) std::cout << (int)c;
    std::cout << "\n\n";

    std::cout << "leave " << __func__ << '\n';

    return full_message;
}

std::string Cryptographer::decrypt_AES(const AES_VECTOR& buffer)
{
    std::cout << __func__ << '\n';
    size_t offset = 0;
    AES_VECTOR encrypted_aes_key(buffer.begin(), buffer.begin() + 256);
    offset += 256;

    AES_VECTOR aes_iv(buffer.begin() + offset, buffer.begin() + offset + 16);
    offset += 16;

    AES_VECTOR ciphertext(buffer.begin() + offset, buffer.end());

    std::cout << "KEY\n";
    for(uint8_t c : encrypted_aes_key) std::cout << std::dec << ' ' << (int)c;
    std::cout << "\n\n";
    std::cout << "IV\n";
    for(uint8_t c : aes_iv) std::cout << ' ' << (int)c;
    std::cout << "\n\n";
    std::cout << "TEXT\n";
    for(uint8_t c : ciphertext) std::cout << ' ' << (int)c;
    std::cout << "\n\n";

    std::cout << "leave " << __func__ << '\n';

    return decrypt_AES({ encrypted_aes_key, aes_iv, ciphertext });
}

std::pair<Cryptographer::AES_VECTOR, Cryptographer::AES_VECTOR>
Cryptographer::generate_AES_key()
{
    std::cout << __func__ << '\n';
    constexpr int AES_KEY_LENGTH = 32;
    constexpr int AES_IV_LENGTH = 16;

    AES_VECTOR aes_key(AES_KEY_LENGTH);
    AES_VECTOR aes_iv(AES_IV_LENGTH);

    RAND_bytes(aes_key.data(), AES_KEY_LENGTH);
    RAND_bytes(aes_iv.data(), AES_IV_LENGTH);

    std::cout << "leave " << __func__ << '\n';

    return { aes_key, aes_iv };
}

Cryptographer::AES_VECTOR Cryptographer::encrypt_AES_key_RSA(const Cryptographer::AES_VECTOR& aes_key,
    EVP_PKEY* recipient_pubkey)
{
    std::cout << __func__ << '\n';
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(recipient_pubkey, nullptr);
    EVP_PKEY_encrypt_init(ctx);

    size_t outlen = 0;
    EVP_PKEY_encrypt(ctx, nullptr, &outlen, aes_key.data(), aes_key.size());

    AES_VECTOR encrypted_key(outlen);
    EVP_PKEY_encrypt(ctx, encrypted_key.data(), &outlen, aes_key.data(), aes_key.size());
    encrypted_key.resize(outlen);

    EVP_PKEY_CTX_free(ctx);

    std::cout << "leave " << __func__ << '\n';
    
    return encrypted_key;
}

Cryptographer::AES_VECTOR Cryptographer::decrypt_AES_key_RSA(const Cryptographer::AES_VECTOR& encrypted_key)
{
    std::cout << __func__ << '\n';
    AES_VECTOR decrypted_key(256);

    assert(private_key_ != nullptr);

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(private_key_, nullptr);
    if (!ctx) return {};

    std::cout << __func__ << 1 << '\n';

    if (EVP_PKEY_decrypt_init(ctx) <= 0) return {};

    std::cout << __func__ << 2 << '\n';

    size_t outlen = decrypted_key.size();
    if (EVP_PKEY_decrypt(ctx,
                         decrypted_key.data(), &outlen,
                         encrypted_key.data(), encrypted_key.size()) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    std::cout << __func__ << 3 << '\n';

    EVP_PKEY_CTX_free(ctx);
    decrypted_key.resize(outlen);
    

    std::cout << "leave " << __func__ << '\n';

    return decrypted_key;
}

Cryptographer::EncryptedPayload Cryptographer::encrypt_AES_msg(const std::string& plaintext, const std::string& pubkey)
{
    std::cout << __func__ << '\n';
    auto [aes_key, aes_iv] = generate_AES_key();
    auto key = encrypt_AES_key_RSA(aes_key, deserialize_public_key(pubkey));

    AES_VECTOR ciphertext(plaintext.size() + AES_BLOCK_SIZE);
    int len = 0, ciphertext_len = 0;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, aes_key.data(), aes_iv.data());

    EVP_EncryptUpdate(ctx,
                      ciphertext.data(), &len,
                      reinterpret_cast<const unsigned char*>(plaintext.data()),
                      plaintext.size());
    ciphertext_len = len;

    EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len);
    ciphertext_len += len;
    ciphertext.resize(ciphertext_len);

    EVP_CIPHER_CTX_free(ctx);

    std::cout << "leave " << __func__ << '\n';

    return EncryptedPayload
        {
            .rsa_encrypted_key = std::move(key),
            .aes_iv = std::move(aes_iv),
            .aes_encrypted_data = std::move(ciphertext)
        };
}

std::string Cryptographer::decrypt_AES(const EncryptedPayload& payload)
{
    std::cout << __func__ << '\n';
    AES_VECTOR aes_key = decrypt_AES_key_RSA(payload.rsa_encrypted_key);

    std::cout << "DECRYPTED KEY\n";
    for(uint8_t c : aes_key) std::cout << ' ' << (int)c;
    std::cout << "\n\n";
    // EVP_PKEY_free(private_key_);

    AES_VECTOR decrypted(payload.aes_encrypted_data.size());
    int len = 0, plaintext_len = 0;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, aes_key.data(), payload.aes_iv.data());

    EVP_DecryptUpdate(ctx, decrypted.data(), &len,
                      payload.aes_encrypted_data.data(),
                      payload.aes_encrypted_data.size());
    plaintext_len = len;

    EVP_DecryptFinal_ex(ctx, decrypted.data() + len, &len);
    plaintext_len += len;
    decrypted.resize(plaintext_len);

    EVP_CIPHER_CTX_free(ctx);

    std::cout << "leave " << __func__ << '\n';

    return std::string(decrypted.begin(), decrypted.end());
}
