#include <iostream>
#include <string>
#include <iomanip>
#include <ctime>
#include <cstring>
#include <unistd.h>
#include <stdexcept>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "../Packet/List.hpp"
#include "../Packet/ClientReq.hpp"
#include "../Packet/Handshake.hpp"
#include "../Packet/SessionMessage.hpp"

#include "../Generic/Codes.hpp"
#include "../Generic/Utility.hpp"

#include "../Crypto/DiffieHellman.hpp"

class Client {
public:
    Client(const std::string& server_ip, int server_port);
    ~Client();
    void connect_to_server();
    void send_to_server(const std::vector<uint8_t>& buffer);
    void recv_from_server(std::vector<uint8_t>& buffer);
    
    int handshake();

    void balance();
    void transfer();
    void list();

private:
    int sock_fd;
    struct sockaddr_in server_address;

    uint32_t m_counter; 
    std::vector<uint8_t> hmac_key;
    std::vector<uint8_t> session_key;
    EVP_PKEY* m_long_term_key;

    std::string m_username;

    void print_formatted_date(std::time_t timestamp);
};
