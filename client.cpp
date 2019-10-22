#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <thread>

int connectToServer();
void sendMessage(int sockfd);
void readMessage(int sockfd);

const std::string PORT = "9090";
const std::string SERVER_ADDRESS = "localhost";

int connectToServer()
{
    struct addrinfo hints;
    struct addrinfo* server_info;  // contains possible address to connect according to our hints

    // set up criteria for type of address we want to connect to
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // get possible addresses to connect to
    int return_val;
    if ((return_val = getaddrinfo(SERVER_ADDRESS.c_str(), PORT.c_str(), &hints, &server_info)) != 0) {
        std::cerr << "Failed getting possible addresses: " << gai_strerror(return_val) << "\n";
        return -1;
    }

    // get a socket file descriptor
    int sockfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (sockfd == -1) {
        std::cerr << "Failed getting socket file descriptor\n";
        return -1;
    }

    // connect to server
    if (connect(sockfd, server_info->ai_addr, server_info->ai_addrlen) == -1) {
        close(sockfd);
        std::cerr << "Failed connecting to socket (couldn't connect to server)\n";
        return -1;
    }

    std::cout << "CONNECTED TO SERVER!!!\n";
    return sockfd;
}

void sendMessage(int sockfd)
{
    std::string message = "";
    std::cout << "Enter message: ";
    std::getline(std::cin, message);  // getline to support spaces in input

    int payload_length = message.length();

    std::string header = std::to_string(payload_length);
    header.append(8 - header.length(), ' ');

    // send header
    if (send(sockfd, header.c_str(), 8, 0) == -1) {
        std::cerr << "Error sending message\n";
        return;
    }

    // send payload
    if (send(sockfd, message.c_str(), payload_length, 0) == -1) {
        std::cerr << "Error sending message\n";
        return;
    }
}

void readMessage(int sockfd)
{
    while (true) {
        char header[8];

        // receive header
        if (recv(sockfd, header, 8, 0) == -1) {
            std::cerr << "Error receiving header\n";
            return;
        }

        long payload_length = strtol(header, NULL, 0);
        char buffer[1024];  // power of 2 because apparently it's better for networking
        memset(buffer, 0, sizeof(buffer));  // fill with 0's so null terminated by default

        // receive payload
        if (recv(sockfd, buffer, payload_length, 0) == -1) {
            std::cerr << "Error receiving payload\n";
            return;
        }

        // newline in beginnging bc we're always on input prompt line
        std::cout << "\nMessage received: " << buffer << "\n";

        // reprint input prompt and flush to print straight away
        std::cout << "Enter message: " << std::flush;
    }
}

int main(int argc, char* argv[])
{
    int sockfd = connectToServer();

    if (sockfd != -1) {
        std::thread read_thread{readMessage, sockfd};

        while (true) {
            sendMessage(sockfd);
        }
    }
}
