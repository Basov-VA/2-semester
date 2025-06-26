#include "tcp_connect.h"
#include "byte_tools.h"

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
#include <errno.h>

const std::string &TcpConnect::GetIp() const {
    return ip_;
}

int TcpConnect::GetPort() const {
    return port_;
}

void TcpConnect::EstablishConnection() {
    sock_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_ == -1) {
        throw std::underflow_error("Couldn't create a socket");
    }

    struct sockaddr_in adress;
    adress.sin_family = AF_INET;
    adress.sin_port = htons(this->port_);
    adress.sin_addr.s_addr = inet_addr(this->ip_.c_str());

    int optional = fcntl(sock_, F_GETFL, 0);
    if(fcntl(sock_, F_SETFL, optional | O_NONBLOCK) == -1)
    {
        throw std::runtime_error(std::string("Error: ") + std::string(std::strerror(errno)));
    }

    int con = connect(sock_,(struct sockaddr*) &adress, sizeof(adress));
    if(con == 0)
    {
        optional = fcntl(sock_, F_GETFL, 0);
        fcntl(sock_, F_SETFL, optional & ~O_NONBLOCK);
    }
    else
    {
        if(errno != EINPROGRESS)
        {
            throw std::runtime_error("Unknown error");
        }
    }

    pollfd fd_;
    fd_.events = POLLOUT;
    fd_.fd = sock_;

    con = poll(&fd_,1,this->connectTimeout_.count());
    if(con == 0)
    {
        throw std::overflow_error("Сonnection waiting time exceeded");
    }
    if(con == -1)
    {
        throw std::runtime_error("Error while connecting to socket");
    }
    else
    {
        optional = fcntl(sock_, F_GETFD, 0);
        fcntl(sock_, F_SETFL, optional & ~O_NONBLOCK);
        return;
    }
}

void TcpConnect::SendData(const std::string &data) const {
    const char* buf = data.c_str();
    int len = data.size();
    while(len > 0)
    {
        int send_ = send(sock_,buf,len,0);
        if(send_ == -1)
        {
            throw std::runtime_error("Couldn't send data to socket");
        }
        len -= send_;
        buf += send_;
    }
}

std::string TcpConnect::ReceiveData(size_t bufferSize) const {
    if(bufferSize == 0)
    {
        struct pollfd fd_;
        fd_.events = POLLIN;
        fd_.fd = sock_;
        int len = 4;
        std::string s(4, 0);
        char *p = &s[0];
        while (len > 0) {
            int ret = poll(&fd_, 1, readTimeout_.count());
            if (ret == 0) {
                throw std::runtime_error("Сonnection waiting time exceeded");
            } else if (ret == -1) {
                throw std::runtime_error("er");
            } else {
                int n = recv(sock_, p, len, 0);
                if (n == -1) {
                    throw std::runtime_error("er recieve");
                }
                p += n;
                len -= n;
            }
        }
        len = BytesToInt(s);

        std::string data(len, 0);
        char *buf = &data[0];


        while (len > 0) {
            int ret = poll(&fd_, 1, readTimeout_.count());
            if (ret == 0) {
                throw std::runtime_error("Сonnection waiting time exceeded");
            } else if (ret == -1) {
                throw std::runtime_error("Error while connecting to remote host");
            } else {
                int n = recv(sock_, buf, len, 0);
                if (n == 0) {
                    throw std::runtime_error("Connection closed");
                }
                if (n == -1) {
                    throw std::runtime_error("er recieve");
                }
                buf += n;
                len -= n;
            }
        }
        return data;
    }
    else
    {
        struct pollfd fd_;
        fd_.events = POLLIN;
        fd_.fd = sock_;
        int len = bufferSize;
        std::string data(len, 0);
        char *buf = &data[0];


        while (len > 0) {
            int ret = poll(&fd_, 1, readTimeout_.count());
            if (ret == 0) {
                throw std::runtime_error("Сonnection waiting time exceeded");
            } else if (ret == -1) {
                throw std::runtime_error("Error while connecting to remote host");
            } else {
                int n = recv(sock_, buf, len, 0);
                if (n == 0) {
                    throw std::runtime_error("Connection closed");
                }
                if (n == -1) {
                    throw std::runtime_error("Failed to receive data");
                }
                buf += n;
                len -= n;
            }
        }
        return data;
    }
}


void TcpConnect::CloseConnection() {
    close(sock_);
    status = SocketStatus::Closed;
}

TcpConnect::~TcpConnect() {
    if(status != SocketStatus::Closed)
    {
        TcpConnect::CloseConnection();
    }
}

TcpConnect::TcpConnect(std::string ip, int port, std::chrono::milliseconds connectTimeout,
                       std::chrono::milliseconds readTimeout):
        ip_(ip),
        port_(port),
        connectTimeout_(connectTimeout),
        readTimeout_(readTimeout),
        status(SocketStatus::Disconnected){}

