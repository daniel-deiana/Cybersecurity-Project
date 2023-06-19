#include <openssl/evp.h>

class DiffieHellman {

    EVP_PKEY *m_dh_parameters;

public:
    DiffieHellman();
    DiffieHellman(const DiffieHellman&) = delete;
    ~DiffieHellman();

    EVP_PKEY* generateEphemeralKey();
    int generateSharedSecret(EVP_PKEY* private_key, EVP_PKEY* peer_ephemeral_key, unsigned char*& shared_secret, size_t& shared_secret_size);

    static int serializeKey(EVP_PKEY* key, uint8_t*& serialized_key, int& serialized_key_size);
    static EVP_PKEY* deserializeKey(uint8_t* serialized_key, int serialized_key_size);
};
