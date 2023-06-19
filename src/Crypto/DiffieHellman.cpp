#include <iostream>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/conf.h>
#include <openssl/dh.h>

#include "DiffieHellman.hpp"

using namespace std;

DiffieHellman::DiffieHellman() 
{
    // p of the diffie hellman protocol
    static unsigned char dhp_2048[] = {
        0xDB, 0x53, 0x9C, 0x0B, 0xF0, 0xAE, 0x71, 0x24, 0x9B, 0xB8,
        0x43, 0x75, 0x67, 0x79, 0xD0, 0xD9, 0x44, 0xF8, 0x57, 0xD0,
        0x29, 0x28, 0x22, 0xF2, 0xD3, 0x36, 0xE4, 0x2C, 0xDC, 0x2B,
        0x5C, 0x22, 0x36, 0x13, 0x88, 0xFB, 0xBF, 0x22, 0x39, 0x19,
        0x6B, 0x02, 0x28, 0x89, 0x75, 0xE8, 0xE2, 0x0C, 0x81, 0xC1,
        0xBF, 0x28, 0x5D, 0xEF, 0x75, 0x4A, 0x49, 0x08, 0x3A, 0x6F,
        0xA5, 0xAE, 0xBF, 0xEA, 0x47, 0xD5, 0x7C, 0xAE, 0x13, 0x44,
        0x5A, 0xCC, 0xDF, 0x61, 0xC6, 0xA6, 0xE3, 0xE2, 0x53, 0xED,
        0x34, 0xF9, 0x75, 0x61, 0x49, 0x95, 0x1E, 0x2B, 0x90, 0x4D,
        0x9B, 0x72, 0x79, 0xC0, 0x36, 0x77, 0x06, 0xE9, 0x13, 0x08,
        0x84, 0x19, 0xC9, 0x62, 0xA3, 0xC1, 0x86, 0x13, 0xF0, 0xF9,
        0xA1, 0x54, 0x73, 0xB8, 0x54, 0xFC, 0x83, 0xFD, 0x51, 0x51,
        0xCE, 0x66, 0x33, 0xBA, 0x11, 0x10, 0xFB, 0x38, 0xD1, 0x03,
        0x71, 0x22, 0xD4, 0x34, 0xA6, 0x21, 0x49, 0x2A, 0x75, 0xCF,
        0xC1, 0xFE, 0xF0, 0xB0, 0x33, 0xA4, 0x0E, 0x34, 0xCB, 0xA0,
        0x4A, 0x8B, 0xA8, 0x65, 0x6C, 0x7C, 0xF9, 0xB8, 0x71, 0xBE,
        0xC6, 0xB5, 0xB3, 0x1E, 0x3B, 0xD3, 0x2B, 0x9B, 0xEC, 0x7D,
        0xD5, 0x4C, 0xBA, 0x18, 0xBB, 0xEE, 0xAB, 0x06, 0x67, 0x86,
        0x0B, 0x16, 0xAF, 0xDC, 0xBE, 0xB3, 0x09, 0x0B, 0x32, 0xDE,
        0x68, 0x1F, 0x81, 0x68, 0xC9, 0x56, 0x16, 0xA0, 0xC4, 0x4E,
        0x70, 0xA9, 0xB8, 0xD4, 0x71, 0x80, 0xF4, 0x56, 0xA4, 0x5E,
        0xFA, 0x5F, 0x9E, 0x48, 0x72, 0x22, 0xDB, 0xB6, 0x1E, 0x56,
        0x3A, 0xA5, 0xE9, 0x46, 0xF3, 0x1B, 0x3F, 0xA9, 0xA4, 0x0E,
        0xD4, 0x29, 0x0B, 0x6E, 0x4B, 0x08, 0x9C, 0x5F, 0x61, 0x5A,
        0xC9, 0x94, 0xCB, 0xA4, 0x16, 0x45, 0x52, 0x07, 0xFD, 0xBD,
        0x81, 0x1B, 0x34, 0x9D, 0xDE, 0x6B
    };

    // g of the diffie hellman protocol 
    static unsigned char dhg_2048[] = {
        0x02
    };

    DH *dh = DH_new();
    BIGNUM *p, *g;

    if (dh == NULL)
        throw std::runtime_error("\033[1;31m[ERROR]\033[0m DiffieHellman::DiffieHellman() >> Failed to create low level DH parameters structure");

    p = BN_bin2bn(dhp_2048, sizeof(dhp_2048), NULL);
    g = BN_bin2bn(dhg_2048, sizeof(dhg_2048), NULL);
    if (p == NULL || g == NULL || !DH_set0_pqg(dh, p, NULL, g)) {
        DH_free(dh);
        BN_free(p);
        BN_free(g);

        throw std::runtime_error("\033[1;31m[ERROR]\033[0m DiffieHellman::DiffieHellman() >> Failed to setup low level DH parameters structure");
    }

    m_dh_parameters = EVP_PKEY_new();
    if (!m_dh_parameters)
        throw std::runtime_error("\033[1;31m[ERROR]\033[0m DiffieHellman::DiffieHellman() >> Failed to create high level DH parameters structure");

    if(EVP_PKEY_set1_DH(m_dh_parameters, dh) != 1)
        throw std::runtime_error("\033[1;31m[ERROR]\033[0m DiffieHellman::DiffieHellman() >> Failed to setup high level DH parameters structure");

    DH_free(dh);
}

DiffieHellman::~DiffieHellman() 
{
    EVP_PKEY_free(m_dh_parameters);
}

EVP_PKEY* DiffieHellman::generateEphemeralKey() 
{
    EVP_PKEY_CTX *DH_ctx = EVP_PKEY_CTX_new(m_dh_parameters, NULL);
    if(!DH_ctx) 
        throw std::runtime_error("\033[1;31m[ERROR]\033[0m DiffieHellman::generateEphemeralKey() >> Failed to create DH context");

    EVP_PKEY *ephemeral_key = NULL;

    if(EVP_PKEY_keygen_init(DH_ctx) != 1)
        throw std::runtime_error("\033[1;31m[ERROR]\033[0m DiffieHellman::generateEphemeralKey() >> Failed to initialize DH context");

    if(EVP_PKEY_keygen(DH_ctx, &ephemeral_key) != 1)
        throw std::runtime_error("\033[1;31m[ERROR]\033[0m DiffieHellman::generateEphemeralKey() >> Failed to generate ephemeral key");
    
    EVP_PKEY_CTX_free(DH_ctx);
    return ephemeral_key;
}

int DiffieHellman::generateSharedSecret(EVP_PKEY* private_key, EVP_PKEY* peer_ephemeral_key, unsigned char*& shared_secret, size_t& shared_secret_size) {

    EVP_PKEY_CTX* derive_ctx = EVP_PKEY_CTX_new(private_key, NULL);
    if (!derive_ctx) {
        cerr << "[-] (DiffieHellman) Failed to create derive context" << endl;
        return -1;
    }

    if (EVP_PKEY_derive_init(derive_ctx) <= 0) {
        cerr << "[-] (DiffieHellman) Failed to initialize derive context" << endl;
        return -1;
    }

    if (EVP_PKEY_derive_set_peer(derive_ctx, peer_ephemeral_key) <= 0) {
        cerr << "[-] (DiffieHellman) Failed to set peer ephemeral keys in the context" << endl;
        return -1;
    }

    EVP_PKEY_derive(derive_ctx, NULL, &shared_secret_size);
    shared_secret = new unsigned char[int(shared_secret_size)];
    if (EVP_PKEY_derive(derive_ctx, shared_secret, &shared_secret_size) <= 0) {
        cerr << "[-] (DiffieHellman) Failed to generate shared secret" << endl;
        return -1;
    }
    
    EVP_PKEY_CTX_free(derive_ctx);
    return 0;
}

int DiffieHellman::serializeKey(EVP_PKEY* key, uint8_t*& serialized_key, int& serialized_key_size) {

    BIO *bio = BIO_new(BIO_s_mem());
    if (!bio) {
        cerr << "[-] (DiffieHellman) Failed to create BIO" << endl;
        BIO_free(bio);
        return -1;
    }

    if (!PEM_write_bio_PUBKEY(bio, key)) {
        cerr << "[-] (DiffieHellman) Failed to write key in the BIO" << endl;
        BIO_free(bio);
        return -1;
    }

    serialized_key_size = BIO_pending(bio);
    serialized_key = new uint8_t[1024];

    int read = BIO_read(bio, serialized_key, serialized_key_size);
    if (read != serialized_key_size) {
        cerr << "[-] (DiffieHellman) Failed to write the serialized key in the buffer" << endl;
        BIO_free(bio);
        delete[] serialized_key;
        return -1;
    }
    
    BIO_free(bio);
    return 0;
}

EVP_PKEY* DiffieHellman::deserializeKey(uint8_t* serialized_key, int serialized_key_size) {

    BIO *bio = BIO_new_mem_buf(serialized_key, serialized_key_size);
    if (!bio) {
        cerr << "[-] (DiffieHellman) Failed to create the BIO" << endl;
        return nullptr;
    }

    EVP_PKEY* deserialized_key = nullptr;
    deserialized_key = PEM_read_bio_PUBKEY(bio, NULL, NULL, NULL);
    if (!deserialized_key) {        
        cerr << "[-] (DiffieHellman) Failed to read the deserialized key" << endl;
        BIO_free(bio);
        return nullptr;
    }

    BIO_free(bio);
    return deserialized_key;
}