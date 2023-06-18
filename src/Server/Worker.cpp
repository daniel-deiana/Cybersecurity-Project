#include "Worker.hpp"

Worker::Worker(jobs_t* jobs) 
{
    this->jobs = jobs;
}

void Worker::Run() 
{
    while (true) {
        {
            std::unique_lock<std::mutex> lock(jobs->socket_mutex);
            jobs->socket_cv.wait(lock, [&]() { return !jobs->socket_queue.empty() || jobs->stop; });

            if (jobs->stop) 
            {
                #ifdef DEBUG
                std::cout << BLUE_BOLD << "[WORKER]" << RESET << " >> stop" << std::endl;
                #endif
                return;
            }

            client_socket = jobs->socket_queue.front();
            jobs->socket_queue.erase(jobs->socket_queue.begin());
        }

        #ifdef DEBUG
        std::cout << BLUE_BOLD << "[WORKER]" << RESET 
                  << " >> Client connected (socket: "
                  << client_socket << ")." << std::endl;
        #endif

        try {
            // Exchange session key

            ClientReq request;
            while(true) 
            {
                request = RequestHandler();

                #ifdef DEBUG
                std::cout << BLUE_BOLD << "[WORKER]" << RESET << " >> ";
                std::cout << "Client " << client_socket << " -> " 
                        << request.request_code << ":" 
                        << request.recipient << ":" 
                        << request.amount << std::endl;
                #endif 

                switch(request.request_code) 
                {
                    case CODE_BALANCE_REQUEST: {
                        BalanceHandler();
                        break;
                    }
                    case CODE_TRANSFER_REQUEST: {
                        TransferHandler();
                        break;
                    }
                    case CODE_LIST_REQUEST: {
                        ListHandler();
                        break;
                    }
                    default: throw std::runtime_error("\033[1;31m[ERROR]\033[0m Bad format message (request_code not known)");
                }
            }
        }
        catch(std::runtime_error& e) {
            #ifdef DEBUG
            std::cout << BLUE_BOLD << "[WORKER]" << RESET << " >> ";
            #endif
            std::cerr << e.what() << std::endl;
            close(client_socket);

            // Something went wrong: we need to clear the session (session key and HMAC key).

            continue;
        }

        #ifdef DEBUG
        std::cout << BLUE_BOLD << "[WORKER]" << RESET << " >> " 
                << "Client disconnected (socket: " 
                << client_socket << ")." << std::endl;
        #endif
    }

}

ClientReq Worker::RequestHandler()
{
    std::vector<uint8_t> buffer(SessionMessage::get_size(REQUEST_PACKET_SIZE));

    Receive(buffer, SessionMessage::get_size(REQUEST_PACKET_SIZE));

    SessionMessage encrypted_request = SessionMessage::deserialize(buffer, REQUEST_PACKET_SIZE);
    buffer.clear();

    #ifdef DEBUG
    std::cout << "Incoming encrypted message..." << std::endl;
    encrypted_request.print();
    #endif 

    // TO REMOVE
    std::vector<uint8_t> new_hmac_key(256, 0);
    std::vector<uint8_t> new_session_key(256, 1);
    // TO REMOVE

    if(!encrypted_request.verify_HMAC(new_hmac_key.data()))
        throw std::runtime_error("\033[1;31m[ERROR]\033[0m HMAC verification: FAILED.");

    std::vector<uint8_t> plaintext(REQUEST_PACKET_SIZE);
    encrypted_request.decrypt(new_session_key, plaintext);

    return ClientReq::deserialize(plaintext);
}

void Worker::BalanceHandler()
{
    #ifdef DEBUG
    std::cout << BLUE_BOLD << "[WORKER]" << RESET << " >> "
            << "balance (socket: " 
            << client_socket << ")." << std::endl;
    #endif
}

void Worker::TransferHandler()
{
    #ifdef DEBUG
    std::cout << BLUE_BOLD << "[WORKER]" << RESET << " >> "
            << "transfer (socket: " 
            << client_socket << ")." << std::endl;
    #endif
}

void Worker::ListHandler()
{
    std::vector<row_data_t> list = ListByUsername("transactions/Alice.txt");
    unsigned int n = list.size(); 
    
    List response(CODE_LIST_RESPONSE_1, n);
    std::vector<uint8_t> plaintext(LIST_RESPONSE_1_SIZE);
    response.serialize(plaintext);
    
    // TO REMOVE
    std::vector<uint8_t> new_hmac_key(256, 0);
    std::vector<uint8_t> new_session_key(256, 1);
    // TO REMOVE

    SessionMessage encrypted_response(new_session_key, new_hmac_key, plaintext);
    
    #ifdef DEBUG
    std::cout << "Sending encrypted message..." << std::endl;
    encrypted_response.print();
    #endif

    std::vector<uint8_t> to_send = encrypted_response.serialize();
    Send(to_send);

    plaintext.clear();
    plaintext.resize(LIST_RESPONSE_2_SIZE);
 
    for (row_data_t& transaction : list)
    {
        List response(CODE_LIST_RESPONSE_2, 
                    transaction.amount, 
                    reinterpret_cast<uint8_t*>(const_cast<char*>(transaction.dest.data())), 
                    sizeof(uint8_t[USER_SIZE]), 
                    reinterpret_cast<std::time_t>(transaction.timestamp));
        response.serialize(plaintext);

        Send(plaintext);

        plaintext.clear();
        plaintext.resize(LIST_RESPONSE_2_SIZE);
    }
}

ssize_t Worker::Receive(std::vector<uint8_t>& buffer, ssize_t buffer_size) {
    ssize_t total_bytes_received = 0;

    while (total_bytes_received < buffer_size) {
        ssize_t bytes_received = recv(client_socket, (void*)(buffer.data() + total_bytes_received), buffer_size - total_bytes_received, 0);

        if (bytes_received == -1)
            throw std::runtime_error("\033[1;31m[ERROR]\033[0m failed to receive data");

        if (bytes_received == 0) 
        {
            char message[sizeof("Client disconnected (socket: )") + sizeof(int)] = {0};
            sprintf(message, "Client disconnected (socket: %d)", client_socket);
            throw std::runtime_error(message);
        }

        total_bytes_received += bytes_received;
    }
    return total_bytes_received;
}

ssize_t Worker::Send(const std::vector<uint8_t>& buffer) {
    ssize_t total_bytes_sent = 0;
    ssize_t buffer_size = buffer.size();

    while (total_bytes_sent < buffer_size) {
        ssize_t bytes_sent = send(client_socket, (void*)(buffer.data() + total_bytes_sent), buffer_size - total_bytes_sent, 0);

        if (bytes_sent == -1 && (errno == EPIPE || errno == ECONNRESET)) {
            char message[sizeof("Client disconnected (socket: )") + sizeof(int)] = { 0 };
            sprintf(message, "Client disconnected (socket: %d)", client_socket);
            throw std::runtime_error(message);
        }

        if (bytes_sent == -1)
            throw std::runtime_error("\033[1;31m[ERROR]\033[0m failed to send data");

        total_bytes_sent += bytes_sent;
    }

    return total_bytes_sent;
}

std::vector<row_data_t> Worker::ListByUsername(const std::string& filename)
{
    std::vector<row_data_t> rows;
    std::ifstream file(filename);

    if (!file.is_open())
        throw std::runtime_error("Failed to open the file");

    std::string line;
    while (std::getline(file, line)) 
    {
        row_data_t row;
        row.dest = "";
        std::istringstream iss(line);
        if (iss >> row.dest >> row.amount >> row.timestamp)
            rows.push_back(row);
        else
            throw std::runtime_error("Failed to parse a line in the file");
    }

    file.close();
    return rows;
}


void Worker::AppendTransactionByUsername(const std::string& filename, const row_data_t& row)
{
    std::ofstream file(filename, std::ios::app);

    if (!file.is_open())
        throw std::runtime_error("Failed to open the file for appending");

    file << row.dest << " " << row.amount << " " << row.timestamp << "\n";

    file.close();
}
