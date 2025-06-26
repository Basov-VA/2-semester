#include "tcp_connect.h"
#include "byte_tools.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <chrono>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <limits>
#include <utility>

TcpConnect::TcpConnect(std::string ip, int port, std::chrono::milliseconds connectTimeout,
                       std::chrono::milliseconds readTimeout):
        ip_(ip),
        port_(port),
        connectTimeout_(connectTimeout),
        readTimeout_(readTimeout),
        status_(SocketStatus::Disconnected){}



void TcpConnect::EstablishConnection() {
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    address.sin_addr.s_addr = inet_addr(ip_.c_str());
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ < 0) {
        throw std::runtime_error("Failedsock");
    }
    int flags = fcntl(sock_, F_GETFL, 0);
    fcntl(sock_, F_SETFL, flags | O_NONBLOCK);

    int ret = connect(sock_, (struct sockaddr *) &address, sizeof(address));
    if (ret == 0) {
        // Соединение установлено немедленно
        flags = fcntl(sock_, F_GETFL, 0);
        fcntl(sock_, F_SETFL, flags & ~O_NONBLOCK);
        return;
    }
    struct pollfd pfd;
    pfd.fd = sock_;
    pfd.events = POLLOUT;
    ret = poll(&pfd, 1, connectTimeout_.count());
    if (ret == 0) {
        throw std::runtime_error("Connection-");
    } else if (ret < 0) {
        throw std::runtime_error("Errorcon");
    } else {
        flags = fcntl(sock_, F_GETFL, 0);
        fcntl(sock_, F_SETFL, flags & ~O_NONBLOCK);
        return;
    }
}
const std::string &TcpConnect::GetIp() const {
    return ip_;
}

int TcpConnect::GetPort() const {
    return port_;
}

TcpConnect::~TcpConnect() {
    if(status_ != SocketStatus::Closed)
    {
        TcpConnect::CloseConnection();
    }
}

//void TcpConnect::SetNonBlocking(int sockfd) {
//    int flags = fcntl(sockfd, F_GETFL, 0);
//    if (flags == -1) {
//        throw std::runtime_error("Failed to get socket flags");
//    }
//    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
//        throw std::runtime_error("Failed to set socket non-blocking");
//    }
//}
//
//void TcpConnect::SetBlocking(int sockfd) {
//    int flags = fcntl(sockfd, F_GETFL, 0);
//    if (flags == -1) {
//        throw std::runtime_error("Failed to get socket flags");
//    }
//    if (fcntl(sockfd, F_SETFL, flags & ~O_NONBLOCK) == -1) {
//        throw std::runtime_error("Failed to set socket blocking");
//    }
//}
//
//int TcpConnect::WaitForSocket(int sockfd, std::chrono::milliseconds timeout) {
//    struct pollfd pfd;
//    pfd.fd = sockfd;
//    pfd.events = POLLOUT;
//
//    int ret = poll(&pfd, 1, timeout.count());
//    if (ret < 0) {
//        // Если произошла ошибка при ожидании, возвращаем код ошибки
//        return -1;
//    } else if (ret == 0) {
//        // Если время ожидания истекло, возвращаем 0
//        return 0;
//    } else {
//        // В случае успеха возвращаем положительное значение
//        return 1;
//    }
//}




void TcpConnect::CloseConnection() {
    close(sock_);
    status_ = SocketStatus::Closed;
}
