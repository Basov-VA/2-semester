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

const std::string &TcpConnect::GetIp() const {
    return ip_;
}

int TcpConnect::GetPort() const {
    return port_;
}

TcpConnect::TcpConnect(std::string ip, int port, std::chrono::milliseconds connectTimeout, std::chrono::milliseconds readTimeout):
    ip_(ip),
    port_(port),
    connectTimeout_(connectTimeout),
    readTimeout_(readTimeout),
    status_(SocketStatus::NotInitialized){}


void TcpConnect::EstablishConnection(){
    sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(port_);
    peer_addr.sin_addr.s_addr = inet_addr(ip_.c_str());
    sock_ = socket(AF_INET, SOCK_STREAM, 0);

    int flags = fcntl(sock_, F_GETFL, 0);
    fcntl(sock_, F_SETFL, flags | O_NONBLOCK);
    if(sock_ == -1)
    {
        throw std::runtime_error("cant create socket in establish connection");

    }
    status_ = SocketStatus::Connected;
    if(!connect(sock_, (struct sockaddr*)&peer_addr, sizeof(peer_addr)))
    {
        flags = fcntl(sock_, F_GETFL, 0);
        fcntl(sock_, F_SETFL, flags | ~O_NONBLOCK);
        return;
    }

    pollfd pfd;
    pfd.fd = sock_;
    pfd.events = POLLOUT;
    
    int ret = poll(&pfd, 1, connectTimeout_.count());
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

void TcpConnect::SendData(const std::string& data) const {
    const char* buf = data.c_str();
    int left_data_size = data.size();
    while(left_data_size)
    {
        int sended = send(sock_, buf, left_data_size, 0);
        if(sended < 0){
            throw std::runtime_error("Failed sending data");
        }
        left_data_size -= sended;
        buf += sended;
    }
}

bool TcpConnect::recieveFullData(char* buf, size_t size) const
    {
        while(size > 0) {
            timeval deltaTime;
            deltaTime.tv_sec = readTimeout_.count() / 1000;
            deltaTime.tv_usec = (readTimeout_.count() % 1000) * 1000;

            if (setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, &deltaTime, sizeof(deltaTime)) < 0) {
                throw std::runtime_error("Failed to set socket timeout");
            }
            int recievedData = recv(size, buf, sock_, 0);
            if(recievedData < 0) {
                throw std::runtime_error("Failed to recieve data");
            }

            if(recievedData == 0) {
                return false; 
            }
            buf += recievedData;
            size -= recievedData;
        }
        return true;
    }

std::string TcpConnect::ReceiveData(size_t bufferSize) const {
    if(bufferSize == 0){
        char dataSize[4];
        if(!recieveFullData(dataSize, 4)) {
            throw std::runtime_error("Failed recieve size of needed data");
        }
        int len = BytesToInt(dataSize);
        if(len <= 0)
        {
            throw std::runtime_error("Recieve wrong size of recieve data");
        }
        std::string data(len, 0);
        if(!recieveFullData(&data[0], len)){
            return "";
        }
        return data;
    }

    std::string data(bufferSize, 0);
    if(!recieveFullData(&data[0], bufferSize)){
        return "";
    }
}

void TcpConnect::CloseConnection(){
    close(sock_);
    status_ = SocketStatus::Closed;
}


