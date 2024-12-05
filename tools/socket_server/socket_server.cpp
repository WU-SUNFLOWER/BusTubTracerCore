#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <csignal>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "myapi/api_manager.h"

#define SOCKET_PATH "/tmp/bustub_core_socket"
#define RECV_BUFFER_SIZE 16

#define PROTOCOL_HEADER_STR "BTC"
#define PROTOCOL_OPTIONS_OFFSET 3
#define PROTOCOL_PAYLOAD_LENGTH_OFFSET 4
#define PROTOCOL_HEADER_SIZE 8

// type=0, client->server
// type=1, server->client
#define OPTIONS_TYPE_MASK 0x01  

static std::unique_ptr<bustub::BustubInstance> kBustubInstance = nullptr;

void BustubInit() {
    std::cout << "Initialize BusTub..." << std::endl;
    auto bustub = std::make_unique<bustub::BustubInstance>("/tmp/bustub.db");
    bustub->GenerateTestTable();

    kBustubInstance = std::move(bustub);

    std::cout << "BusTub Initialized!" << std::endl;

}

auto PackDatagram(const std::string &payload) -> std::vector<uint8_t> {
    uint32_t payload_length = payload.length();
    std::vector<uint8_t> datagram(PROTOCOL_HEADER_SIZE + payload_length);
    // set the header
    memcpy(datagram.data(), PROTOCOL_HEADER_STR, 3);
    // set the options
    datagram[PROTOCOL_OPTIONS_OFFSET] = 0x01;
    // set the payload length
    uint32_t payload_length_bigend = htonl(payload_length);
    memcpy(datagram.data() + PROTOCOL_PAYLOAD_LENGTH_OFFSET, 
           reinterpret_cast<uint8_t*>(&payload_length_bigend), sizeof(uint32_t));
    // set the payload
    memcpy(datagram.data() + PROTOCOL_HEADER_SIZE, payload.c_str(), payload_length);

    return datagram;
}

int main() {

    pid_t parent_pid = getppid();

    BustubInit();

    int server_fd;
    int client_fd;
    sockaddr_un address;

    char recv_buffer[RECV_BUFFER_SIZE];
    std::string header_buffer;
    std::string payload_buffer;

    unlink(SOCKET_PATH);

    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        std::cerr << "can't create socket" << std::endl;
        exit(1);
    }

    bzero(&address, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == -1) {
        std::cerr << "can't bind address" << std::endl;
        exit(1);
    }

    if (listen(server_fd, 5) == -1) {
        std::cerr << "can't listen socket" << std::endl;
    }

    // all the preparation work is completed,
    // now let's notify the parent process!
    kill(parent_pid, SIGUSR1);

    while (true) {
        if ((client_fd = accept(server_fd, nullptr, nullptr)) == -1) {
            std::cerr << "can't accept client" << std::endl;
            continue;
        }
        std::cout << "accept client fd: " << client_fd << std::endl;

        bool has_received_header = false;
        uint8_t options = 0x00;
        uint32_t total_payload_length = 0;

        while (true) {
            
            // read raw data from kernel buffer to server buffer
            int num_bytes = recv(client_fd, recv_buffer, RECV_BUFFER_SIZE, 0);
            if (num_bytes <= 0) {
                if (num_bytes == 0) {
                    std::cout << "client closes connection: " << num_bytes << std::endl;
                } else {
                    std::cerr << "can't recv from client: " << num_bytes << std::endl;
                }
                break;
            }

            // start to read new datagram
            if (!has_received_header) {
                
                // store received data to header buffer.
                header_buffer.append(recv_buffer, num_bytes);

                // if the datagram header is uncompleted,
                // let's continue to receive data.
                if (header_buffer.length() < PROTOCOL_HEADER_SIZE) {
                    continue;
                }

                // ok, yet we have got the completed header. 
                const char* raw_header_buffer = header_buffer.c_str();
                // check header
                if (strncmp(raw_header_buffer, PROTOCOL_HEADER_STR, 3) != 0) {
                    std::cerr << "illegal header of client request" << std::endl;
                    continue;
                }
                // extract options
                options = 
                    *reinterpret_cast<const uint8_t*>(raw_header_buffer + PROTOCOL_OPTIONS_OFFSET);
                if ((options & OPTIONS_TYPE_MASK) != 0) {
                    std::cerr << "illegal datagram type!" << std::endl;
                }
                // extract payload length
                total_payload_length = 
                    *reinterpret_cast<const uint32_t*>(raw_header_buffer + PROTOCOL_PAYLOAD_LENGTH_OFFSET);
                total_payload_length = ntohl(total_payload_length);
                // clear our payload buffer,
                // and then copy the excess bytes to payload buffer.
                payload_buffer.clear();
                payload_buffer.append(raw_header_buffer + PROTOCOL_HEADER_SIZE, 
                    header_buffer.length() - PROTOCOL_HEADER_SIZE);
                // don't forget to set flag finally
                has_received_header = true;

                /*
                std::cout << "receive new datagram header:" 
                    << " options=" << std::to_string(options)
                    << " payload_length=" << total_payload_length
                    << std::endl;                
                */

            } else {
                // just store the data to payload buffer
                payload_buffer.append(recv_buffer, num_bytes);
            }

            if (payload_buffer.length() > total_payload_length) {
                std::cerr << "find more bytes than payload length!" << std::endl;
                payload_buffer.resize(total_payload_length);
            }

            // all the bytes of payload have been read to payload buffer!
            if (payload_buffer.length() == total_payload_length) {
                //std::cout << "receive from client: " << payload_buffer << std::endl;

                ApiManager api_manager(kBustubInstance.get());
                std::string respond = std::move(api_manager.DispatchRequest(payload_buffer));
                
                std::vector<uint8_t> datagram = std::move(PackDatagram(respond));
                if (send(client_fd, datagram.data(), datagram.size(), 0) == -1) {
                    std::cerr << "can't send to client" << std::endl;
                    break;
                }

                // don't forget to clear header buffer and reset header flag
                header_buffer.clear();
                has_received_header = false;
            }

        }

        close(client_fd);

    }

    close(server_fd);
    unlink(SOCKET_PATH);

    return 0;

}