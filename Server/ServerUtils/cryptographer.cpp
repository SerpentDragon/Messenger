#include "cryptographer.h"

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

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
    if (!ctx) return;

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

    EVP_PKEY_CTX_free(ctx);

    BIO* pub_bio = BIO_new(BIO_s_mem());

    if (!PEM_write_bio_PUBKEY(pub_bio, private_key_))
    {
        BIO_free(pub_bio);
        EVP_PKEY_free(private_key_);
        return;
    }

    public_key_ = PEM_read_bio_PUBKEY(pub_bio, nullptr, nullptr, nullptr);

    BIO_free(pub_bio);
    if (!public_key_)
    {
        EVP_PKEY_free(private_key_);
        return;
    }
}

std::pair<std::string, std::string> Cryptographer::serialize_RSA_keys() const
{
    auto k1 = serialize_private_key();
    auto k2 = serialize_public_key();

    return { k1, k2 };
}

void Cryptographer::deserialize_RSA_keys(const std::pair<std::string, std::string>& keys)
{
    private_key_ = deserialize_private_key(keys.first);
    public_key_ = deserialize_public_key(keys.second);
}

std::string Cryptographer::serialize_private_key() const
{
    BIO* mem = BIO_new(BIO_s_mem());
    PEM_write_bio_PrivateKey(mem, private_key_, nullptr, nullptr, 0, nullptr, nullptr);

    char* data = nullptr;
    auto len = BIO_get_mem_data(mem, &data);
    std::string private_key(data, len);

    BIO_free(mem);

    return private_key;
}

std::string Cryptographer::serialize_public_key() const
{
    BIO* mem = BIO_new(BIO_s_mem());
    PEM_write_bio_PUBKEY(mem, public_key_);

    char* data = nullptr;
    auto len = BIO_get_mem_data(mem, &data);
    std::string public_key(data, len);

    BIO_free(mem);

    return public_key;
}

EVP_PKEY* Cryptographer::deserialize_private_key(const std::string& key)
{
    BIO* mem = BIO_new_mem_buf(key.data(), static_cast<int>(key.size()));
    EVP_PKEY* private_key = PEM_read_bio_PrivateKey(mem, nullptr, nullptr, nullptr);
    BIO_free(mem);

    return private_key;
}

EVP_PKEY* Cryptographer::deserialize_public_key(const std::string& key)
{
    BIO* mem = BIO_new_mem_buf(key.data(), static_cast<int>(key.size()));
    EVP_PKEY* public_key = PEM_read_bio_PUBKEY(mem, nullptr, nullptr, nullptr);
    BIO_free(mem);

    return public_key;
}

Cryptographer::AES_VECTOR Cryptographer::encrypt_AES(const std::string& plaintext, const std::string& pub_key)
{
    auto result = encrypt_AES_msg(plaintext, pub_key);

    std::vector<uint8_t> full_message;
    full_message.insert(full_message.end(), result.rsa_encrypted_key.begin(), result.rsa_encrypted_key.end());
    full_message.insert(full_message.end(), result.aes_iv.begin(), result.aes_iv.end());
    full_message.insert(full_message.end(), result.aes_encrypted_data.begin(), result.aes_encrypted_data.end());

    return full_message;
}

std::string Cryptographer::decrypt_AES(const AES_VECTOR& buffer)
{
    size_t offset = 0;
    AES_VECTOR encrypted_aes_key(buffer.begin(), buffer.begin() + 256);
    offset += 256;

    AES_VECTOR aes_iv(buffer.begin() + offset, buffer.begin() + offset + 16);
    offset += 16;

    AES_VECTOR ciphertext(buffer.begin() + offset, buffer.end());

    return decrypt_AES({ encrypted_aes_key, aes_iv, ciphertext });
}

std::pair<Cryptographer::AES_VECTOR, Cryptographer::AES_VECTOR>
Cryptographer::generate_AES_key()
{
    constexpr int AES_KEY_LENGTH = 32;
    constexpr int AES_IV_LENGTH = 16;

    AES_VECTOR aes_key(AES_KEY_LENGTH);
    AES_VECTOR aes_iv(AES_IV_LENGTH);

    RAND_bytes(aes_key.data(), AES_KEY_LENGTH);
    RAND_bytes(aes_iv.data(), AES_IV_LENGTH);

    return { aes_key, aes_iv };
}

Cryptographer::AES_VECTOR Cryptographer::encrypt_AES_key_RSA(const Cryptographer::AES_VECTOR& aes_key,
    EVP_PKEY* recipient_pubkey)
{
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(recipient_pubkey, nullptr);
    EVP_PKEY_encrypt_init(ctx);

    size_t outlen = 0;
    EVP_PKEY_encrypt(ctx, nullptr, &outlen, aes_key.data(), aes_key.size());

    AES_VECTOR encrypted_key(outlen);
    EVP_PKEY_encrypt(ctx, encrypted_key.data(), &outlen, aes_key.data(), aes_key.size());
    encrypted_key.resize(outlen);

    EVP_PKEY_CTX_free(ctx);

    return encrypted_key;
}

Cryptographer::AES_VECTOR Cryptographer::decrypt_AES_key_RSA(const Cryptographer::AES_VECTOR& encrypted_key)
{
    AES_VECTOR decrypted_key(256);

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(private_key_, nullptr);

    if (!ctx) return {};
    if (EVP_PKEY_decrypt_init(ctx) <= 0) return {};

    size_t outlen = decrypted_key.size();
    if (EVP_PKEY_decrypt(ctx,
                         decrypted_key.data(), &outlen,
                         encrypted_key.data(), encrypted_key.size()) <= 0)
    {
        EVP_PKEY_CTX_free(ctx);
        return {};
    }

    EVP_PKEY_CTX_free(ctx);
    decrypted_key.resize(outlen);

    return decrypted_key;
}

Cryptographer::EncryptedPayload Cryptographer::encrypt_AES_msg(const std::string& plaintext, const std::string& pubkey)
{
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

    return EncryptedPayload
        {
            .rsa_encrypted_key = std::move(key),
            .aes_iv = std::move(aes_iv),
            .aes_encrypted_data = std::move(ciphertext)
        };
}

std::string Cryptographer::decrypt_AES(const EncryptedPayload& payload)
{
    AES_VECTOR aes_key = decrypt_AES_key_RSA(payload.rsa_encrypted_key);

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

    return std::string(decrypted.begin(), decrypted.end());
}
